"""
Generate figures/fir_ideal_vs_rtl.pdf

Plots the ideal FIR frequency response (DTFT from float coefficients) against
the response actually measured from the Verilator RTL simulation.

Data source: the most recent fir_freq_results.json produced by
    cd core/hw/sim && python run_fir_freq.py --ntaps 32 --corner 5e6 --window hann

JSON schema (written by test_fir_freq_response.py + run_fir_freq.py):
    freqs_ideal  : list of N_ideal floats (Hz), dense sweep 0..Fs/2
    H_ideal      : list of N_ideal floats, |H| from DTFT of float coefficients
    freqs_rtl    : list of 60 floats (Hz), log-spaced measurement points
    H_rtl        : list of 60 floats, |H| measured by driving RTL with cosines
    ntaps        : int
    fs           : float
    corner_hz    : float  (added by runner)
    window       : str    (added by runner)
"""

import json
import math
import pathlib
import os
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# ── Find most recent sim result ───────────────────────────────────────────────
_here     = pathlib.Path(__file__).resolve().parent
_sim_root = _here.parent / "core" / "hw" / "sim" / "artifacts" / "fir_freq"

candidates = sorted(
    _sim_root.glob("fir_freq_*/fir_freq_results.json"),
    key=lambda p: p.stat().st_mtime,
    reverse=True,
)
if not candidates:
    raise FileNotFoundError(
        "No fir_freq_results.json found. "
        "Run:  cd core/hw/sim && python run_fir_freq.py --ntaps 32 --corner 5e6 --window hann"
    )
json_path = candidates[0]
print(f"Loading {json_path}")

with open(json_path) as f:
    r = json.load(f)

freqs_ideal = np.array(r["freqs_ideal"])
H_ideal     = np.array(r["H_ideal"])
freqs_rtl   = np.array(r["freqs_rtl"])
H_rtl       = np.array(r["H_rtl"])
fs          = float(r["fs"])
ntaps       = int(r["ntaps"])
corner_hz   = float(r.get("corner_hz", 0.0))
window      = r.get("window", "unknown")

def _db(v):
    return 20.0 * math.log10(max(float(v), 1e-6))

H_ideal_db = np.array([_db(h) for h in H_ideal])
H_rtl_db   = np.array([_db(h) for h in H_rtl])

# ── Plot ──────────────────────────────────────────────────────────────────────
fig, ax = plt.subplots(figsize=(8, 5))

# Ideal curve (dense, no markers)
ax.semilogx(freqs_ideal[1:] / 1e6, H_ideal_db[1:],
            color="#1f77b4", linewidth=2.0,
            label="Ideal  (DTFT of float coefficients)")

# RTL curve (sparse, with markers — each point is a real sim measurement)
ax.semilogx(freqs_rtl / 1e6, H_rtl_db,
            color="#d62728", linewidth=1.5, linestyle="--",
            marker="x", markersize=6, markeredgewidth=1.5,
            label="RTL simulation  (Verilator, measured)")

# Reference lines
ax.axhline(-6,  color="gray", linewidth=0.8, linestyle=":",  alpha=0.7,
           label=r"$-6$ dB")
ax.axhline(-40, color="gray", linewidth=0.8, linestyle="--", alpha=0.5,
           label=r"$-40$ dB")
if corner_hz > 0:
    ax.axvline(corner_hz / 1e6, color="#2ca02c", linewidth=1.0,
               linestyle=":", alpha=0.8,
               label=f"$f_c$ = {corner_hz/1e6:.0f} MHz")

ax.set_xlabel("Frequency  (MHz)", fontsize=11)
ax.set_ylabel("|H(f)|  (dB)", fontsize=11)
win_label = window.capitalize() if window != "unknown" else ""
ax.set_title(
    f"FIR frequency response: ideal vs RTL  "
    f"({win_label} window, $N={ntaps}$, $f_c={corner_hz/1e6:.0f}$ MHz)",
    fontsize=11,
)
ax.legend(fontsize=9)
ax.grid(True, which="both", alpha=0.35)

# x-axis: Hz on a log scale, but label in MHz
ax.set_xlim(freqs_ideal[1] / 1e6, fs / 2e6)

# y-axis: clip to sensible range
finite = [v for v in list(H_ideal_db[1:]) + list(H_rtl_db) if math.isfinite(v)]
y_lo = max(min(finite) - 5, -110)
y_hi = max(finite) + 3
ax.set_ylim(y_lo, y_hi)

out = os.path.join(os.path.dirname(__file__), "fir_ideal_vs_rtl.pdf")
fig.savefig(out, bbox_inches="tight")
print(f"Saved {out}")
