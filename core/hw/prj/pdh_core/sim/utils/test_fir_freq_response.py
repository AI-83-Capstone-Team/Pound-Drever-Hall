"""
Measure the frequency response of the FIR RTL by driving sine-wave inputs
and comparing the steady-state output amplitude against the ideal response
computed analytically from the tap coefficients.

The test drives the `fir` module directly (not through pdh_core) so that
din_i and dout_o are accessible without any command-protocol overhead.

Environment variables:
    COEFF_CSV   Path to a CSV file with one Q15 float per line.
                Default: <repo_root>/test_resources/fir_coeffs_example.csv

Run via:
    python run_fir_freq.py [--csv path/to/coeffs.csv]
"""

import cmath
import math
import os
import pathlib

import json

import cocotb
from cocotb.clock import Clock
from cocotb.triggers import ClockCycles, RisingEdge

# ── RTL parameters (must match Makefile.fir_freq) ──────────────────────────────

NTAPS = 32
DW    = 16
AW    = 5          # = log2(NTAPS)
FS    = 125e6      # sample rate, Hz

# Pipeline latency (cycles from din_i to dout_o):
#   1 cycle  – tap output register (out_r in fir_tap)
#   AW cycles – pipelined adder tree
# Add NTAPS to fill the delay line with steady-state samples.
N_WARMUP = NTAPS + AW + 1 + 16     # = 54 cycles (with a comfortable margin)


# ── Helpers ────────────────────────────────────────────────────────────────────

def _float_to_q15(x: float) -> int:
    return max(-32768, min(32767, round(x * 32768.0)))


def _as_signed16(v: int) -> int:
    v = int(v) & 0xFFFF
    return v - 0x10000 if v >= 0x8000 else v


def _dft_amplitude(samples: list, omega: float) -> float:
    """
    Estimate the amplitude of a sinusoid at angular frequency omega
    using exact DFT evaluation at that frequency.
    """
    s = sum(x * cmath.exp(-1j * omega * n) for n, x in enumerate(samples))
    return 2.0 * abs(s) / len(samples)


# ── FIR coefficient loading ─────────────────────────────────────────────────────

async def _program_fir(dut, coeffs: list) -> None:
    """
    Write Q15 coefficients into the tap BRAM one address at a time
    (tap_mem_write_en_i), then pulse tap_chain_write_en_i for one cycle to
    load every tap simultaneously from the BRAM.
    """
    dut.tap_chain_write_en_i.value = 0
    dut.tap_mem_write_en_i.value   = 1
    for i, c in enumerate(coeffs):
        dut.tap_addr_i.value  = i
        dut.tap_coeff_i.value = _float_to_q15(c) & 0xFFFF
        await RisingEdge(dut.clk)

    dut.tap_mem_write_en_i.value = 0
    await RisingEdge(dut.clk)

    # One-cycle strobe: all taps latch tap_coeffs[i] simultaneously.
    dut.tap_chain_write_en_i.value = 1
    await RisingEdge(dut.clk)
    dut.tap_chain_write_en_i.value = 0
    await RisingEdge(dut.clk)


# ── Per-frequency measurement ───────────────────────────────────────────────────

async def _measure(dut, freq: float, n_meas: int) -> float:
    """
    Drive a cosine at freq Hz for N_WARMUP + n_meas cycles.
    Collect dout_o samples only during the last n_meas cycles (steady state).
    Return |H(freq)| = output_amplitude / input_amplitude.
    """
    omega   = 2.0 * math.pi * freq / FS
    amp_q15 = round(0.45 * 32767)      # ~45 % FS — headroom against overflow in adder tree

    in_samples:  list = []
    out_samples: list = []

    for n in range(N_WARMUP + n_meas):
        raw = round(amp_q15 * math.cos(omega * n)) & 0xFFFF
        dut.din_i.value = raw
        await RisingEdge(dut.clk)
        if n >= N_WARMUP:
            in_samples.append(_as_signed16(raw) / 32768.0)
            out_samples.append(_as_signed16(int(dut.dout_o.value)) / 32768.0)

    a_in  = _dft_amplitude(in_samples,  omega)
    a_out = _dft_amplitude(out_samples, omega)
    return a_out / a_in if a_in > 1e-9 else 0.0


