"""
PDH board-level regression test (loopback configuration).

Hardware setup assumed:
    DAC1 → ADC1 (IN1)
    DAC2 → ADC2 (IN2)

Run from the client/ directory:
    python test.py

Each section exercises one API and verifies the FPGA callback echoes.
Frame captures are plotted at the end so the user can visually inspect
signal integrity.  The final summary lists any failed checks.
"""

import math
import os
import time

import matplotlib
matplotlib.use("Qt5Agg")
import matplotlib.pyplot as plt
import numpy as np

from pdh_api import (
    DacDatSel, DacSel, FirInputSel, FrameCode, PidDatSel, CsSel,
    api_check_signed, api_config_io, api_get_adc, api_get_frame,
    api_reset_fpga, api_set_dac, api_set_led,
    api_set_fir, api_set_nco, api_set_pid, api_set_rotation,
)

REMOTE_DIR = "sw/build"
FIR_COEFFS_CSV = os.path.join(os.path.dirname(__file__), "..", "test_resources", "fir_coeffs_example.csv")

# ── Test parameters ────────────────────────────────────────────────────────────

DAC1_TEST_V      = 0.6     # Volts, set on DAC1 for loopback read-back
DAC2_TEST_V      = -0.4    # Volts, set on DAC2 for loopback read-back
ADC_TOL_V        = 0.05    # ±50 mV acceptable ADC/DAC round-trip error

NCO_FREQ         = 2_000_000.0   # Hz
NCO_SHIFT_DEG    = -120.0        # degrees
NCO_STRIDE_CONST = 7629.395      # Hz per stride (125e6 / (4 * 4096))

ROT_THETA_DEG    = 45.0          # degrees for rotation matrix verification

PID_KP    = 0.5
PID_KD    = 0.2
PID_KI    = 0.2
PID_SP    = -0.657   # setpoint (Q13 normalised = volts at convergence)
PID_DEC   = 100
PID_ALPHA = 2
PID_SAT   = 18

LED_PATTERN = 0b10101010

# ── Helpers ────────────────────────────────────────────────────────────────────

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
    return _check(label, ok, f"got={got:.4f}  expected={expected:.4f}  tol=±{tol}")


def _summary() -> None:
    passed = sum(ok for _, ok in _results)
    total  = len(_results)
    print(f"\n{'═' * 64}")
    print(f"  Results: {passed}/{total} checks passed")
    failed = [(lbl, ok) for lbl, ok in _results if not ok]
    if failed:
        print("  Failed:")
        for lbl, _ in failed:
            print(f"    ✗  {lbl}")
    print(f"{'═' * 64}")


def _plot_frame(result, title: str, ylabel: str = "Code") -> plt.Figure:
    """Generic single-axes frame plot."""
    fig, ax = plt.subplots(figsize=(11, 4))
    t = np.arange(len(result.data))
    for i, col in enumerate(result.columns):
        ax.plot(t, result.data[:, i], label=col, linewidth=0.7)
    ax.set_title(title)
    ax.set_xlabel("Sample")
    ax.set_ylabel(ylabel)
    ax.legend()
    ax.grid(True, alpha=0.4)
    fig.tight_layout()
    return fig


# ── Test sequence ──────────────────────────────────────────────────────────────

