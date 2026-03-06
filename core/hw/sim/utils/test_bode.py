"""
Open-loop Bode analysis of the PDH controller RTL.

Drives adc_dat_a_i with a sinusoid, measures the PID/DAC output, and
computes the open-loop transfer function H(jω) = DFT(out) / DFT(in)
for four signal-path configurations and a sweep of decimation codes.

Signal convention
-----------------
  in_centered[n]  = -(adc_dat_a_i[n] - 8192)   (what the PID sees after ADC negation)
  out_centered[n] = dac_dat_o[n] - 8191         (PID output, centred)
  H_rtl(jω) = DFT(out_centered) / DFT(in_centered)

Line delay (physical loop):
  H(jω) = H_rtl(jω) × exp(-j·2π·freq·line_delay_s)
  where line_delay_s = LINE_DELAY_NS × 1e-9

The line delay is applied analytically because in the open-loop Bode test we
inject directly at the ADC; the physical DAC→ADC path delay is not simulated.

At DC with P-only control, H_rtl ≈ +Kp (positive, phase = 0°).
Phase margin = 180° + ∠H(ω_gc)  where |H(ω_gc)| = 1.
Gain  margin = 1 / |H(ω_pc)|    where ∠H(ω_pc) = -180°.

Environment variables
---------------------
  COEFF_CSV       path to FIR coefficient CSV (one Q15 float per line)
  RESULTS_JSON    output path for JSON results  (default: bode_results.json)
  KP_TEST         Kp gain                        (default: 0.5)
  KI_TEST         Ki gain                        (default: 0.0)
  KD_TEST         Kd gain                        (default: 0.0)
  ALPHA_TEST      EMA alpha exponent (2^-alpha)  (default: 2)
  SATWIDTH_TEST   integrator saturation width    (default: 31)
  LINE_DELAY_NS   physical DAC→ADC delay in ns   (default: 0.0)
  DEC_CODES       comma-separated decimation codes
                  (default: 1,2,5,10,20,50,100,200,500,1000)
"""

import cmath
import json
import math
import os
import pathlib

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import ClockCycles, RisingEdge

# ── RTL / system constants ──────────────────────────────────────────────────────

FS         = 125e6      # sample rate, Hz
ADC_OFFSET = 8192       # 14-bit offset-binary midpoint
DAC_OFFSET = 8191       # 14-bit DAC zero point (unsigned output centred here)
NTAPS      = 32
AW         = 5          # log2(NTAPS)

# FIR pipeline latency added to N_WARMUP for configs that use the FIR.
FIR_LAT = NTAPS + AW + 1 + 16   # = 54 cycles (matches test_fir_freq_response)

# ── Command codes (mirror pdh_core.sv) ─────────────────────────────────────────

CMD_SET_ROT_COEFFS    = 0b0101
CMD_COMMIT_ROT_COEFFS = 0b0110
CMD_SET_PID_COEFFS    = 0b1000
CMD_SET_FIR           = 0b1010
CMD_CONFIG_IO         = 0b1110

# ── PID coeff selects ───────────────────────────────────────────────────────────

PID_SELECT_KP    = 0b0000
PID_SELECT_KD    = 0b0001
PID_SELECT_KI    = 0b0010
PID_SELECT_DEC   = 0b0011
PID_SELECT_SP    = 0b0100
PID_SELECT_ALPHA = 0b0101
PID_SELECT_SAT   = 0b0110
PID_SELECT_EN    = 0b0111

# ── FIR update selects ──────────────────────────────────────────────────────────

FIR_SELECT_ADDR           = 0b000
FIR_SELECT_COEFF          = 0b001
FIR_SELECT_INPUT_SEL      = 0b010
FIR_SELECT_MEM_WRITE_EN   = 0b011
FIR_SELECT_CHAIN_WRITE_EN = 0b100

# FIR input source codes (mirror fir_input_sel_t in pdh_core.sv)
FIR_INPUT_ADC1   = 0b000
FIR_INPUT_I_FEED = 0b010

# ── IO routing selects ──────────────────────────────────────────────────────────

DAC_SEL_PID   = 0b001

# pid_sel codes (mirror pid_sel_t in pdh_core.sv)
PID_SEL_IFEED  = 0b000
PID_SEL_ADCA   = 0b010
PID_SEL_FIROUT = 0b100   # FIR_OUT_W

# Cycles for the 3-FF synchroniser + posedge detector to propagate.
SYNC_CYCLES = 8


# ── Numeric helpers (verbatim from test_pdh_core.py) ───────────────────────────

def _as_signed16(v: int) -> int:
    v = int(v) & 0xFFFF
    return v - 0x10000 if v >= 0x8000 else v


