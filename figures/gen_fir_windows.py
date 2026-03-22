"""
Generate figures/fir_windows.pdf
Three-panel figure:
  A) Window shapes w[n] for all 4 window types
  B) Windowed sinc coefficients h[n]*w[n]
  C) Magnitude response |H(f)| in dB
"""

import math
import cmath
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import os

FS    = 125e6
NTAPS = 32
FC    = 1e6   # 1 MHz corner

WINDOWS = ["rectangular", "hann", "hamming", "blackman"]
COLORS  = ["#444444", "#1f77b4", "#ff7f0e", "#2ca02c"]
LABELS  = ["Rectangular", "Hann", "Hamming", "Blackman"]

def window(name, n, N):
    if name == "hann":
        return 0.5 - 0.5 * math.cos(2 * math.pi * n / (N - 1))
    elif name == "hamming":
        return 0.54 - 0.46 * math.cos(2 * math.pi * n / (N - 1))
    elif name == "blackman":
        return (0.42
                - 0.50 * math.cos(2 * math.pi * n / (N - 1))
                + 0.08 * math.cos(4 * math.pi * n / (N - 1)))
    else:
        return 1.0

def sinc_tap(n, nc, wc):
    x = n - nc
    if abs(x) < 1e-10:
        return wc / math.pi
    return math.sin(wc * x) / (math.pi * x)

def design(name, ntaps=NTAPS, fc=FC, fs=FS):
    wc = 2 * math.pi * fc / fs
    nc = (ntaps - 1) / 2.0
    coeffs = []
    for n in range(ntaps):
        h = sinc_tap(n, nc, wc)
        w = window(name, n, ntaps)
        coeffs.append(h * w)
    return coeffs

def freq_response(coeffs, n_pts=4096, fs=FS):
    N = len(coeffs)
    freqs = np.linspace(0, fs / 2, n_pts)
    H = []
    for f in freqs:
        omega = 2 * math.pi * f / fs
        val = sum(c * cmath.exp(-1j * omega * n) for n, c in enumerate(coeffs))
        H.append(abs(val))
    return freqs, np.array(H)

# ── Compute data ──────────────────────────────────────────────────────────────
ns = np.arange(NTAPS)
all_wins    = [[window(w, n, NTAPS) for n in range(NTAPS)] for w in WINDOWS]
all_coeffs  = [design(w)                                   for w in WINDOWS]
all_freqs   = []
all_H_db    = []
for c in all_coeffs:
    f, H = freq_response(c)
    all_freqs.append(f / 1e6)          # MHz
    all_H_db.append(20 * np.log10(np.maximum(H, 1e-9)))

# ── Plot ──────────────────────────────────────────────────────────────────────
fig, axes = plt.subplots(3, 1, figsize=(7, 9))
fig.subplots_adjust(hspace=0.45)

# Panel A: window shapes
ax = axes[0]
for w_vals, lab, col in zip(all_wins, LABELS, COLORS):
    ax.plot(ns, w_vals, color=col, label=lab, linewidth=1.6)
ax.set_xlabel("Tap index $n$")
ax.set_ylabel("$w[n]$")
ax.set_title("(A)  Window functions", loc="left", fontsize=10, fontweight="bold")
ax.legend(fontsize=8, ncol=2)
ax.set_xlim(0, NTAPS - 1)
ax.set_ylim(-0.05, 1.1)
ax.grid(True, alpha=0.3)

# Panel B: windowed sinc coefficients
ax = axes[1]
for coeffs, lab, col in zip(all_coeffs, LABELS, COLORS):
    ax.plot(ns, coeffs, color=col, label=lab, linewidth=1.6, marker="o",
            markersize=3)
ax.axhline(0, color="gray", linewidth=0.6, linestyle="--")
ax.set_xlabel("Tap index $n$")
ax.set_ylabel("$h[n]$")
ax.set_title("(B)  Windowed sinc coefficients  ($f_c = 1$ MHz,  $N = 32$)",
             loc="left", fontsize=10, fontweight="bold")
ax.legend(fontsize=8, ncol=2)
ax.set_xlim(0, NTAPS - 1)
ax.grid(True, alpha=0.3)

# Panel C: frequency response
ax = axes[2]
for f_mhz, H_db, lab, col in zip(all_freqs, all_H_db, LABELS, COLORS):
    ax.plot(f_mhz, H_db, color=col, label=lab, linewidth=1.6)
ax.axhline(-6, color="gray", linewidth=0.8, linestyle="--", label="$-6$ dB corner")
ax.set_xlabel("Frequency (MHz)")
ax.set_ylabel("|H(f)|  (dB)")
ax.set_title("(C)  Magnitude response", loc="left", fontsize=10, fontweight="bold")
ax.legend(fontsize=8, ncol=2)
ax.set_xlim(0, FS / 2e6)
ax.set_ylim(-100, 5)
ax.grid(True, alpha=0.3, which="both")

# Annotate stopband floors
stopband_dB = {"Rectangular": -21, "Hann": -44, "Hamming": -53, "Blackman": -74}
for lab, col in zip(LABELS, COLORS):
    axes[2].axhline(stopband_dB[lab], color=col, linewidth=0.5, linestyle=":",
                    alpha=0.6)

out = os.path.join(os.path.dirname(__file__), "fir_windows.pdf")
fig.savefig(out, bbox_inches="tight")
print(f"Saved {out}")