# ── Main cocotb test ────────────────────────────────────────────────────────────

@cocotb.test()
async def test_fir_freq_response(dut):
    # ── Load coefficients ───────────────────────────────────────────────────────
    _repo_root   = pathlib.Path(__file__).resolve().parents[6]
    _default_csv = _repo_root / "test_resources" / "fir_coeffs_example.csv"
    _here        = pathlib.Path(__file__).resolve().parent.parent  # sim/
    csv_path     = pathlib.Path(os.environ.get("COEFF_CSV", str(_default_csv)))

    coeffs: list = []
    with open(csv_path) as f:
        for line in f:
            line = line.strip()
            if line:
                coeffs.append(float(line))

    ntaps = len(coeffs)
    print(f"\nLoaded {ntaps} coefficients from {csv_path}")
    print(f"  DC gain  : {sum(coeffs):.6f}")
    print(f"  max|coeff|: {max(abs(c) for c in coeffs):.6f}")

    # ── Clock and reset ─────────────────────────────────────────────────────────
    cocotb.start_soon(Clock(dut.clk, 8, unit="ns").start())

    dut.rst.value                  = 1
    dut.din_i.value                = 0
    dut.tap_addr_i.value           = 0
    dut.tap_coeff_i.value          = 0
    dut.tap_mem_write_en_i.value   = 0
    dut.tap_chain_write_en_i.value = 0
    await ClockCycles(dut.clk, 8)
    dut.rst.value = 0
    await ClockCycles(dut.clk, 4)

    # ── Program FIR (once, before the sweep) ───────────────────────────────────
    await _program_fir(dut, coeffs)

    # ── Frequency sweep ─────────────────────────────────────────────────────────
    f_min    = FS / 256 * 8        # ≈ 3.9 MHz — lowest frequency we sweep
    f_max    = FS * 0.49           # ≈ 61.25 MHz
    n_points = 60

    freqs_sweep = [
        f_min * (f_max / f_min) ** (i / (n_points - 1))
        for i in range(n_points)
    ]

    H_rtl: list = []
    print(f"\nSweeping {n_points} frequencies from {f_min/1e6:.2f} MHz "
          f"to {f_max/1e6:.2f} MHz ...")

    for freq in freqs_sweep:
        n_meas = max(256, int(8.0 * FS / freq))
        mag    = await _measure(dut, freq, n_meas)
        H_rtl.append(mag)
        print(f"  {freq/1e6:7.3f} MHz  |H| = {mag:.4f}  "
              f"({20 * math.log10(max(mag, 1e-6)):.1f} dB)")

    # ── Ideal frequency response (computed from coefficients, full range) ───────
    n_ideal     = 2000
    freqs_ideal = [k * FS / 2 / (n_ideal - 1) for k in range(n_ideal)]
    H_ideal: list = []
    for fk in freqs_ideal:
        omega = 2.0 * math.pi * fk / FS
        H = sum(c * cmath.exp(-1j * omega * n) for n, c in enumerate(coeffs))
        H_ideal.append(abs(H))

    # ── Write results ────────────────────────────────────────────────────────────
    results = {
        "csv_path":    str(csv_path),
        "ntaps":       ntaps,
        "fs":          FS,
        "f_max":       f_max,
        "freqs_ideal": freqs_ideal,
        "H_ideal":     H_ideal,
        "freqs_rtl":   freqs_sweep,
        "H_rtl":       H_rtl,
    }
    out = os.environ.get("RESULTS_JSON", str(_here / "fir_freq_results.json"))
    with open(out, "w") as f:
        json.dump(results, f)
    print(f"\nResults written → {out}")