def _float_to_q15(x: float) -> int:
    if x >= 0.999969482421875:
        return 0x7FFF
    if x <= -1.0:
        return -32768
    return max(-32768, min(32767, round(x * 32768.0)))


def _float_to_q13(x: float) -> int:
    return max(-8192, min(8191, round(x * 8192.0)))


# ── AXI word packing (verbatim from test_pdh_core.py) ──────────────────────────

def _word(rst: int, strobe: int, cmd: int, data: int) -> int:
    return (
        ((rst    & 0x1)  << 31) |
        ((strobe & 0x1)  << 30) |
        ((cmd    & 0xF)  << 26) |
        ((data   & 0x3FFFFFF)  )
    )


# ── Protocol helpers (verbatim from test_pdh_core.py) ──────────────────────────

async def _reset(dut) -> None:
    dut.axi_from_ps_i.value = _word(1, 0, 0, 0)
    await ClockCycles(dut.clk, 10)
    dut.axi_from_ps_i.value = _word(0, 0, 0, 0)
    await ClockCycles(dut.clk, 10)


async def _send(dut, cmd: int, data: int) -> int:
    dut.axi_from_ps_i.value = _word(0, 0, cmd, data)
    await ClockCycles(dut.clk, SYNC_CYCLES)
    dut.axi_from_ps_i.value = _word(0, 1, cmd, data)
    await ClockCycles(dut.clk, SYNC_CYCLES)
    cb = int(dut.axi_to_ps_o.value)
    dut.axi_from_ps_i.value = _word(0, 0, cmd, data)
    await ClockCycles(dut.clk, SYNC_CYCLES)
    return cb


# ── DFT amplitude/phase helper ─────────────────────────────────────────────────

def _dft(samples: list, omega: float) -> complex:
    """Exact DFT evaluation at angular frequency omega (unnormalised)."""
    s = sum(x * cmath.exp(-1j * omega * n) for n, x in enumerate(samples))
    return s / len(samples)


# ── Setup helpers ───────────────────────────────────────────────────────────────

async def _setup_pid(dut, kp: float, ki: float, kd: float,
                     alpha: int, satwidth: int, dec: int) -> None:
    """Program the PID: setpoint=0, then enable."""
    async def _pid(sel, val16):
        await _send(dut, CMD_SET_PID_COEFFS, (sel << 16) | (val16 & 0xFFFF))

    await _pid(PID_SELECT_EN,    0)
    await _pid(PID_SELECT_KP,    _float_to_q15(kp) & 0xFFFF)
    await _pid(PID_SELECT_KD,    _float_to_q15(kd) & 0xFFFF)
    await _pid(PID_SELECT_KI,    _float_to_q15(ki) & 0xFFFF)
    await _pid(PID_SELECT_ALPHA, int(alpha) & 0xFFFF)
    await _pid(PID_SELECT_SP,    0)
    await _pid(PID_SELECT_SAT,   int(satwidth) & 0xFFFF)
    await _pid(PID_SELECT_DEC,   int(dec) & 0xFFFF)
    await _pid(PID_SELECT_EN,    1)


async def _setup_rotation_zero(dut) -> None:
    """Set rotation to 0°: cos=0x7FFF, sin=0, then commit."""
    await _send(dut, CMD_SET_ROT_COEFFS, (0x7FFF & 0xFFFF) | (0 << 16))
    await _send(dut, CMD_SET_ROT_COEFFS, (0x0000 & 0xFFFF) | (1 << 16))
    await _send(dut, CMD_COMMIT_ROT_COEFFS, 0)
    await ClockCycles(dut.clk, 4)


async def _setup_config_io(dut, pid_sel: int) -> None:
    """Route both DACs to PID output, PID input to pid_sel."""
    io_data = DAC_SEL_PID | (DAC_SEL_PID << 3) | (pid_sel << 6)
    await _send(dut, CMD_CONFIG_IO, io_data)


async def _program_fir(dut, coeffs: list, input_sel: int) -> None:
    """Write FIR coefficients via CMD_SET_FIR, then strobe chain_write_en."""
    async def _fir(sel, val=0):
        return await _send(dut, CMD_SET_FIR, (sel << 16) | (val & 0xFFFF))

    await _fir(FIR_SELECT_INPUT_SEL, input_sel)

    await _fir(FIR_SELECT_MEM_WRITE_EN, 1)
    for i, c in enumerate(coeffs):
        await _fir(FIR_SELECT_ADDR,  i)
        await _fir(FIR_SELECT_COEFF, _float_to_q15(c) & 0xFFFF)
    await _fir(FIR_SELECT_MEM_WRITE_EN, 0)

    await _fir(FIR_SELECT_CHAIN_WRITE_EN, 1)
    await _fir(FIR_SELECT_CHAIN_WRITE_EN, 0)


