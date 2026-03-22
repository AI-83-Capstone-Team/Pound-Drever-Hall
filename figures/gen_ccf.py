"""
Generate figures/ccf_diagram.pdf

Closed-loop pure-I controller simulation using exact Q15 arithmetic matching
pid_core.sv (validated by test_pdh_core.py section 12 cocotb test).

Physical motivation
───────────────────
In a PDH laser lock the actuator (piezo/current) integrates the error signal;
a pure-I controller is therefore natural.  In steady state the integrator
accumulates past errors and drives the plant to counteract them.

The CCF CCF(pid_out, err)[k] = (1/N) Σ po_z[n]·er_z[n+k] / (σ_po σ_er) shows
how pid_out at time n is related to err at time n+k.  Because the integral
remembers the past but not the future:

  CCF[k ≈ 0]          ≈  0      (pid_out is accumulated history, uncorrelated
                                   with the current instantaneous error)
  CCF[k = loop_delay] ≈  -1     (the accumulated correction is precisely
                                   anticorrelated with future error reduction:
                                   the more the controller pushed, the more the
                                   plant reduced the error at time n+k)

ADC/DAC sign convention (mirrors pdh_core.sv lines 778-781):
  dat_i = -(adc_dat_a_i - ADC_OFFSET)     (hardware negation)
Higher pid_out → higher adc_raw → lower dat_i → error decreases: negative feedback.

DMA frame signal definitions (pid_io_frame in hw_common.h):
  pid_in   = pid_in_w    (raw PID input = dat_i)
  err      = err_tap_w   = error_pipe1_r  (one-cycle-delayed error)
  pid_out  = pid_out_w   (14-bit unsigned output)
"""

import math
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import os

# ── Exact Q15/Q10 saturating helpers (mirror pid_core.sv) ─────────────────────

def _clamp(x, lo, hi):
    return max(lo, min(hi, int(x)))

def _sat16(x):
    return _clamp(x, -32768, 32767)

def _sat20(x):
    return _clamp(x, -524288, 524287)

def _sat14u(x):
    return _clamp(x, 0, 16383)

def _q15(f):
    return _clamp(round(f * 32768), -32768, 32767)

def _q10(f):
    return _clamp(round(f * 1024), -32768, 32767)


# ── Simulation parameters ─────────────────────────────────────────────────────

RNG_SEED   = 42
N_WARMUP   = 2000      # samples discarded (let integrator reach steady state)
N          = 16384     # samples recorded

# Controller — pure I (KP = 0):
# With no P-term there is no trivial +1 cross-correlation at the pipeline lag;
# the CCF is determined entirely by the I-loop dynamics and shows a dominant
# negative peak at the correction lag.
KP         = 0.0
KI         = 0.5       # integral gain
ALPHA_EXP  = 3         # EMA exponent (unused with KD=0)
SAT_WIDTH  = 15        # minimum valid value → maximum I sensitivity
#   effective I gain = KI / 2^(SAT_WIDTH-15) = KI / 1 = KI
GAIN       = 1.0

# Plant: instant first-order (PLANT_ALPHA=0 → no IIR lag) with loop delay.
# Closed-loop coefficient: KI * PLANT_GAIN / 2^(SAT_WIDTH-15) ≈ 0.5 * 2.0 = 1.0
# (near critical, fast tracking, clean negative CCF peak)
LOOP_DELAY = 4         # samples from pid_out to adc_raw
PLANT_GAIN = 2.0       # ADC counts per pid_out_centered count
PLANT_ALPHA= 0.0       # instant plant (no IIR smoothing)

# Noise: white, zero mean
DIST_STD   = 150       # driving noise (ADC counts)
MEAS_STD   = 0         # measurement noise (0 → cleanest CCF)

DAC_OFFSET = 8191
ADC_OFFSET = 8192

# ── Run simulation ────────────────────────────────────────────────────────────

rng = np.random.default_rng(RNG_SEED)

# Pre-generate noise
dist_noise = rng.normal(0, DIST_STD, N + N_WARMUP)
meas_noise = rng.normal(0, MEAS_STD, N + N_WARMUP) if MEAS_STD > 0 else \
             np.zeros(N + N_WARMUP)

