"""
cocotb sanity test for pdh_core.

Mirrors the command-echo checks from client/test.py but operates directly
on the RTL — no DMA, no server, no hardware.  DMA-related inputs are tied
to benign static values.

Run via:
    make -C core/hw/prj/pdh_core/sim          # or: make sim from core/hw/
"""

import math
import cocotb
from cocotb.clock import Clock
from cocotb.triggers import ClockCycles, RisingEdge

# ── Command codes (match pdh_core.sv) ──────────────────────────────────────────

CMD_IDLE              = 0b0000
CMD_SET_LED           = 0b0001
CMD_SET_DAC           = 0b0010
CMD_GET_ADC           = 0b0011
CMD_CHECK_SIGNED      = 0b0100
CMD_SET_ROT_COEFFS    = 0b0101
CMD_COMMIT_ROT_COEFFS = 0b0110
CMD_GET_FRAME         = 0b0111
CMD_SET_PID_COEFFS    = 0b1000
CMD_SET_NCO           = 0b1001
CMD_SET_FIR           = 0b1010
CMD_CONFIG_DEMOD      = 0b1011
CMD_CONFIG_IO         = 0b1110

# ── PID coeff selects ──────────────────────────────────────────────────────────

PID_SELECT_KP    = 0b0000
PID_SELECT_KD    = 0b0001
PID_SELECT_KI    = 0b0010
PID_SELECT_DEC   = 0b0011
PID_SELECT_SP    = 0b0100
PID_SELECT_ALPHA = 0b0101
PID_SELECT_SAT   = 0b0110
PID_SELECT_EN    = 0b0111
PID_SELECT_GAIN  = 0b1000
PID_SELECT_BIAS  = 0b1001
PID_SELECT_EGAIN = 0b1010

# ── NCO coeff selects ──────────────────────────────────────────────────────────

NCO_SELECT_STRIDE = 0b000
NCO_SELECT_SHIFT  = 0b001
NCO_SELECT_INV    = 0b010
NCO_SELECT_SUB    = 0b011
NCO_SELECT_EN     = 0b100

# ── FIR update selects ─────────────────────────────────────────────────────────

FIR_SELECT_ADDR           = 0b000
FIR_SELECT_COEFF          = 0b001
FIR_SELECT_INPUT_SEL      = 0b010
FIR_SELECT_MEM_WRITE_EN   = 0b011
FIR_SELECT_CHAIN_WRITE_EN = 0b100

# ── IQ Demod selects ───────────────────────────────────────────────────────────

DEMOD_REF_NCO1   = 0
DEMOD_REF_NCO2   = 1

DEMOD_IN_NCO1    = 0b000
DEMOD_IN_NCO2    = 0b001
DEMOD_IN_ADC_A   = 0b010
DEMOD_IN_ADC_B   = 0b011
DEMOD_IN_I_FEED  = 0b100
DEMOD_IN_Q_FEED  = 0b101
DEMOD_IN_FIR     = 0b110

CAPTURE_DEMOD    = 0b1000

# ── IO routing selects ─────────────────────────────────────────────────────────

DAC_SEL_REGISTER = 0b000
DAC_SEL_PID      = 0b001
DAC_SEL_NCO1     = 0b010
DAC_SEL_NCO2     = 0b011

PID_SEL_IFEED = 0b000
PID_SEL_QFEED = 0b001
PID_SEL_ADCA  = 0b010
PID_SEL_ADCB  = 0b011

# ── Check-signed register selects ─────────────────────────────────────────────

CS_ADC_A     = 0b00000
CS_ADC_B     = 0b00001
CS_DEMOD_LPF = 0b01001
CS_IO        = 0b10000

# ── Constants ──────────────────────────────────────────────────────────────────

STRIDE_CONST = 7629.39453125  # 125e6 / (4 * 4096)

# Cycles to hold each strobe phase.
# The 3-FF synchroniser + posedge detector + 2 pipeline stages for the
# callback to reflect the new coefficients requires ~6 cycles from input.
# 8 gives comfortable margin.
SYNC_CYCLES = 8

# ── Result tracking ────────────────────────────────────────────────────────────

_results: list[tuple[str, bool]] = []


def _section(title: str) -> None:
    print(f"\n{'─' * 64}")
    print(f"  {title}")
    print(f"{'─' * 64}")


def _check(label: str, ok: bool, detail: str = "") -> bool:
    tag = "PASS" if ok else "FAIL"
    msg = f"  [{tag}] {label}"
    if detail:
        msg += f"  ({detail})"
    print(msg)
    _results.append((label, ok))
    return ok


def _check_approx(label: str, got: float, expected: float, tol: float) -> bool:
    ok = abs(got - expected) <= tol
    return _check(label, ok, f"got={got:.5f}  expected={expected:.5f}  tol=±{tol}")


def _summary() -> None:
    passed = sum(ok for _, ok in _results)
    total  = len(_results)
    print(f"\n{'═' * 64}")
    print(f"  Results: {passed}/{total} checks passed")
    failed = [lbl for lbl, ok in _results if not ok]
    if failed:
        print("  Failed:")
        for lbl in failed:
            print(f"    ✗  {lbl}")
    print(f"{'═' * 64}")


# ── Numeric helpers ────────────────────────────────────────────────────────────

def _as_signed16(v: int) -> int:
    v = int(v) & 0xFFFF
    return v - 0x10000 if v >= 0x8000 else v


def _float_to_q15(x: float) -> int:
    """Convert float in [-1, 1) to signed Q15 integer."""
    if x >= 0.999969482421875:
        return 0x7FFF
    if x <= -1.0:
        return -32768
    return max(-32768, min(32767, round(x * 32768.0)))


def _float_to_q13(x: float) -> int:
    """Convert float in [-1, 1) to signed Q13 integer."""
    return max(-8192, min(8191, round(x * 8192.0)))


def _float_to_q10(x: float) -> int:
    """Convert float gain to signed Q10 integer (range [-32, 32))."""
    return max(-32768, min(32767, round(x * 1024.0)))


# ── AXI word packing ───────────────────────────────────────────────────────────

def _word(rst: int, strobe: int, cmd: int, data: int) -> int:
    """Pack the 32-bit AXI word presented to axi_from_ps_i."""
    return (
        ((rst    & 0x1)       << 31) |
        ((strobe & 0x1)       << 30) |
        ((cmd    & 0xF)       << 26) |
        ((data   & 0x3FFFFFF)      )
    )


