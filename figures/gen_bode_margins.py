"""
Generate figures/bode_margins.pdf
Classic two-panel Bode plot with Phase Margin and Gain Margin annotated.

Synthetic loop: proportional gain K through a single-pole low-pass plant
    L(s) = K / (1 + s/w0)
with a pure transport delay of tau seconds.
This gives a clean, analytically tractable example.
"""

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import os

# ── System parameters ─────────────────────────────────────────────────────────
# L(jω) = K / (1 + jω/ω₀) · e^{-jωτ}
# Chosen so PM ≈ 45° and GM ≈ 12 dB — a realistic, stable loop.
K   = 4.0               # loop gain
w0  = 2 * np.pi * 800   # plant pole at 800 Hz
tau = 25e-6             # 25 µs pure delay (physical DAC→ADC path)

freqs = np.logspace(2, 5, 2000)   # 100 Hz to 100 kHz
omega = 2 * np.pi * freqs

# Loop transfer function L(jw) = K/(1 + jw/w0) * exp(-j*w*tau)
L = (K / (1 + 1j * omega / w0)) * np.exp(-1j * omega * tau)

gain_db  = 20 * np.log10(np.abs(L))
phase_raw = np.angle(L, deg=True)

# Unwrap phase
phase_unwrap = np.unwrap(np.angle(L)) * 180 / np.pi

# ── Find crossover frequencies by interpolation ───────────────────────────────
def interp_cross(freqs, y, threshold):
    """Return (f, y_at_f) where y first crosses threshold going downward."""
    for i in range(len(y) - 1):
        if y[i] >= threshold > y[i + 1]:
            frac = (threshold - y[i]) / (y[i + 1] - y[i])
            f_cross = freqs[i] + frac * (freqs[i + 1] - freqs[i])
            return f_cross
    return None

f_gc = interp_cross(freqs, np.abs(L), 1.0)   # gain crossover (|L|=1)
f_pc = interp_cross(freqs, phase_unwrap, -180.0)   # phase crossover

# Interpolate values at crossovers
pm_deg = None
gm_db  = None
if f_gc is not None:
    idx = np.searchsorted(freqs, f_gc)
    frac = (f_gc - freqs[idx-1]) / (freqs[idx] - freqs[idx-1])
    phase_at_gc = phase_unwrap[idx-1] + frac * (phase_unwrap[idx] - phase_unwrap[idx-1])
    pm_deg = 180.0 + phase_at_gc
if f_pc is not None:
    idx = np.searchsorted(freqs, f_pc)
    frac = (f_pc - freqs[idx-1]) / (freqs[idx] - freqs[idx-1])
    gain_at_pc = gain_db[idx-1] + frac * (gain_db[idx] - gain_db[idx-1])
    gm_db = -gain_at_pc

print(f"f_gc = {f_gc/1e3:.2f} kHz   PM = {pm_deg:.1f}°")
print(f"f_pc = {f_pc/1e3:.2f} kHz   GM = {gm_db:.1f} dB")

# ── Plot ──────────────────────────────────────────────────────────────────────
fig, (ax_gain, ax_phase) = plt.subplots(2, 1, figsize=(7, 6),
                                        sharex=True)
fig.subplots_adjust(hspace=0.12)

# ── Gain panel ────────────────────────────────────────────────────────────────
ax_gain.semilogx(freqs, gain_db, color="#1f77b4", linewidth=2)
ax_gain.axhline(0, color="gray", linewidth=0.8, linestyle="--")
if f_gc:
    ax_gain.axvline(f_gc, color="#ff7f0e", linewidth=1.0, linestyle=":")
if f_pc:
    ax_gain.axvline(f_pc, color="#d62728", linewidth=1.0, linestyle=":")
    # GM double-headed arrow
    idx = np.searchsorted(freqs, f_pc)
    frac = (f_pc - freqs[idx-1]) / (freqs[idx] - freqs[idx-1])
    g_at_pc = gain_db[idx-1] + frac * (gain_db[idx] - gain_db[idx-1])
    ax_gain.annotate(
        "", xy=(f_pc, 0), xytext=(f_pc, g_at_pc),
        arrowprops=dict(arrowstyle="<->", color="#d62728", lw=1.5)
    )
    ax_gain.text(f_pc * 1.12, g_at_pc / 2, f"GM = {gm_db:.0f} dB",
                 color="#d62728", fontsize=8, va="center")

ax_gain.set_ylabel("Gain  (dB)", fontsize=10)
ax_gain.set_ylim(-60, 35)
ax_gain.grid(True, alpha=0.3, which="both")
ax_gain.set_title("Open-Loop Bode Plot — Stability Margins", fontsize=11)

# labels for vertical lines
if f_gc:
    ax_gain.text(f_gc * 1.05, ax_gain.get_ylim()[0] + 3,
                 f"$f_{{gc}}$", color="#ff7f0e", fontsize=9)
if f_pc:
    ax_gain.text(f_pc * 1.05, ax_gain.get_ylim()[0] + 3,
                 f"$f_{{pc}}$", color="#d62728", fontsize=9)

# ── Phase panel ───────────────────────────────────────────────────────────────
ax_phase.semilogx(freqs, phase_unwrap, color="#1f77b4", linewidth=2)
ax_phase.axhline(-180, color="gray", linewidth=0.8, linestyle="--")
if f_gc:
    ax_phase.axvline(f_gc, color="#ff7f0e", linewidth=1.0, linestyle=":")
    # PM double-headed arrow at f_gc
    idx = np.searchsorted(freqs, f_gc)
    frac = (f_gc - freqs[idx-1]) / (freqs[idx] - freqs[idx-1])
    ph_at_gc = phase_unwrap[idx-1] + frac * (phase_unwrap[idx] - phase_unwrap[idx-1])
    ax_phase.annotate(
        "", xy=(f_gc, -180), xytext=(f_gc, ph_at_gc),
        arrowprops=dict(arrowstyle="<->", color="#ff7f0e", lw=1.5)
    )
    ax_phase.text(f_gc * 1.12, (-180 + ph_at_gc) / 2,
                  f"PM = {pm_deg:.0f}°",
                  color="#ff7f0e", fontsize=8, va="center")
if f_pc:
    ax_phase.axvline(f_pc, color="#d62728", linewidth=1.0, linestyle=":")

ax_phase.set_ylabel("Phase  (°)", fontsize=10)
ax_phase.set_xlabel("Frequency  (Hz)", fontsize=10)
ax_phase.set_ylim(-360, 10)
ax_phase.set_yticks([-360, -270, -180, -90, 0])
ax_phase.grid(True, alpha=0.3, which="both")

# ── x-axis labels ─────────────────────────────────────────────────────────────
ax_phase.set_xlim(freqs[0], freqs[-1])

out = os.path.join(os.path.dirname(__file__), "bode_margins.pdf")
fig.savefig(out, bbox_inches="tight")
print(f"Saved {out}")