ki_i        = _q15(KI)
gain_i      = _q10(GAIN)
egain_i     = _q10(1.0)
sat_thresh  = 1 << SAT_WIDTH

# PID state (registers in pid_core.sv)
sum_err_r   = 0
err_pipe1_r = 0    # error_pipe1_r: delayed error used by P/D and captured as err_tap
yk_r        = 0
i_err_r     = 0
pid_out     = DAC_OFFSET

# Plant state and delay buffer
plant_state    = 0.0
pid_delay_buf  = [DAC_OFFSET] * (LOOP_DELAY + 2)

pid_out_arr    = []
err_tap_arr    = []   # records err_pipe1_r (= err_tap_w in DMA frame)

for n in range(N + N_WARMUP):
    # ── Plant: maps delayed pid_out to adc_raw ────────────────────────────────
    delayed_pid  = pid_delay_buf[0]
    u_c          = delayed_pid - DAC_OFFSET          # pid_out_centered
    plant_state  = (PLANT_ALPHA * plant_state
                    + (1.0 - PLANT_ALPHA) * PLANT_GAIN * u_c)

    adc_raw = _clamp(
        ADC_OFFSET + int(round(plant_state))
                   + int(round(dist_noise[n]))
                   + int(round(meas_noise[n])),
        0, 16383
    )

    # ── RTL ADC negation (pdh_core.sv L778-781) ───────────────────────────────
    tmp_s = -((adc_raw & 0x3FFF) - ADC_OFFSET)      # 15-bit signed
    dat_i = _sat16(tmp_s)

    # ── pid_core.sv arithmetic, decimation=1, all ticks active ───────────────
    error_w      = _sat16(dat_i - 0)                 # setpoint = 0
    egain_prod   = error_w * egain_i                  # egain=1.0 → no change
    error_gained = _sat16(egain_prod >> 10)

    # Integrator with anti-windup
    sum_wide = sum_err_r + error_gained
    if   sum_wide >=  sat_thresh: sum1 = sat_thresh - 1
    elif sum_wide <= -sat_thresh: sum1 = -sat_thresh + 1
    else:                          sum1 = sum_wide
    if (not (pid_out == 16383 and sum1 > sum_err_r) and
        not (pid_out == 0     and sum1 < sum_err_r)):
        sum2 = sum1
    else:
        sum2 = sum_err_r

    # EMA (for derivative; KD=0 here so unused in output)
    err_minus_ema = error_gained - yk_r
    yk_new = _sat16((err_minus_ema >> ALPHA_EXP) + yk_r)

    # I product — uses PREVIOUS cycle's pipeline state
    i_err_new = ki_i * sum_err_r     # 32-bit Q30

    # Shift I to 16-bit (SAT_WIDTH=15 → shift by 15 → i_shifted = KI * sum_err_r)
    i_shifted = _sat16(i_err_r >> SAT_WIDTH)

    # P = 0 (KP = 0)
    total    = _sat20(i_shifted)
    gain_pr  = total * gain_i
    g_shift  = _sat20(gain_pr >> 10)
    pid_out  = _sat14u(g_shift + DAC_OFFSET)

    # Advance registers
    sum_err_r   = sum2
    err_pipe1_r = error_gained    # will be captured as err_tap_w next cycle
    yk_r        = yk_new
    i_err_r     = i_err_new

    # Advance plant delay buffer
    pid_delay_buf.append(pid_out)
    pid_delay_buf.pop(0)

    if n >= N_WARMUP:
        pid_out_arr.append(pid_out)
        err_tap_arr.append(err_pipe1_r)   # err_tap_w = error_pipe1_r (DMA frame)

# ── Compute CCF  (identical formula to api_control_metrics in api.py) ─────────

pid_out_np = np.array(pid_out_arr, dtype=float)
err_np     = np.array(err_tap_arr,  dtype=float)

po_z = pid_out_np - pid_out_np.mean()
er_z = err_np     - err_np.mean()

denom = float(np.std(po_z) * np.std(er_z))
if denom > 0:
    # np.correlate(er_z, po_z)[N-1+k] = Σ po_z[n]·er_z[n+k]  (positive k = future err)
    # This convention: high pid_out[n] → low err[n+loop_delay] → CCF[+loop_delay] < 0
    ccf_raw = np.correlate(er_z, po_z, mode='full') / (denom * N)