# ── Protocol helpers ───────────────────────────────────────────────────────────

async def _reset(dut) -> None:
    """Assert the synchronous/async reset then release it."""
    dut.axi_from_ps_i.value = _word(1, 0, CMD_IDLE, 0)
    await ClockCycles(dut.clk, 10)
    dut.axi_from_ps_i.value = _word(0, 0, CMD_IDLE, 0)
    await ClockCycles(dut.clk, 10)


async def _send(dut, cmd: int, data: int) -> int:
    """
    Two-step strobe sequence; returns the callback value captured after the
    strobe=1 phase has propagated through the synchroniser pipeline.

    Sequence (each phase lasts SYNC_CYCLES clock cycles):
        strobe=0  →  strobe=1  →  strobe=0
    """
    dut.axi_from_ps_i.value = _word(0, 0, cmd, data)
    await ClockCycles(dut.clk, SYNC_CYCLES)

    dut.axi_from_ps_i.value = _word(0, 1, cmd, data)
    await ClockCycles(dut.clk, SYNC_CYCLES)
    cb = int(dut.axi_to_ps_o.value)

    dut.axi_from_ps_i.value = _word(0, 0, cmd, data)
    await ClockCycles(dut.clk, SYNC_CYCLES)
    return cb


# ── Callback field extractors ──────────────────────────────────────────────────
#
# Each function takes the raw 32-bit callback and returns a named tuple/dict
# of fields.  Bit positions are taken directly from pdh_core.sv.

def _cb_cmd(cb: int) -> int:
    return (cb >> 28) & 0xF

def _cb_led(cb: int) -> dict:
    return {"led": cb & 0xFF, "cmd": _cb_cmd(cb)}

def _cb_dac(cb: int) -> dict:
    return {"dac1": (cb >> 14) & 0x3FFF, "dac2": cb & 0x3FFF, "cmd": _cb_cmd(cb)}

def _cb_adc(cb: int) -> dict:
    return {"adca": cb & 0x3FFF, "adcb": (cb >> 14) & 0x3FFF, "cmd": _cb_cmd(cb)}

def _cb_nco(cb: int) -> dict:
    # {CMD, 1'd0, stride[11:0], shift[11:0], sub, inv, en}
    return {
        "en":     (cb >>  0) & 0x1,
        "inv":    (cb >>  1) & 0x1,
        "sub":    (cb >>  2) & 0x1,
        "shift":  (cb >>  3) & 0xFFF,
        "stride": (cb >> 15) & 0xFFF,
        "cmd":    _cb_cmd(cb),
    }

def _cb_rot(cb: int) -> dict:
    # {CMD, sin[15:2], cos[15:2]}
    cos14 = (cb >>  0) & 0x3FFF
    sin14 = (cb >> 14) & 0x3FFF
    # Bottom 2 bits are truncated in the callback; reconstruct Q15 approximation
    cos_q15 = _as_signed16(cos14 << 2)
    sin_q15 = _as_signed16(sin14 << 2)
    return {
        "cos": cos_q15 / 32768.0,
        "sin": sin_q15 / 32768.0,
        "cmd": _cb_cmd(cb),
    }

def _cb_pid(cb: int) -> dict:
    # {CMD, 8'd0, coeff_sel[3:0], payload[15:0]}
    return {
        "payload":   cb & 0xFFFF,
        "coeff_sel": (cb >> 16) & 0xF,
        "cmd":       _cb_cmd(cb),
    }

def _cb_config_io(cb: int) -> dict:
    # {CMD, 19'b0, pid_sel[2:0], dac2_sel[2:0], dac1_sel[2:0]}
    return {
        "dac1": (cb >> 0) & 0x7,
        "dac2": (cb >> 3) & 0x7,
        "pid":  (cb >> 6) & 0x7,
        "cmd":  _cb_cmd(cb),
    }

def _cb_cs(cb: int) -> dict:
    # {CMD, 7'd0, reg_sel[4:0], 16-bit payload}
    return {
        "payload": _as_signed16(cb & 0xFFFF),
        "reg_sel": (cb >> 16) & 0x1F,
        "cmd":     _cb_cmd(cb),
    }

def _cb_fir(cb: int) -> dict:
    # {CMD, 9'd0, update_sel[2:0], payload[15:0]}
    return {
        "payload":    cb & 0xFFFF,
        "update_sel": (cb >> 16) & 0x7,
        "cmd":        _cb_cmd(cb),
    }

def _cb_config_demod(cb: int) -> dict:
    # {CMD_CONFIG_DEMOD, 20'b0, lpf_alpha[3:0], in_sel[2:0], ref_sel[0]}
    return {
        "ref_sel":   (cb >> 0) & 0x1,
        "in_sel":    (cb >> 1) & 0x7,
        "lpf_alpha": (cb >> 4) & 0xF,
        "cmd":       _cb_cmd(cb),
    }


# ── Main test ──────────────────────────────────────────────────────────────────