def main() -> None:
    figs: list[plt.Figure] = []

    # ── 1. FPGA reset ──────────────────────────────────────────────────────────
    _section("1. FPGA Reset")
    rst = api_reset_fpga()
    _check("reset_fpga: status == 0", rst.status == 0)
    time.sleep(0.01)

    # ── 2. LED ─────────────────────────────────────────────────────────────────
    _section("2. LED")
    led = api_set_led(LED_PATTERN)
    _check(
        "set_led: echo matches",
        led.led_code_cb == LED_PATTERN,
        f"sent=0x{LED_PATTERN:02X}  got=0x{led.led_code_cb:02X}",
    )

    # ── 3. IO: both DACs from register ────────────────────────────────────────
    _section("3. IO Config — DACs from register")
    io = api_config_io(DacDatSel.REGISTER, DacDatSel.REGISTER, PidDatSel.ADC_A)
    _check("dac1_dat_sel echo", io.dac1_dat_sel_cb == int(DacDatSel.REGISTER))
    _check("dac2_dat_sel echo", io.dac2_dat_sel_cb == int(DacDatSel.REGISTER))
    _check("pid_dat_sel echo",  io.pid_dat_sel_cb  == int(PidDatSel.ADC_A))

    # ── 4. DAC / ADC loopback ─────────────────────────────────────────────────
    _section("4. DAC / ADC Loopback")
    api_set_dac(DAC1_TEST_V, DacSel.DAC_1)
    api_set_dac(DAC2_TEST_V, DacSel.DAC_2)
    adc = api_get_adc()
    print(f"  ADC IN1:  code={adc.in1_code}   V={adc.in1_v:.4f}")
    print(f"  ADC IN2:  code={adc.in2_code}   V={adc.in2_v:.4f}")
    _check_approx("ADC IN1 ≈ DAC1", adc.in1_v, DAC1_TEST_V, ADC_TOL_V)
    _check_approx("ADC IN2 ≈ DAC2", adc.in2_v, DAC2_TEST_V, ADC_TOL_V)

    # LOOPBACK frame: shows what the FPGA is writing to the DACs and reading
    # back from the ADCs in real time — visual sanity check of the path.
    _section("4b. Frame: LOOPBACK (DAC feed vs ADC input — static DC)")
    lb = api_get_frame(1, FrameCode.LOOPBACK, REMOTE_DIR)
    _check("LOOPBACK frame received", lb.data.size > 0)
    if lb.data.size > 0:
        fig, axes = plt.subplots(2, 1, figsize=(11, 6), sharex=True)
        t = np.arange(len(lb.data))
        axes[0].plot(t, lb.data[:, 0], label="dac1_feed", linewidth=0.7)
        axes[0].plot(t, lb.data[:, 1], label="dac2_feed", linewidth=0.7)
        axes[0].set_title(f"LOOPBACK — DAC feeds  (DAC1={DAC1_TEST_V} V, DAC2={DAC2_TEST_V} V)")
        axes[0].set_ylabel("Code")
        axes[0].legend(); axes[0].grid(True, alpha=0.4)
        axes[1].plot(t, lb.data[:, 2], label="adc_a", linewidth=0.7)
        axes[1].plot(t, lb.data[:, 3], label="adc_b", linewidth=0.7)
        axes[1].set_title("ADC inputs (should match DAC feeds through loopback cable)")
        axes[1].set_xlabel("Sample")
        axes[1].set_ylabel("Code")
        axes[1].legend(); axes[1].grid(True, alpha=0.4)
        fig.tight_layout()
        figs.append(fig)

    # ── 5. NCO ────────────────────────────────────────────────────────────────
    _section("5. NCO Configuration")
    nco = api_set_nco(NCO_FREQ, NCO_SHIFT_DEG, en=1)
    expected_stride = round(NCO_FREQ / NCO_STRIDE_CONST)
    print(f"  stride_cb           = {nco.stride_cb}  (expected ≈ {expected_stride})")
    print(f"  registered_freq     = {nco.registered_freq:.1f} Hz  "
          f"(target {NCO_FREQ:.0f} Hz,  error {nco.registered_freq_error:.1f} Hz)")
    print(f"  registered_phase    = {nco.registered_phase_shift:.2f}°  "
          f"(target {NCO_SHIFT_DEG:.2f}°,  error {nco.registered_shift_error:.2f}°)")
    _check("NCO stride echo matches", nco.stride_cb == expected_stride,
           f"got={nco.stride_cb}  expected={expected_stride}")
    _check("NCO freq error < 1 stride",
           abs(nco.registered_freq_error) < NCO_STRIDE_CONST,
           f"{nco.registered_freq_error:.1f} Hz")
    _check("NCO en echo == 1", nco.en_cb == 1)

    # ── 6. Rotation matrix ────────────────────────────────────────────────────
    _section("6. Rotation Matrix")
    rot = api_set_rotation(ROT_THETA_DEG)
    expected_cos = math.cos(math.radians(ROT_THETA_DEG))
    expected_sin = math.sin(math.radians(ROT_THETA_DEG))
    print(f"  cos_cb = {rot.cos_cb:.4f}  (expected {expected_cos:.4f})")
    print(f"  sin_cb = {rot.sin_cb:.4f}  (expected {expected_sin:.4f})")
    # Q15 resolution is ~1/32768 ≈ 3e-5; 2 LSBs lost in the 14-bit CB field → tol ≈ 0.01
    _check_approx("cos echo", rot.cos_cb, expected_cos, 0.01)
    _check_approx("sin echo", rot.sin_cb, expected_sin, 0.01)

    # ── 7. IO: NCO → DACs, ADC_A → PID input ─────────────────────────────────
    _section("7. IO Config — NCO drives DACs")
    io = api_config_io(DacDatSel.NCO_1, DacDatSel.NCO_2, PidDatSel.ADC_A)
    _check("dac1 → NCO_1", io.dac1_dat_sel_cb == int(DacDatSel.NCO_1))
    _check("dac2 → NCO_2", io.dac2_dat_sel_cb == int(DacDatSel.NCO_2))
    _check("pid  ← ADC_A", io.pid_dat_sel_cb  == int(PidDatSel.ADC_A))

    # ── 8. PID coefficients (disabled) ───────────────────────────────────────
    _section("8. PID Configuration (disabled)")
    pid = api_set_pid(
        kp=PID_KP, kd=PID_KD, ki=PID_KI, sp=PID_SP,
        dec=PID_DEC, alpha=PID_ALPHA, sat=PID_SAT, en=0,
    )
    print(f"  kp_cb={pid.kp_cb:.4f}  kd_cb={pid.kd_cb:.4f}  ki_cb={pid.ki_cb:.4f}")
    print(f"  sp_cb={pid.sp_cb:.4f}  dec_cb={pid.dec_cb}  en_cb={pid.en_cb}")
    _check_approx("kp echo", pid.kp_cb, PID_KP, 0.01)
    _check_approx("kd echo", pid.kd_cb, PID_KD, 0.01)
    _check_approx("ki echo", pid.ki_cb, PID_KI, 0.01)
    _check_approx("sp echo", pid.sp_cb, PID_SP, 0.01)
    _check("dec echo", pid.dec_cb == PID_DEC, f"got={pid.dec_cb}")
    _check("en=0 echo", pid.en_cb == 0)

    # ── 9. OSC_INSPECT frame ──────────────────────────────────────────────────
    _section("9. Frame: OSC_INSPECT")
    osc = api_get_frame(1, FrameCode.OSC_INSPECT, REMOTE_DIR)
    _check("OSC_INSPECT frame received", osc.data.size > 0)
    if osc.data.size > 0:
        nco_out_std = osc.data[:, 0].std()
        _check("nco_out1 is oscillating", nco_out_std > 1000,
               f"std={nco_out_std:.0f}")
        figs.append(_plot_frame(
            osc,
            f"OSC_INSPECT — NCO @ {NCO_FREQ/1e6:.1f} MHz, shift={NCO_SHIFT_DEG}°",
        ))

    # ── 10. LOOPBACK frame (NCO driving DACs) ─────────────────────────────────
    _section("10. Frame: LOOPBACK (NCO → DAC → ADC)")
    lb2 = api_get_frame(1, FrameCode.LOOPBACK, REMOTE_DIR)
    _check("LOOPBACK frame (NCO) received", lb2.data.size > 0)
    if lb2.data.size > 0:
        fig, axes = plt.subplots(2, 1, figsize=(11, 6), sharex=True)
        t = np.arange(len(lb2.data))
        axes[0].plot(t, lb2.data[:, 0], label="dac1_feed (NCO1)", linewidth=0.7)
        axes[0].plot(t, lb2.data[:, 1], label="dac2_feed (NCO2)", linewidth=0.7)
        axes[0].set_title(f"LOOPBACK — DAC feeds  (NCO @ {NCO_FREQ/1e6:.1f} MHz)")
        axes[0].set_ylabel("Code")
        axes[0].legend(); axes[0].grid(True, alpha=0.4)
        axes[1].plot(t, lb2.data[:, 2], label="adc_a", linewidth=0.7)
        axes[1].plot(t, lb2.data[:, 3], label="adc_b", linewidth=0.7)
        axes[1].set_title("ADC inputs (loopback: should track DAC feeds with cable delay)")
        axes[1].set_xlabel("Sample")
        axes[1].set_ylabel("Code")
        axes[1].legend(); axes[1].grid(True, alpha=0.4)
        fig.tight_layout()
        figs.append(fig)

    # ── 11. ADC_DATA_IN frame ────────────────────────────────────────────
    _section("11. Frame: ADC_DATA_IN (demodulated IQ)")
    ang = api_get_frame(1, FrameCode.ADC_DATA_IN, REMOTE_DIR)
    _check("ADC_DATA_IN frame received", ang.data.size > 0)
    if ang.data.size > 0:
        fig, axes = plt.subplots(2, 1, figsize=(11, 6), sharex=True)
        t = np.arange(len(ang.data))
        axes[0].plot(t, ang.data[:, 0], label="adc_a", linewidth=0.7)
        axes[0].plot(t, ang.data[:, 1], label="adc_b", linewidth=0.7)
        axes[0].set_title("ADC_DATA_IN — Raw ADC inputs")
        axes[0].set_ylabel("Code (14-bit signed)")
        axes[0].legend(); axes[0].grid(True, alpha=0.4)
        axes[1].plot(t, ang.data[:, 2], label="i_feed", linewidth=0.7)
        axes[1].plot(t, ang.data[:, 3], label="q_feed", linewidth=0.7)
        axes[1].set_title("Demodulated outputs (rotation applied)")
        axes[1].set_xlabel("Sample")
        axes[1].set_ylabel("Code (Q13)")
        axes[1].legend(); axes[1].grid(True, alpha=0.4)
        fig.tight_layout()
        figs.append(fig)

    # ── 12. PID enable + convergence check ───────────────────────────────────
    # PID fed with raw ADC1 (no rotation): pid_dat_sel = ADC_A.
    # At convergence, adc_dat_a_16s = sp_r = PID_SP * 8192, which maps
    # directly to PID_SP volts on the ADC output.
    _section("12. PID Enable + ADC Convergence Check")
    api_config_io(DacDatSel.PID, DacDatSel.NCO_2, PidDatSel.ADC_A)
    api_set_pid(
        kp=PID_KP, kd=PID_KD, ki=PID_KI, sp=PID_SP,
        dec=PID_DEC, alpha=PID_ALPHA, sat=PID_SAT, en=1,
    )
    time.sleep(0.1)   # wait for PID to settle
    adc_locked = api_get_adc()
    print(f"  ADC IN1:  {adc_locked.in1_v:.4f} V  (target {PID_SP} V)")
    _check_approx("ADC IN1 ≈ PID setpoint", adc_locked.in1_v, PID_SP, 0.02)

    err = api_get_frame(PID_DEC, FrameCode.PID_ERR_TAPS, REMOTE_DIR)
    _check("PID_ERR_TAPS frame received", err.data.size > 0)
    if err.data.size > 0:
        err_mean = err.data[:, 0].mean()
        err_std  = err.data[:, 0].std()
        print(f"  Error signal:  mean={err_mean:.1f}  std={err_std:.1f}  (Q13 codes)")
        figs.append(_plot_frame(
            err,
            f"PID_ERR_TAPS — kp={PID_KP}, ki={PID_KI}, sp={PID_SP}  (dec={PID_DEC})",
            ylabel="Code (Q13)",
        ))

    # ── 13. check_signed spot-checks ─────────────────────────────────────────
    _section("13. check_signed register reads")
    cs_io   = api_check_signed(CsSel.IO)
    cs_adca = api_check_signed(CsSel.ADC_A)
    cs_adcb = api_check_signed(CsSel.ADC_B)
    print(f"  IO reg      (sel={int(CsSel.IO):02}): value={cs_io.reg_value_cb}")
    print(f"  ADC_A       (sel={int(CsSel.ADC_A):02}): value={cs_adca.reg_value_cb}")
    print(f"  ADC_B       (sel={int(CsSel.ADC_B):02}): value={cs_adcb.reg_value_cb}")
    _check("check_signed IO sel echo",   cs_io.reg_sel_cb   == int(CsSel.IO))
    _check("check_signed ADC_A sel echo", cs_adca.reg_sel_cb == int(CsSel.ADC_A))
    _check("check_signed ADC_B sel echo", cs_adcb.reg_sel_cb == int(CsSel.ADC_B))

    # ── 14. FIR filter configuration ─────────────────────────────────────────
    _section("14. FIR Filter Configuration")

    # Route NCO1 → DAC1 so the FIR input (ADC1 via loopback) sees a 2 MHz
    # sine wave.  The 100 kHz low-pass FIR should visibly attenuate it.
    api_config_io(DacDatSel.NCO_1, DacDatSel.NCO_2, PidDatSel.ADC_A)

    # Load coefficients from the example CSV
    coeffs: list[float] = []
    with open(FIR_COEFFS_CSV) as f:
        for line in f:
            line = line.strip()
            if line:
                coeffs.append(float(line))

    print(f"  Loading {len(coeffs)} coefficients from {os.path.basename(FIR_COEFFS_CSV)}")
    print(f"  Input select: {FirInputSel.ADC1.name} ({int(FirInputSel.ADC1)})")

    fir = api_set_fir(FIR_COEFFS_CSV, FirInputSel.ADC1)

    _check("set_fir: status == 0",         fir.status == 0)
    _check("set_fir: input_sel echo",      fir.input_sel_cb == int(FirInputSel.ADC1),
           f"got={fir.input_sel_cb}  expected={int(FirInputSel.ADC1)}")
    _check("set_fir: mem_write_en=1 echo", fir.mem_wen_en_cb == 1,
           f"got={fir.mem_wen_en_cb}")
    _check("set_fir: mem_write_en=0 echo", fir.mem_wen_dis_cb == 0,
           f"got={fir.mem_wen_dis_cb}")
    _check("set_fir: chain_write_en=1 echo", fir.chain_wen_cb == 1,
           f"got={fir.chain_wen_cb}")

    # FIR_IO frame: capture FIR input and output
    _section("14b. Frame: FIR_IO")
    fir_frame = api_get_frame(1, FrameCode.FIR_IO, REMOTE_DIR)
    _check("FIR_IO frame received", fir_frame.data.size > 0)
    if fir_frame.data.size > 0:
        figs.append(_plot_frame(
            fir_frame,
            "FIR_IO — filter input vs output (fc=100 kHz low-pass)",
        ))

    # ── Summary ────────────────────────────────────────────────────────────────
    _summary()
    plt.show()


if __name__ == "__main__":
    main()