else:
    ccf_raw = np.zeros(2 * N - 1)

lag_samp    = np.arange(-(N - 1), N)
fs_eff      = 125e6        # dec=1

# Find the dominant NEGATIVE peak (causal anticorrelation: high pid_out → low future err)
peak_idx    = int(np.argmin(ccf_raw))
peak_val    = float(ccf_raw[peak_idx])
peak_samp   = int(lag_samp[peak_idx])
peak_us     = peak_samp / fs_eff * 1e6

print(f"CCF negative peak: {peak_val:.4f}  lag = {peak_samp} samples  ({peak_us:.4f} µs)")
print(f"CCF at lag 0: {ccf_raw[N - 1]:.4f}")

# ── Plot ──────────────────────────────────────────────────────────────────────

fig, (ax_sig, ax_ccf) = plt.subplots(2, 1, figsize=(7.5, 5.5))
fig.subplots_adjust(hspace=0.45)

# ── Panel A: 400-sample snippet of the time series ────────────────────────────
SHOW = 400
t_us = np.arange(SHOW) / fs_eff * 1e6

po_show = pid_out_np[:SHOW] - pid_out_np[:SHOW].mean()
er_show = err_np[:SHOW]     - err_np[:SHOW].mean()
scale   = max(np.max(np.abs(po_show)), np.max(np.abs(er_show)), 1.0)

ax_sig.plot(t_us, er_show / scale, color="#d62728", linewidth=1.2, alpha=0.9,
            label=r"err  ($\approx$ err\_tap\_w)")
ax_sig.plot(t_us, po_show / scale, color="#1f77b4", linewidth=1.2, alpha=0.9,
            label=r"pid\_out  (centred, normalised)")
ax_sig.axhline(0, color="gray", linewidth=0.5, linestyle="--")

ax_sig.set_xlabel("Time  (µs)", fontsize=10)
ax_sig.set_ylabel("Normalised amplitude", fontsize=10)
ax_sig.set_title(
    rf"(A)  Closed-loop time series  "
    rf"($K_i = {KI}$, loop delay $= {LOOP_DELAY}$ samples, dec $= 1$)",
    loc="left", fontsize=9, fontweight="bold")
ax_sig.legend(fontsize=8, loc="upper right")
ax_sig.grid(True, alpha=0.3)
ax_sig.set_xlim(0, t_us[-1])

# ── Panel B: CCF — show lags -3 to +20 so the negative causal peak at +loop_delay
#    is prominent and the positive integrator-memory peak (at -loop_delay) is off-screen
WIN_LO, WIN_HI = -3, 20
mask = (lag_samp >= WIN_LO) & (lag_samp <= WIN_HI)

ax_ccf.plot(lag_samp[mask], ccf_raw[mask], color="#444444", linewidth=1.5)
ax_ccf.axhline( 0, color="gray",    linewidth=0.7, linestyle="--")
ax_ccf.axhline(-1, color="#d62728", linewidth=0.7, linestyle=":",
               alpha=0.6, label=r"$\pm 1$ bound")
ax_ccf.axhline( 1, color="#1f77b4", linewidth=0.7, linestyle=":", alpha=0.6)

# Mark the dominant peak if within display range
if WIN_LO <= peak_samp <= WIN_HI:
    ax_ccf.axvline(peak_samp, color="#ff7f0e", linewidth=1.5, linestyle="--",
                   label=f"Peak: CCF = {peak_val:.2f}  (lag = {peak_samp} samples)")

ax_ccf.set_xlabel("Lag  (samples)", fontsize=10)
ax_ccf.set_ylabel("CCF  (normalised)", fontsize=10)
ax_ccf.set_title(
    r"(B)  Cross-correlation  CCF(pid\_out, err)$[\,k\,]$",
    loc="left", fontsize=10, fontweight="bold")
ax_ccf.legend(fontsize=8, loc="lower right")
ax_ccf.grid(True, alpha=0.3)
ax_ccf.set_xlim(WIN_LO, WIN_HI)
ax_ccf.set_ylim(-1.15, 1.15)

out = os.path.join(os.path.dirname(__file__), "ccf_diagram.pdf")
fig.savefig(out, bbox_inches="tight")
print(f"Saved {out}")