# ── Per-frequency measurement ───────────────────────────────────────────────────

async def _measure(dut, freq: float, dec: int, n_warmup: int,
                   line_delay_s: float) -> complex:
    """
    Drive adc_dat_a_i with a cosine at freq Hz for n_warmup + n_meas cycles.
    Return H(jω) including the analytical line-delay phase factor.
    """
    omega  = 2.0 * math.pi * freq / FS
    n_meas = max(512, int(8.0 * FS / freq))

    amp_code = round(0.30 * ADC_OFFSET)

    in_samples:  list = []
    out_samples: list = []

    for n in range(n_warmup + n_meas):
        raw = (ADC_OFFSET + round(amp_code * math.cos(omega * n))) & 0x3FFF
        dut.adc_dat_a_i.value = raw
        await RisingEdge(dut.clk)
        if n >= n_warmup:
            in_samples.append(-(raw - ADC_OFFSET))
            out_samples.append(int(dut.dac_dat_o.value) - DAC_OFFSET)

    h_in  = _dft(in_samples,  omega)
    h_out = _dft(out_samples, omega)
    if abs(h_in) < 1e-9:
        return complex(0, 0)

    H_rtl = h_out / h_in

    if line_delay_s > 0.0:
        H_rtl *= cmath.exp(-1j * 2.0 * math.pi * freq * line_delay_s)

    return H_rtl


# ── Stability margin helpers ────────────────────────────────────────────────────

def _unwrap(phases: list) -> list:
    result = list(phases)
    for i in range(1, len(result)):
        diff = result[i] - result[i - 1]
        while diff > math.pi:
            diff -= 2 * math.pi
        while diff < -math.pi:
            diff += 2 * math.pi
        result[i] = result[i - 1] + diff
    return result


def _interp(x0, y0, x1, y1, y_target):
    if abs(y1 - y0) < 1e-30:
        return None
    return x0 + (y_target - y0) * (x1 - x0) / (y1 - y0)


def _compute_margins(freqs, H_complex):
    gains      = [abs(h) for h in H_complex]
    phases     = _unwrap([cmath.phase(h) for h in H_complex])
    phases_deg = [math.degrees(p) for p in phases]

    f_gc, pm_deg = None, None
    f_pc, gm_db  = None, None

    for i in range(len(freqs) - 1):
        g0, g1 = gains[i], gains[i + 1]
        p0, p1 = phases_deg[i], phases_deg[i + 1]
        f0, f1 = freqs[i], freqs[i + 1]

        if f_gc is None and g0 >= 1.0 > g1:
            f_gc = _interp(f0, g0, f1, g1, 1.0)
            if f_gc is not None:
                frac   = (f_gc - f0) / (f1 - f0) if f1 != f0 else 0
                pm_deg = 180.0 + (p0 + frac * (p1 - p0))

        if f_pc is None and p0 >= -180.0 > p1:
            f_pc = _interp(f0, p0, f1, p1, -180.0)
            if f_pc is not None:
                frac    = (f_pc - f0) / (f1 - f0) if f1 != f0 else 0
                g_at_pc = g0 + frac * (g1 - g0)
                gm_db   = -20.0 * math.log10(max(g_at_pc, 1e-9))

    return {"f_gc": f_gc, "pm_deg": pm_deg, "f_pc": f_pc, "gm_db": gm_db}


# ── Main cocotb test ────────────────────────────────────────────────────────────

