"""
Generate figures/settling_overshoot.pdf
Annotated time-domain diagram showing how settling time and overshoot
are defined, matching the exact algorithm in api_control_metrics().
"""

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import os

rng = np.random.default_rng(42)

# ── Synthetic error signal ────────────────────────────────────────────────────
N  = 400
t  = np.linspace(0, 1.0, N)   # seconds (arbitrary units, looks like ms)
e_ss = 5.0                     # steady-state error (non-zero to make it visible)

# Exponentially decaying oscillation + steady-state + small noise
decay  = np.exp(-6 * t)
osc    = decay * 60 * np.sin(2 * np.pi * 4 * t)
noise  = rng.normal(0, 0.8, N) * (0.05 + 0.95 * decay)
err    = e_ss + osc + noise

# ── Reproduce the settling-time algorithm ────────────────────────────────────
steady   = float(np.mean(err[-(N // 5):]))
peak_abs = float(np.max(np.abs(err)))
band     = 0.05 * peak_abs

in_band  = np.abs(err - steady) <= band
t_settle = None
for i in range(N):
    if np.all(in_band[i:]):
        t_settle = t[i]
        break

# Overshoot (first 20%)
transient  = err[: N // 5]
peak_range = float(np.max(transient) - np.min(transient))
overshoot  = (float(np.max(transient)) - steady) / peak_range * 100.0
peak_val   = float(np.max(transient))
peak_idx   = int(np.argmax(transient))

print(f"steady={steady:.2f}  band=±{band:.2f}  t_settle={t_settle:.3f}s")
print(f"overshoot={overshoot:.1f}%  peak={peak_val:.2f}")

# ── Plot ──────────────────────────────────────────────────────────────────────
fig, ax = plt.subplots(figsize=(8, 4.5))

t_ms = t * 1000   # display in ms for readability

# Shaded transient region (first 20%)
ax.axvspan(t_ms[0], t_ms[N // 5 - 1], color="#ffeeba", alpha=0.55,
           label="Transient window (first 20%)")

# Error signal
ax.plot(t_ms, err, color="#1f77b4", linewidth=1.2, label="Error $e[n]$")

# Steady-state line
ax.axhline(steady, color="#2ca02c", linewidth=1.5, linestyle="-",
           label=f"$e_{{ss}}$ = {steady:.1f}")

# ±5% band
ax.axhline(steady + band, color="#2ca02c", linewidth=1.0, linestyle="--",
           label=f"±5% band (±{band:.1f})")
ax.axhline(steady - band, color="#2ca02c", linewidth=1.0, linestyle="--")

# Settling time
if t_settle is not None:
    ax.axvline(t_settle * 1e3, color="#d62728", linewidth=1.5, linestyle=":",
               label=f"Settling time $t_s$ = {t_settle*1e3:.1f} ms")

# Overshoot annotation
ax.annotate(
    "", xy=(t_ms[peak_idx], peak_val),
    xytext=(t_ms[peak_idx], steady),
    arrowprops=dict(arrowstyle="<->", color="#ff7f0e", lw=1.8)
)
ax.text(t_ms[peak_idx] + 8, (peak_val + steady) / 2,
        f"Overshoot\n= {overshoot:.0f}%",
        color="#ff7f0e", fontsize=8, va="center")

# Peak range bracket
min_t = float(np.argmin(transient))
min_v = float(np.min(transient))
ax.annotate(
    "", xy=(t_ms[int(min_t)], min_v),
    xytext=(t_ms[int(min_t)], peak_val),
    arrowprops=dict(arrowstyle="<->", color="gray", lw=1.2, linestyle="dashed")
)
ax.text(t_ms[int(min_t)] - 30, (peak_val + min_v) / 2,
        "peak\nrange", color="gray", fontsize=7, ha="center", va="center")

ax.set_xlabel("Time  (ms)", fontsize=10)
ax.set_ylabel("Error  (ADC counts)", fontsize=10)
ax.set_title("Settling Time and Overshoot", fontsize=11)
ax.legend(fontsize=8, loc="upper right")
ax.grid(True, alpha=0.3)
ax.set_xlim(t_ms[0], t_ms[-1])

out = os.path.join(os.path.dirname(__file__), "settling_overshoot.pdf")
fig.savefig(out, bbox_inches="tight")
print(f"Saved {out}")