@cocotb.test()
async def test_pdh_core(dut):
    """
    Command-echo regression test for pdh_core.

    Exercises every command that has an observable callback echo and checks
    that the FPGA register round-trips match what was sent.  DMA/frame
    capture is out of scope for this lightweight RTL sim.
    """
    global _results
    _results = []

    # Start 125 MHz clock (8 ns period)
    cocotb.start_soon(Clock(dut.clk, 8, unit="ns").start())

    # Tie off DMA-related inputs to benign values
    dut.dma_ready_i.value  = 1
    dut.bram_ready_i.value = 1
    dut.adc_dat_a_i.value  = 0
    dut.adc_dat_b_i.value  = 0

    # ── 1. Reset ───────────────────────────────────────────────────────────────
    _section("1. FPGA Reset")
    await _reset(dut)
    _check("reset: led_o cleared",        int(dut.led_o.value) == 0)
    _check("reset: rst_o deasserted",     int(dut.rst_o.value) == 0)

    # ── 2. Set LED ─────────────────────────────────────────────────────────────
    _section("2. Set LED")
    LED_PAT = 0b10101010
    cb = await _send(dut, CMD_SET_LED, LED_PAT & 0xFF)
    f  = _cb_led(cb)
    _check("set_led: led_o matches",   int(dut.led_o.value) == LED_PAT,
           f"got=0x{int(dut.led_o.value):02X}")
    _check("set_led: echo matches",    f["led"] == LED_PAT,
           f"got=0x{f['led']:02X}")
    _check("set_led: cmd echo",        f["cmd"] == CMD_SET_LED)

    # ── 3. Config IO — both DACs from register, PID from ADC_A ────────────────
    _section("3. Config IO")
    io_data = DAC_SEL_REGISTER | (DAC_SEL_REGISTER << 3) | (PID_SEL_ADCA << 6)
    cb = await _send(dut, CMD_CONFIG_IO, io_data)
    f  = _cb_config_io(cb)
    _check("config_io: dac1 sel echo", f["dac1"] == DAC_SEL_REGISTER)
    _check("config_io: dac2 sel echo", f["dac2"] == DAC_SEL_REGISTER)
    _check("config_io: pid sel echo",  f["pid"]  == PID_SEL_ADCA)
    _check("config_io: cmd echo",      f["cmd"]  == CMD_CONFIG_IO)

    # ── 4. Set DAC ─────────────────────────────────────────────────────────────
    # The C server negates the voltage before converting, so we replicate that.
    # dac_cb_w = {CMD, dac1_dat_r[13:0], dac2_dat_r[13:0]}
    _section("4. Set DAC")

    DAC1_V   = 0.6
    dac1_code = round((-DAC1_V + 1.0) / 2.0 * 16383)   # server negation
    dac1_data = dac1_code & 0x3FFF                       # dac_sel bit14 = 0 → DAC1
    cb = await _send(dut, CMD_SET_DAC, dac1_data)
    f  = _cb_dac(cb)
    _check("set_dac1: code echo", f["dac1"] == dac1_code,
           f"got={f['dac1']} exp={dac1_code}")
    _check("set_dac1: cmd echo",  f["cmd"]  == CMD_SET_DAC)

    DAC2_V   = -0.4
    dac2_code = round((-DAC2_V + 1.0) / 2.0 * 16383)
    dac2_data = (dac2_code & 0x3FFF) | (1 << 14)        # dac_sel bit14 = 1 → DAC2
    cb = await _send(dut, CMD_SET_DAC, dac2_data)
    f  = _cb_dac(cb)
    _check("set_dac2: code echo", f["dac2"] == dac2_code,
           f"got={f['dac2']} exp={dac2_code}")
    _check("set_dac2: cmd echo",  f["cmd"]  == CMD_SET_DAC)

    # ── 5. Get ADC ─────────────────────────────────────────────────────────────
    # adc_cb_w = {CMD, adc_dat_b_i[13:0], adc_dat_a_i[13:0]}
    _section("5. Get ADC")
    ADC_A_CODE = 0x1234 & 0x3FFF
    ADC_B_CODE = 0x0ABC & 0x3FFF
    dut.adc_dat_a_i.value = ADC_A_CODE
    dut.adc_dat_b_i.value = ADC_B_CODE
    await ClockCycles(dut.clk, 4)   # let signals settle through combinatorial path

    cb = await _send(dut, CMD_GET_ADC, 0)
    f  = _cb_adc(cb)
    _check("get_adc: adc_a echo", f["adca"] == ADC_A_CODE,
           f"got=0x{f['adca']:04X} exp=0x{ADC_A_CODE:04X}")
    _check("get_adc: adc_b echo", f["adcb"] == ADC_B_CODE,
           f"got=0x{f['adcb']:04X} exp=0x{ADC_B_CODE:04X}")
    _check("get_adc: cmd echo",   f["cmd"]  == CMD_GET_ADC)

    # ── 6. Set NCO ─────────────────────────────────────────────────────────────
    # Replicates cmd_set_nco() phase arithmetic from control.c.
    # set_nco_cb_w = {CMD, 1'd0, stride[11:0], shift[11:0], sub, inv, en}
    _section("6. Set NCO")
    NCO_FREQ      = 2_000_000.0
    NCO_SHIFT_DEG = -120.0
    NCO_EN        = 1

    stride = round(NCO_FREQ / STRIDE_CONST)
    shift_rad = (-NCO_SHIFT_DEG) / 180.0 * math.pi     # server negates shift_deg
    if shift_rad < 0:
        shift_rad += 2 * math.pi
    inv, sub = 0, 0
    if shift_rad > math.pi:
        shift_rad -= math.pi
        inv ^= 1
    if shift_rad > math.pi / 2:
        sub = 1
        inv ^= 1
        shift_rad = -(shift_rad - math.pi)
    inv &= 1
    shift_int = abs(round(shift_rad / (math.pi / 2) * 4095))

    async def _nco(sel, val=0):
        return await _send(dut, CMD_SET_NCO, (sel << 16) | (val & 0xFFF))

    await _nco(NCO_SELECT_EN,     0)          # disable first
    await _nco(NCO_SELECT_STRIDE, stride)
    await _nco(NCO_SELECT_SHIFT,  shift_int)
    await _nco(NCO_SELECT_INV,    inv)
    await _nco(NCO_SELECT_SUB,    sub)
    cb = await _nco(NCO_SELECT_EN, NCO_EN)  # enable — final CB carries all state

    f = _cb_nco(cb)
    _check("nco: stride echo", f["stride"] == stride,
           f"got={f['stride']} exp={stride}")
    _check("nco: shift echo",  f["shift"]  == shift_int,
           f"got={f['shift']} exp={shift_int}")
    _check("nco: inv echo",    f["inv"]    == inv)
    _check("nco: sub echo",    f["sub"]    == sub)
    _check("nco: en echo",     f["en"]     == NCO_EN)
    _check("nco: cmd echo",    f["cmd"]    == CMD_SET_NCO)

    # ── 7. Set Rotation ────────────────────────────────────────────────────────
    # set_rot_cb_w = {CMD, sin[15:2], cos[15:2]}
    # (bottom 2 bits of each coefficient are truncated in the callback)
    _section("7. Set Rotation")
    ROT_DEG  = 45.0
    theta    = math.radians(ROT_DEG)
    cos_q15  = _float_to_q15(math.cos(theta))
    sin_q15  = _float_to_q15(math.sin(theta))

    # rot_select=0 → cos, rot_select=1 → sin
    await _send(dut, CMD_SET_ROT_COEFFS, (cos_q15 & 0xFFFF) | (0 << 16))
    cb = await _send(dut, CMD_SET_ROT_COEFFS, (sin_q15 & 0xFFFF) | (1 << 16))
    f  = _cb_rot(cb)
    _check_approx("set_rot: cos echo", f["cos"], math.cos(theta), 0.01)
    _check_approx("set_rot: sin echo", f["sin"], math.sin(theta), 0.01)
    _check("set_rot: cmd echo", f["cmd"] == CMD_SET_ROT_COEFFS)

    cb = await _send(dut, CMD_COMMIT_ROT_COEFFS, 0)
    _check("commit_rot: cmd echo", _cb_cmd(cb) == CMD_COMMIT_ROT_COEFFS)

    # ── 8. Set PID coefficients ────────────────────────────────────────────────
    # set_pid_cb_w = {CMD, 8'd0, coeff_sel[3:0], payload[15:0]}
    _section("8. Set PID (disabled)")
    PID_KP    =  0.5
    PID_KD    =  0.2
    PID_KI    =  0.2
    PID_SP    = -0.657
    PID_DEC   = 100
    PID_ALPHA =  2
    PID_SAT   = 18
    PID_EN    =  0

    kp_i = _float_to_q15(PID_KP)
    kd_i = _float_to_q15(PID_KD)
    ki_i = _float_to_q15(PID_KI)
    sp_i = _float_to_q13(PID_SP)

    async def _pid(sel, val16):
        return await _send(dut, CMD_SET_PID_COEFFS, (sel << 16) | (val16 & 0xFFFF))

    cb = await _pid(PID_SELECT_KP, kp_i & 0xFFFF)
    f  = _cb_pid(cb)
    _check_approx("pid: kp echo", _as_signed16(f["payload"]) / 32768.0, PID_KP, 0.01)

    cb = await _pid(PID_SELECT_KD, kd_i & 0xFFFF)
    f  = _cb_pid(cb)
    _check_approx("pid: kd echo", _as_signed16(f["payload"]) / 32768.0, PID_KD, 0.01)

    cb = await _pid(PID_SELECT_KI, ki_i & 0xFFFF)
    f  = _cb_pid(cb)
    _check_approx("pid: ki echo", _as_signed16(f["payload"]) / 32768.0, PID_KI, 0.01)

    cb = await _pid(PID_SELECT_DEC, PID_DEC & 0xFFFF)
    f  = _cb_pid(cb)
    _check("pid: dec echo",   f["payload"] == PID_DEC,   f"got={f['payload']}")

    cb = await _pid(PID_SELECT_SP, sp_i & 0xFFFF)
    f  = _cb_pid(cb)
    _check_approx("pid: sp echo", _as_signed16(f["payload"]) / 8192.0, PID_SP, 0.01)

    cb = await _pid(PID_SELECT_ALPHA, PID_ALPHA & 0xFFFF)
    f  = _cb_pid(cb)
    _check("pid: alpha echo", f["payload"] == PID_ALPHA, f"got={f['payload']}")

    cb = await _pid(PID_SELECT_SAT, PID_SAT & 0xFFFF)
    f  = _cb_pid(cb)
    _check("pid: sat echo",   f["payload"] == PID_SAT,   f"got={f['payload']}")

    cb = await _pid(PID_SELECT_EN, PID_EN & 0xFFFF)
    f  = _cb_pid(cb)
    _check("pid: en echo",    f["payload"] == PID_EN,    f"got={f['payload']}")

    PID_GAIN = 2.5
    gain_i = _float_to_q10(PID_GAIN)
    cb = await _pid(PID_SELECT_GAIN, gain_i & 0xFFFF)
    f  = _cb_pid(cb)
    _check_approx("pid: gain echo",
                  _as_signed16(f["payload"]) / 1024.0, PID_GAIN, 0.002)

    PID_BIAS = 0.5  # volts
    bias_i = max(-8191, min(8191, round(PID_BIAS * 8191.0)))
    cb = await _pid(PID_SELECT_BIAS, bias_i & 0xFFFF)
    f  = _cb_pid(cb)
    _check_approx("pid: bias echo",
                  _as_signed16(f["payload"]) / 8191.0, PID_BIAS, 0.001)

    PID_EGAIN = 1.5
    egain_i = _float_to_q10(PID_EGAIN)
    cb = await _pid(PID_SELECT_EGAIN, egain_i & 0xFFFF)
    f  = _cb_pid(cb)
    _check_approx("pid: egain echo",
                  _as_signed16(f["payload"]) / 1024.0, PID_EGAIN, 0.002)

    # Reset bias and egain for subsequent sections
    await _pid(PID_SELECT_BIAS,  0)
    await _pid(PID_SELECT_EGAIN, _float_to_q10(1.0) & 0xFFFF)

    # ── 9. Config IO — NCO drives DACs ────────────────────────────────────────
    _section("9. Config IO — NCO drives DACs")
    io_data = DAC_SEL_NCO1 | (DAC_SEL_NCO2 << 3) | (PID_SEL_ADCA << 6)
    cb = await _send(dut, CMD_CONFIG_IO, io_data)
    f  = _cb_config_io(cb)
    _check("config_io nco: dac1 → NCO1", f["dac1"] == DAC_SEL_NCO1)
    _check("config_io nco: dac2 → NCO2", f["dac2"] == DAC_SEL_NCO2)
    _check("config_io nco: pid ← ADC_A", f["pid"]  == PID_SEL_ADCA)

    # Let the NCO run for a while and verify the output is non-zero
    await ClockCycles(dut.clk, 200)
    samples = []
    for _ in range(8):
        await RisingEdge(dut.clk)
        samples.append(int(dut.dac_dat_o.value))
    nco_nonzero = any(s != 0 for s in samples)
    _check("nco output is non-zero", nco_nonzero, f"samples={samples[:4]}")

    # ── 10. Check signed — register readback ───────────────────────────────────
    _section("10. Check Signed")
    for label, reg_sel in [("IO", CS_IO), ("ADC_A", CS_ADC_A), ("ADC_B", CS_ADC_B)]:
        cb = await _send(dut, CMD_CHECK_SIGNED, reg_sel & 0x1F)
        f  = _cb_cs(cb)
        _check(f"check_signed {label}: reg_sel echo", f["reg_sel"] == reg_sel,
               f"got={f['reg_sel']}")
        _check(f"check_signed {label}: cmd echo",     f["cmd"] == CMD_CHECK_SIGNED)

    # ── 11. Set FIR ────────────────────────────────────────────────────────────
    # set_fir_cb_w = {CMD, 9'd0, update_sel[2:0], payload[15:0]}
    _section("11. Set FIR")

    async def _fir(sel, val=0):
        return await _send(dut, CMD_SET_FIR, (sel << 16) | (val & 0xFFFF))

    FIR_ADDR  = 0x1A          # tap address (5-bit, fits in NTAPS=32)
    FIR_COEFF = 0x5A3C        # arbitrary 16-bit coefficient

    cb = await _fir(FIR_SELECT_ADDR, FIR_ADDR)
    f  = _cb_fir(cb)
    _check("fir: addr echo",       f["payload"]    == FIR_ADDR,         f"got=0x{f['payload']:04X}")
    _check("fir: addr sel echo",   f["update_sel"] == FIR_SELECT_ADDR)
    _check("fir: addr cmd echo",   f["cmd"]        == CMD_SET_FIR)

    cb = await _fir(FIR_SELECT_COEFF, FIR_COEFF)
    f  = _cb_fir(cb)
    _check("fir: coeff echo",      f["payload"]    == FIR_COEFF,        f"got=0x{f['payload']:04X}")
    _check("fir: coeff sel echo",  f["update_sel"] == FIR_SELECT_COEFF)

    cb = await _fir(FIR_SELECT_MEM_WRITE_EN, 1)
    f  = _cb_fir(cb)
    _check("fir: mem_wr_en=1",     f["payload"]    == 1)
    _check("fir: mem_wr sel echo", f["update_sel"] == FIR_SELECT_MEM_WRITE_EN)

    cb = await _fir(FIR_SELECT_MEM_WRITE_EN, 0)
    f  = _cb_fir(cb)
    _check("fir: mem_wr_en=0",     f["payload"]    == 0)

    cb = await _fir(FIR_SELECT_CHAIN_WRITE_EN, 1)
    f  = _cb_fir(cb)
    _check("fir: chain_wr_en=1",   f["payload"]    == 1)
    _check("fir: chain sel echo",  f["update_sel"] == FIR_SELECT_CHAIN_WRITE_EN)

    cb = await _fir(FIR_SELECT_CHAIN_WRITE_EN, 0)
    f  = _cb_fir(cb)
    _check("fir: chain_wr_en=0",   f["payload"]    == 0)

    # ── 12. PID gain functional test ───────────────────────────────────────────
    # Route both DACs to PID output so dac_dat_o == pid_out_w regardless of dac_sel_r.
    # Route PID input from ADC_A.
    # With kp=0.5, ki=kd=0, sp=0, error = -2048:
    #   gain=1.0 → pid_out = 7167  (deviation from midpoint 8191 = -1024)
    #   gain=2.0 → pid_out = 6143  (deviation = -2048 = 2 × -1024)
    #   gain=0.5 → pid_out = 7679  (deviation = -512  = 0.5 × -1024)
    #   gain=4.0 → pid_out = 4095  (linear, gain > 2)
    #   gain=9.0 → pid_out = 0     (saturated at DAC min: 8191 - 9216 = -1025 → 0)
    _section("12. PID gain functional test")

    io_data = DAC_SEL_PID | (DAC_SEL_PID << 3) | (PID_SEL_ADCA << 6)
    await _send(dut, CMD_CONFIG_IO, io_data)

    # Drive ADC_A above the midpoint (0x2000) to produce a known signed error.
    # adc_dat_a_16s_r = -(0x2800 - 0x2000) = -2048
    ADC_GAIN_TEST = 0x2800
    dut.adc_dat_a_i.value = ADC_GAIN_TEST
    await ClockCycles(dut.clk, 4)

    kp_i = _float_to_q15(0.5)

    # Set kp=0.5, ki=kd=0, sp=0, dec=1, alpha=4, sat=31, gain=1.0, then enable.
    await _pid(PID_SELECT_KP,    kp_i & 0xFFFF)
    await _pid(PID_SELECT_KI,    0)
    await _pid(PID_SELECT_KD,    0)
    await _pid(PID_SELECT_SP,    0)
    await _pid(PID_SELECT_DEC,   1)
    await _pid(PID_SELECT_ALPHA, 4)
    await _pid(PID_SELECT_SAT,   31)
    await _pid(PID_SELECT_GAIN,  _float_to_q10(1.0) & 0xFFFF)
    await _pid(PID_SELECT_EN,    1)

    await ClockCycles(dut.clk, 20)
    out_gain1 = int(dut.dac_dat_o.value)

    await _pid(PID_SELECT_GAIN, _float_to_q10(2.0) & 0xFFFF)
    await ClockCycles(dut.clk, 20)
    out_gain2 = int(dut.dac_dat_o.value)

    await _pid(PID_SELECT_GAIN, _float_to_q10(0.5) & 0xFFFF)
    await ClockCycles(dut.clk, 20)
    out_gain_half = int(dut.dac_dat_o.value)

    await _pid(PID_SELECT_GAIN, _float_to_q10(4.0) & 0xFFFF)
    await ClockCycles(dut.clk, 20)
    out_gain4 = int(dut.dac_dat_o.value)

    await _pid(PID_SELECT_GAIN, _float_to_q10(9.0) & 0xFFFF)
    await ClockCycles(dut.clk, 20)
    out_gain9 = int(dut.dac_dat_o.value)

    # Restore and disable.
    await _pid(PID_SELECT_GAIN, _float_to_q10(1.0) & 0xFFFF)
    await _pid(PID_SELECT_EN, 0)

    # The PID DAC offset is 8191.  Signed deviation from that midpoint scales with gain.
    PID_MID = 8191
    dev1    = out_gain1    - PID_MID
    dev2    = out_gain2    - PID_MID
    dev_half= out_gain_half - PID_MID

    _check("gain=1.0: pid output != midpoint", dev1 != 0,
           f"pid_out={out_gain1}  dev={dev1}")
    _check("gain=1.0: pid output exact",       out_gain1 == 7167,
           f"got={out_gain1}  exp=7167")
    _check("gain=2.0: pid output exact",       out_gain2 == 6143,
           f"got={out_gain2}  exp=6143")
    _check("gain=0.5: pid output exact",       out_gain_half == 7679,
           f"got={out_gain_half}  exp=7679")
    _check_approx("gain=2.0 doubles deviation",   float(dev2),    dev1 * 2.0,   1.0)
    _check_approx("gain=0.5 halves deviation",    float(dev_half), dev1 * 0.5,  1.0)
    _check("gain=4.0: pid output exact",          out_gain4 == 4095,
           f"got={out_gain4}  exp=4095")
    _check("gain=9.0: output saturates to 0",     out_gain9 == 0,
           f"got={out_gain9}  exp=0")

    # ── 13. PID bias functional test ───────────────────────────────────────────
    # With gain=1.0 and a fixed error, adding bias shifts the DAC output by the
    # expected number of codes (bias_volts * 8191 DAC codes).
    # Setup: kp=0.5, ki=kd=0, sp=0, ADC_A=0x2800 → signed error = -2048
    #   bias=0    → pid_out = 7167  (verified by section 12)
    # The physical DAC output is inverted (higher code → more negative voltage),
    # so pid_core stores -bias_i in bias_r to compensate.  A positive bias setting
    # therefore SUBTRACTS codes from pid_out, shifting the physical output positive.
    #   bias=+0.5V → offset = -4095 codes → pid_out = 7167 - 4095 = 3072
    #   bias=-0.5V → offset = +4095 codes → pid_out = 7167 + 4095 = 11262
    _section("13. PID bias functional test")

    io_data = DAC_SEL_PID | (DAC_SEL_PID << 3) | (PID_SEL_ADCA << 6)
    await _send(dut, CMD_CONFIG_IO, io_data)

    dut.adc_dat_a_i.value = ADC_GAIN_TEST  # 0x2800, same as section 12
    await ClockCycles(dut.clk, 4)

    kp_i = _float_to_q15(0.5)
    await _pid(PID_SELECT_KP,    kp_i & 0xFFFF)
    await _pid(PID_SELECT_KI,    0)
    await _pid(PID_SELECT_KD,    0)
    await _pid(PID_SELECT_SP,    0)
    await _pid(PID_SELECT_DEC,   1)
    await _pid(PID_SELECT_ALPHA, 4)
    await _pid(PID_SELECT_SAT,   31)
    await _pid(PID_SELECT_GAIN,  _float_to_q10(1.0) & 0xFFFF)
    await _pid(PID_SELECT_BIAS,  0)
    await _pid(PID_SELECT_EN,    1)
    await ClockCycles(dut.clk, 20)
    out_bias0 = int(dut.dac_dat_o.value)

    bias_pos = max(-8191, min(8191, round(0.5 * 8191.0)))
    await _pid(PID_SELECT_BIAS, bias_pos & 0xFFFF)
    await ClockCycles(dut.clk, 20)
    out_bias_pos = int(dut.dac_dat_o.value)

    bias_neg = max(-8191, min(8191, round(-0.5 * 8191.0)))
    await _pid(PID_SELECT_BIAS, bias_neg & 0xFFFF)
    await ClockCycles(dut.clk, 20)
    out_bias_neg = int(dut.dac_dat_o.value)

    await _pid(PID_SELECT_BIAS, 0)
    await _pid(PID_SELECT_EN,   0)

    _check("bias=0: output matches section 12", out_bias0 == 7167,
           f"got={out_bias0}  exp=7167")
    _check_approx("bias=+0.5V: output shifted down by ~4095 codes",
                  float(out_bias_pos - out_bias0), -4095.0, 2.0)
    _check_approx("bias=-0.5V: output shifted up by ~4095 codes",
                  float(out_bias_neg - out_bias0), 4095.0, 2.0)

    # ── 14. PID egain functional + err_tap verification ───────────────────────
    # Same setup as section 12 (kp=0.5, ki=kd=0, sp=0, ADC_A=0x2800 → error=-2048,
    # gain=1.0). egain scales the error BEFORE Kp:
    #   egain=1.0 → error_gained=-2048 → pid_out=7167  (same as gain=1.0 in §12)
    #   egain=2.0 → error_gained=-4096 → pid_out=6143  (same as gain=2.0 in §12)
    #   egain=0.5 → error_gained=-1024 → pid_out=7679  (same as gain=0.5 in §12)
    # err_tap (error_pipe1_r) reflects the gained error directly.
    _section("14. PID egain functional + err_tap")

    io_data = DAC_SEL_PID | (DAC_SEL_PID << 3) | (PID_SEL_ADCA << 6)
    await _send(dut, CMD_CONFIG_IO, io_data)

    ADC_EGAIN_TEST = 0x2800
    dut.adc_dat_a_i.value = ADC_EGAIN_TEST
    await ClockCycles(dut.clk, 4)

    kp_i = _float_to_q15(0.5)
    await _pid(PID_SELECT_KP,    kp_i & 0xFFFF)
    await _pid(PID_SELECT_KI,    0)
    await _pid(PID_SELECT_KD,    0)
    await _pid(PID_SELECT_SP,    0)
    await _pid(PID_SELECT_DEC,   1)
    await _pid(PID_SELECT_ALPHA, 4)
    await _pid(PID_SELECT_SAT,   31)
    await _pid(PID_SELECT_GAIN,  _float_to_q10(1.0) & 0xFFFF)
    await _pid(PID_SELECT_EGAIN, _float_to_q10(1.0) & 0xFFFF)
    await _pid(PID_SELECT_EN,    1)
    await ClockCycles(dut.clk, 20)
    out_egain1    = int(dut.dac_dat_o.value)
    err_egain1    = _as_signed16(int(dut.u_pid.err_tap.value))

    await _pid(PID_SELECT_EGAIN, _float_to_q10(2.0) & 0xFFFF)
    await ClockCycles(dut.clk, 20)
    out_egain2    = int(dut.dac_dat_o.value)
    err_egain2    = _as_signed16(int(dut.u_pid.err_tap.value))

    await _pid(PID_SELECT_EGAIN, _float_to_q10(0.5) & 0xFFFF)
    await ClockCycles(dut.clk, 20)
    out_egain_half = int(dut.dac_dat_o.value)
    err_egain_half = _as_signed16(int(dut.u_pid.err_tap.value))

    await _pid(PID_SELECT_EGAIN, _float_to_q10(1.0) & 0xFFFF)
    await _pid(PID_SELECT_EN,    0)

    _check("egain=1.0: pid output exact",   out_egain1     == 7167,
           f"got={out_egain1}  exp=7167")
    _check("egain=2.0: pid output exact",   out_egain2     == 6143,
           f"got={out_egain2}  exp=6143")
    _check("egain=0.5: pid output exact",   out_egain_half == 7679,
           f"got={out_egain_half}  exp=7679")
    _check("egain=1.0: err_tap exact",      err_egain1     == -2048,
           f"got={err_egain1}  exp=-2048")
    _check("egain=2.0: err_tap exact",      err_egain2     == -4096,
           f"got={err_egain2}  exp=-4096")
    _check("egain=0.5: err_tap exact",      err_egain_half == -1024,
           f"got={err_egain_half}  exp=-1024")
    _check_approx("egain=2.0 doubles err_tap", float(err_egain2),    err_egain1 * 2.0, 1.0)
    _check_approx("egain=0.5 halves err_tap",  float(err_egain_half), err_egain1 * 0.5, 1.0)

    # ── 15. IRQ output verification ────────────────────────────────────────────
    # Verify irq_o fires correctly:
    #   - One 1-cycle pulse after strobe_edge_w for non-IDLE, non-GET_FRAME cmds
    #   - Silent for CMD_IDLE
    #   - Fires on dma_ready_i rising edge (not on strobe) for CMD_GET_FRAME
    _section("15. IRQ output")

    async def _strobe_watch_irq(cmd: int, data: int, window: int = SYNC_CYCLES) -> bool:
        """Present strobe 0→1, return True if irq_o pulses at any point within window."""
        dut.axi_from_ps_i.value = _word(0, 0, cmd, data)
        await ClockCycles(dut.clk, SYNC_CYCLES)
        dut.axi_from_ps_i.value = _word(0, 1, cmd, data)
        saw_high = False
        for _ in range(window):
            await RisingEdge(dut.clk)
            if int(dut.irq_o.value) == 1:
                saw_high = True
        dut.axi_from_ps_i.value = _word(0, 0, cmd, data)
        await ClockCycles(dut.clk, SYNC_CYCLES)
        return saw_high

    # 15a. Normal commands must fire irq_o
    _check("irq: fires for CMD_SET_LED",    await _strobe_watch_irq(CMD_SET_LED,    0xAA))
    _check("irq: fires for CMD_CONFIG_IO",  await _strobe_watch_irq(CMD_CONFIG_IO,  0))
    _check("irq: fires for CMD_GET_ADC",    await _strobe_watch_irq(CMD_GET_ADC,    0))
    _check("irq: fires for CMD_SET_DAC",    await _strobe_watch_irq(CMD_SET_DAC,    0))

    # 15b. CMD_IDLE must NOT fire irq_o
    _check("irq: silent for CMD_IDLE",      not await _strobe_watch_irq(CMD_IDLE, 0))

    # 15c. CMD_GET_FRAME: irq fires on dma_ready_i rising edge, not on strobe
    dut.dma_ready_i.value = 0
    await ClockCycles(dut.clk, 8)   # flush dma_ready through 3FF → goes to 0

    # Strobe CMD_GET_FRAME; irq_o must stay low during and after the strobe
    dut.axi_from_ps_i.value = _word(0, 0, CMD_GET_FRAME, 0)
    await ClockCycles(dut.clk, SYNC_CYCLES)
    dut.axi_from_ps_i.value = _word(0, 1, CMD_GET_FRAME, 0)
    irq_during_strobe = False
    for _ in range(SYNC_CYCLES):
        await RisingEdge(dut.clk)
        if int(dut.irq_o.value) == 1:
            irq_during_strobe = True
    dut.axi_from_ps_i.value = _word(0, 0, CMD_GET_FRAME, 0)
    await ClockCycles(dut.clk, SYNC_CYCLES)
    _check("irq: silent during GET_FRAME strobe", not irq_during_strobe)

    # Now raise dma_ready_i; irq_o must fire within 3FF latency + 1 edge cycle = ~6
    dut.dma_ready_i.value = 1
    irq_after_dma = False
    for _ in range(8):
        await RisingEdge(dut.clk)
        if int(dut.irq_o.value) == 1:
            irq_after_dma = True
            break
    _check("irq: fires on dma_ready_i rising edge (GET_FRAME)", irq_after_dma)
    dut.dma_ready_i.value = 1   # leave ready=1 for any subsequent tests

    # ── 16. Config Demod — command echo ───────────────────────────────────────
    # config_demod_cb_w = {CMD_CONFIG_DEMOD, 20'b0, lpf_alpha[3:0], in_sel[2:0], ref_sel[0]}
    # data bits: [0]=ref_sel, [3:1]=in_sel, [7:4]=lpf_alpha
    _section("16. Config Demod (echo)")

    async def _config_demod(ref_sel: int, in_sel: int, lpf_alpha: int = 8) -> int:
        data = ((lpf_alpha & 0xF) << 4) | ((in_sel & 0x7) << 1) | (ref_sel & 0x1)
        return await _send(dut, CMD_CONFIG_DEMOD, data)

    cb = await _config_demod(DEMOD_REF_NCO1, DEMOD_IN_I_FEED, lpf_alpha=8)
    f  = _cb_config_demod(cb)
    _check("config_demod: ref_sel NCO1 echo",   f["ref_sel"]   == DEMOD_REF_NCO1,
           f"got={f['ref_sel']} exp={DEMOD_REF_NCO1}")
    _check("config_demod: in_sel I_FEED echo",  f["in_sel"]    == DEMOD_IN_I_FEED,
           f"got={f['in_sel']} exp={DEMOD_IN_I_FEED}")
    _check("config_demod: lpf_alpha=8 echo",    f["lpf_alpha"] == 8,
           f"got={f['lpf_alpha']} exp=8")
    _check("config_demod: cmd echo",            f["cmd"]       == CMD_CONFIG_DEMOD)

    cb = await _config_demod(DEMOD_REF_NCO2, DEMOD_IN_ADC_A, lpf_alpha=10)
    f  = _cb_config_demod(cb)
    _check("config_demod2: ref_sel NCO2 echo",  f["ref_sel"]   == DEMOD_REF_NCO2,
           f"got={f['ref_sel']} exp={DEMOD_REF_NCO2}")
    _check("config_demod2: in_sel ADC_A echo",  f["in_sel"]    == DEMOD_IN_ADC_A,
           f"got={f['in_sel']} exp={DEMOD_IN_ADC_A}")
    _check("config_demod2: lpf_alpha=10 echo",  f["lpf_alpha"] == 10,
           f"got={f['lpf_alpha']} exp=10")
    _check("config_demod2: cmd echo",           f["cmd"]       == CMD_CONFIG_DEMOD)

    # ── 17. IQ Demod functional — in-phase and quadrature ─────────────────────
    # Uses the NCO's two outputs as self-contained stimulus:
    #   Test A (in-phase):  NCO shift=0  → NCO1=NCO2=cos(ωt)
    #                       ref=NCO1, in=NCO1 → cos²(ωt) → DC mean ≈ amplitude²/2 > 5000
    #   Test B (quadrature): NCO shift=4095 → NCO2 ≈ sin(ωt)
    #                        ref=NCO1, in=NCO2 → ½sin(2ωt) → mean ≈ 0 < 2000
    #                        std > 100 confirms the multiplier is live
    _section("17. IQ Demod functional (in-phase and quadrature)")

    DEMOD_STRIDE = 100        # ~762.9 kHz; period ≈ 164 clock cycles
    DEMOD_SAMPLES = 1000      # ≈6 full NCO periods — enough for stable mean
    DEMOD_IN_PHASE_THRESH  = 5000
    DEMOD_QUAD_MEAN_THRESH = 2000
    DEMOD_QUAD_STD_THRESH  = 100

    # Set frame code to CAPTURE_DEMOD.
    # Frame packing: {demod_lpf_r[63:48], demod_out_w[47:32], demod_ref_w[31:16], demod_in_w[15:0]}
    # demod_out_w is in bits [47:32].
    await _send(dut, CMD_GET_FRAME, (CAPTURE_DEMOD << 22) | 1)

    # ── Test A: in-phase (shift=0, NCO1=NCO2) ─────────────────────────────────
    await _nco(NCO_SELECT_EN,     0)          # disable while reconfiguring
    await _nco(NCO_SELECT_STRIDE, DEMOD_STRIDE)
    await _nco(NCO_SELECT_SHIFT,  0)
    await _nco(NCO_SELECT_INV,    0)
    await _nco(NCO_SELECT_SUB,    0)
    await _nco(NCO_SELECT_EN,     1)

    await _config_demod(DEMOD_REF_NCO1, DEMOD_IN_NCO1)
    await ClockCycles(dut.clk, 32)     # allow NCO and demod pipeline to fill

    samples_a = []
    for _ in range(DEMOD_SAMPLES):
        await RisingEdge(dut.clk)
        samples_a.append(_as_signed16((int(dut.dma_data_o.value) >> 32) & 0xFFFF))

    mean_a = sum(samples_a) / len(samples_a)
    _check("demod in-phase: mean(demod_out) > threshold",
           mean_a > DEMOD_IN_PHASE_THRESH,
           f"mean={mean_a:.1f}  threshold={DEMOD_IN_PHASE_THRESH}")

    # ── Test B: quadrature (shift≈90°, NCO1=cos, NCO2=sin) ────────────────────
    await _nco(NCO_SELECT_EN,     0)
    await _nco(NCO_SELECT_SHIFT,  4095)    # ≈89.98° offset; phi2 = phi1 + 4095
    await _nco(NCO_SELECT_EN,     1)

    await _config_demod(DEMOD_REF_NCO1, DEMOD_IN_NCO2)
    await ClockCycles(dut.clk, 32)     # pipeline settle

    samples_b = []
    for _ in range(DEMOD_SAMPLES):
        await RisingEdge(dut.clk)
        samples_b.append(_as_signed16((int(dut.dma_data_o.value) >> 32) & 0xFFFF))

    mean_b  = sum(samples_b) / len(samples_b)
    var_b   = sum((x - mean_b) ** 2 for x in samples_b) / len(samples_b)
    std_b   = var_b ** 0.5

    _check("demod quadrature: abs(mean(demod_out)) < threshold",
           abs(mean_b) < DEMOD_QUAD_MEAN_THRESH,
           f"mean={mean_b:.1f}  threshold=±{DEMOD_QUAD_MEAN_THRESH}")
    _check("demod quadrature: std(demod_out) > threshold (live output)",
           std_b > DEMOD_QUAD_STD_THRESH,
           f"std={std_b:.1f}  threshold={DEMOD_QUAD_STD_THRESH}")

    # Restore NCO to disabled state
    await _nco(NCO_SELECT_EN, 0)

    # ── 18. Demod LPF via CMD_CHECK_SIGNED ────────────────────────────────────
    # EMA LPF runs at 125 MHz on demod_out_w.
    # With alpha=8, τ = 256 / 125e6 ≈ 2 µs → 5τ ≈ 1280 cycles.
    # Use in-phase NCO (shift=0): demod_out = cos²(ωt) → DC ≈ 16383.
    # After ~2000 cycles the LPF should settle to the same DC level.
    _section("18. Demod LPF — CMD_CHECK_SIGNED")

    # Configure: NCO at DEMOD_STRIDE, shift=0; demod ref=NCO1, in=NCO1, alpha=8
    await _nco(NCO_SELECT_EN,     0)
    await _nco(NCO_SELECT_STRIDE, DEMOD_STRIDE)
    await _nco(NCO_SELECT_SHIFT,  0)
    await _nco(NCO_SELECT_EN,     1)
    await _config_demod(DEMOD_REF_NCO1, DEMOD_IN_NCO1, lpf_alpha=8)

    # Wait ≥ 5τ (1280 cycles at alpha=8) for the EMA to settle
    await ClockCycles(dut.clk, 2000)

    # Read back demod_lpf_r via CMD_CHECK_SIGNED with CS_DEMOD_LPF
    cb = await _send(dut, CMD_CHECK_SIGNED, CS_DEMOD_LPF & 0x1F)
    f  = _cb_cs(cb)
    _check("demod_lpf check_signed: reg_sel echo",
           f["reg_sel"] == CS_DEMOD_LPF,
           f"got={f['reg_sel']} exp={CS_DEMOD_LPF}")
    _check("demod_lpf check_signed: cmd echo",
           f["cmd"] == CMD_CHECK_SIGNED)
    _check("demod_lpf check_signed: LPF settled to DC > threshold  [cos²(ωt) → DC ≈ 16383]",
           f["payload"] > 5000,
           f"payload={f['payload']}  threshold=5000")

    # Disable NCO again
    await _nco(NCO_SELECT_EN, 0)

    # ── Summary ────────────────────────────────────────────────────────────────
    _summary()
    failed = [lbl for lbl, ok in _results if not ok]
    assert not failed, f"Failed checks: {failed}"