@cocotb.test()
async def test_bode(dut):
    _repo_root   = pathlib.Path(__file__).resolve().parents[4]
    _default_csv = _repo_root / "test_resources" / "fir_coeffs_example.csv"
    _here        = pathlib.Path(__file__).resolve().parent.parent  # sim/

    csv_path     = pathlib.Path(os.environ.get("COEFF_CSV", str(_default_csv)))
    results_path = pathlib.Path(os.environ.get("RESULTS_JSON",
                                               str(_here / "bode_results.json")))
    kp_test       = float(os.environ.get("KP_TEST",      "0.5"))
    ki_test       = float(os.environ.get("KI_TEST",      "0.0"))
    kd_test       = float(os.environ.get("KD_TEST",      "0.0"))
    alpha_test    = int(os.environ.get("ALPHA_TEST",   "2"))
    satwidth_test = int(os.environ.get("SATWIDTH_TEST", "31"))
    delay_ns      = float(os.environ.get("LINE_DELAY_NS", "0.0"))
    delay_s       = delay_ns * 1e-9
    dec_codes     = [int(d) for d in os.environ.get("DEC_CODES",
                                                     "1,2,5,10,20,50,100,200,500,1000").split(",")]

    coeffs: list = []
    with open(csv_path) as f:
        for line in f:
            line = line.strip()
            if line:
                coeffs.append(float(line))
    ntaps = len(coeffs)
    print(f"\nLoaded {ntaps} FIR taps from {csv_path}")
    print(f"PID: Kp={kp_test}  Ki={ki_test}  Kd={kd_test}  "
          f"alpha={alpha_test}  satwidth={satwidth_test}")
    print(f"Line delay: {delay_ns} ns  |  decimation codes: {dec_codes}")

    cocotb.start_soon(Clock(dut.clk, 8, unit="ns").start())
    dut.dma_ready_i.value  = 1
    dut.bram_ready_i.value = 1
    dut.adc_dat_a_i.value  = ADC_OFFSET
    dut.adc_dat_b_i.value  = ADC_OFFSET
    await _reset(dut)

    configs = [
        ("direct_adc",    PID_SEL_ADCA,   False, FIR_INPUT_ADC1,   False),
        ("rotation",      PID_SEL_IFEED,  False, FIR_INPUT_ADC1,   True),
        ("fir_adc",       PID_SEL_FIROUT, True,  FIR_INPUT_ADC1,   False),
        ("fir_rotation",  PID_SEL_FIROUT, True,  FIR_INPUT_I_FEED, True),
    ]

    all_results = {}

    for cfg_name, pid_sel, use_fir, fir_input_sel, use_rotation in configs:
        print(f"\n{'='*60}")
        print(f"  Config: {cfg_name}")
        print(f"{'='*60}")

        if use_rotation:
            await _setup_rotation_zero(dut)
        if use_fir:
            await _program_fir(dut, coeffs, fir_input_sel)
        await _setup_config_io(dut, pid_sel)

        extra_fir = FIR_LAT if use_fir else 0
        extra_rot = 2 if use_rotation else 0

        cfg_data = {"decimations": dec_codes, "data": {}}

        for dec in dec_codes:
            f_nyq   = FS / (2.0 * dec)
            n_freqs = 15
            freqs   = [
                0.03 * f_nyq * (0.48 / 0.03) ** (i / (n_freqs - 1))
                for i in range(n_freqs)
            ]

            await _setup_pid(dut, kp_test, ki_test, kd_test,
                             alpha_test, satwidth_test, dec)

            n_warmup = max(200, 2 * ntaps + 5 * dec) + extra_fir + extra_rot

            print(f"\n  dec={dec:5d}  f_nyq={f_nyq/1e3:8.1f} kHz  "
                  f"n_warmup={n_warmup}")

            H_meas: list = []
            for freq in freqs:
                H = await _measure(dut, freq, dec, n_warmup, delay_s)
                H_meas.append(H)
                gain_db = 20.0 * math.log10(max(abs(H), 1e-9))
                phase_d = math.degrees(cmath.phase(H))
                print(f"    {freq/1e3:8.2f} kHz  |H|={abs(H):.4f} "
                      f"({gain_db:.1f} dB)  ∠H={phase_d:.1f}°")

            margins = _compute_margins(freqs, H_meas)

            cfg_data["data"][str(dec)] = {
                "freqs":     freqs,
                "gain_db":   [20.0 * math.log10(max(abs(h), 1e-9)) for h in H_meas],
                "phase_deg": [math.degrees(cmath.phase(h)) for h in H_meas],
                "f_gc":      margins["f_gc"],
                "pm_deg":    margins["pm_deg"],
                "f_pc":      margins["f_pc"],
                "gm_db":     margins["gm_db"],
            }

            pm_str = f"{margins['pm_deg']:.1f}°" if margins["pm_deg"] is not None else "N/A"
            gm_str = f"{margins['gm_db']:.1f} dB" if margins["gm_db"] is not None else "N/A"
            print(f"    → PM={pm_str}  GM={gm_str}")

        all_results[cfg_name] = cfg_data

    output = {
        "configs":        all_results,
        "fs":             FS,
        "kp":             kp_test,
        "ki":             ki_test,
        "kd":             kd_test,
        "alpha":          alpha_test,
        "satwidth":       satwidth_test,
        "line_delay_ns":  delay_ns,
        "csv_path":       str(csv_path),
        "ntaps":          ntaps,
    }
    with open(results_path, "w") as f:
        json.dump(output, f)
    print(f"\nResults written → {results_path}")
