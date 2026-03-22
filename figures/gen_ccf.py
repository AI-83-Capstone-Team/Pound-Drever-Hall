"""
Generate figures/ccf_diagram.pdf
Two-panel CCF diagram showing how a peak at positive lag indicates
that pid_out predicts future error reduction.
"""

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import os

rng = np.random.default_rng(7)

fs   = 10e3           # 10 kHz (illustrative)
T    = 0.04           # 40 ms total
N    = int(T * fs)
t    = np.arange(N) / fs * 1e3   # ms

delay_ms = 8.0        # 8 ms loop delay
d_samp   = int(delay_ms * 1e-3 * fs)

# ── Synthetic signals ─────────────────────────────────────────────────────────
# pid_out: controller fires a short burst at t≈5 ms, decays
act_center = int(0.12 * N)
pid_out = np.zeros(N)
for i in range(N):
    if i >= act_center:
        pid_out[i] = 0.9 * np.exp(-(i - act_center) / (0.025 * N))
pid_out += rng.normal(0, 0.025, N)

# err: starts elevated, drops after delay, recovers slowly, plus noise
err = np.zeros(N)
for i in range(N):
    if i < act_center + d_samp:
        err[i] = 0.5 + rng.normal(0, 0.04)
    else:
        decay = np.exp(-(i - act_center - d_samp) / (0.06 * N))
        err[i] = 0.5 * decay + rng.normal(0, 0.04)

# ── Compute CCF ───────────────────────────────────────────────────────────────
po_z  = pid_out - pid_out.mean()
er_z  = err     - err.mean()
denom = np.std(po_z) * np.std(er_z) * N
ccf_raw  = np.correlate(po_z, er_z, mode='full') / (denom if denom > 0 else 1)
lag_samp = np.arange(-(N - 1), N)
lag_ms   = lag_samp / fs * 1e3

peak_idx = int(np.argmax(np.abs(ccf_raw)))
peak_lag = float(lag_ms[peak_idx])
peak_val = float(ccf_raw[peak_idx])

# ── Plot ──────────────────────────────────────────────────────────────────────
fig, (ax_sig, ax_ccf) = plt.subplots(2, 1, figsize=(7.5, 5.5))
fig.subplots_adjust(hspace=0.40)

# ── Top: time series ──────────────────────────────────────────────────────────
ax_sig.plot(t, pid_out, color="#1f77b4", linewidth=1.5,
            label="pid_out  (controller acts)")
ax_sig.plot(t, err,     color="#d62728", linewidth=1.5,
            label="err  (error signal)")

t_act = t[act_center]
t_resp = t[min(act_center + d_samp, N - 1)]
ax_sig.axvline(t_act,  color="#1f77b4", linewidth=0.8, linestyle="--", alpha=0.6)
ax_sig.axvline(t_resp, color="#d62728", linewidth=0.8, linestyle="--", alpha=0.6)
ax_sig.annotate(
    "", xy=(t_resp, 0.55), xytext=(t_act, 0.55),
    arrowprops=dict(arrowstyle="-|>", color="#ff7f0e", lw=1.8)
)
ax_sig.text((t_act + t_resp) / 2, 0.60, f"delay $d$ = {delay_ms:.0f} ms",
            ha="center", color="#ff7f0e", fontsize=9)

ax_sig.set_xlabel("Time  (ms)", fontsize=10)
ax_sig.set_ylabel("Amplitude  (a.u.)", fontsize=10)
ax_sig.set_title("(A)  Controller output and error signal", loc="left",
                 fontsize=10, fontweight="bold")
ax_sig.legend(fontsize=8)
ax_sig.grid(True, alpha=0.3)
ax_sig.set_xlim(t[0], t[-1])

# ── Bottom: CCF ───────────────────────────────────────────────────────────────
# Show only a window around zero for clarity
window_ms = 25
mask = np.abs(lag_ms) < window_ms
ax_ccf.plot(lag_ms[mask], ccf_raw[mask], color="#444444", linewidth=1.5)
ax_ccf.axhline(0, color="gray", linewidth=0.7, linestyle="--")
ax_ccf.axvline(peak_lag, color="#ff7f0e", linewidth=1.5, linestyle="--",
               label=f"Peak lag = {peak_lag:.1f} ms  (CCF = {peak_val:.2f})")

ax_ccf.set_xlabel("Lag  (ms)", fontsize=10)
ax_ccf.set_ylabel("CCF  (normalised)", fontsize=10)
ax_ccf.set_title("(B)  Cross-correlation of pid_out vs err", loc="left",
                 fontsize=10, fontweight="bold")
ax_ccf.legend(fontsize=8)
ax_ccf.grid(True, alpha=0.3)
ax_ccf.set_xlim(-window_ms, window_ms)

out = os.path.join(os.path.dirname(__file__), "ccf_diagram.pdf")
fig.savefig(out, bbox_inches="tight")
print(f"Saved {out}")
