"""
Generate figures/fir_ideal_vs_q15.pdf

Compare the frequency response of the ideal float-coefficient FIR filter
against the Q15-quantized version that the FPGA actually runs.

Both responses are computed analytically from their coefficient arrays using
the DTFT formula H(f) = sum_n h[n] * exp(-j*omega*n).

The difference in the stopband shows the quantisation noise floor imposed
by 16-bit fixed-point representation.
"""

import cmath
import math
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import os

FS    = 125e6
NTAPS = 32
FC    = 5e6       # 5 MHz — sits comfortably in the middle of the spectrum
WIN   = "hann"


# ── Coefficient design (mirrors fir_design.py exactly) ───────────────────────

def _window(name, n, N):
    if name == "hann":
        return 0.5 - 0.5 * math.cos(2 * math.pi * n / (N - 1))
    elif name == "hamming":
        return 0.54 - 0.46 * math.cos(2 * math.pi * n / (N - 1))
    elif name == "blackman":
        return (0.42
                - 0.50 * math.cos(2 * math.pi * n / (N - 1))
                + 0.08 * math.cos(4 * math.pi * n / (N - 1)))
    return 1.0


def design_float(ntaps, fc, fs, window):
    wc = 2 * math.pi * fc / fs
    nc = (ntaps - 1) / 2.0
    coeffs = []
    for n in range(ntaps):
        x = n - nc
        h = wc / math.pi if abs(x) < 1e-10 else math.sin(wc * x) / (math.pi * x)
        coeffs.append(h * _window(window, n, ntaps))
    return coeffs


def quantise_q15(coeffs):
    """Round-trip through Q15: float → int16 → float."""
    return [round(c * 32768.0) / 32768.0 for c in coeffs]


def dtft(coeffs, freqs, fs):
    """Evaluate |H(f)| for each f in freqs."""
    H = []
    for f in freqs:
        omega = 2 * math.pi * f / fs
        val = sum(c * cmath.exp(-1j * omega * n) for n, c in enumerate(coeffs))
        H.append(abs(val))
    return np.array(H)


# ── Compute ───────────────────────────────────────────────────────────────────

h_float = design_float(NTAPS, FC, FS, WIN)
h_q15   = quantise_q15(h_float)

n_pts = 3000
freqs = np.linspace(0, FS / 2, n_pts)

H_float = dtft(h_float, freqs, FS)
H_q15   = dtft(h_q15,   freqs, FS)

H_float_db = 20 * np.log10(np.maximum(H_float, 1e-8))
H_q15_db   = 20 * np.log10(np.maximum(H_q15,   1e-8))
diff_db     = H_float_db - H_q15_db   # positive where float is deeper

f_mhz = freqs / 1e6

print(f"Float DC gain  : {sum(h_float):.8f}")
print(f"Q15   DC gain  : {sum(h_q15):.8f}")
print(f"Max float coeff: {max(abs(c) for c in h_float):.8f}")
print(f"Max Q15  coeff : {max(abs(c) for c in h_q15):.8f}")
print(f"Max |diff| dB  : {np.max(np.abs(diff_db)):.3f} dB")

# Typical stopband level (median beyond 2*fc, ignoring deep nulls)
stop_mask  = freqs > 2 * FC
floor_flt  = float(np.median(H_float_db[stop_mask]))
floor_q15  = float(np.median(H_q15_db[stop_mask]))
print(f"Median stopband (float): {floor_flt:.1f} dB")
print(f"Median stopband (Q15)  : {floor_q15:.1f} dB")

# ── Plot ──────────────────────────────────────────────────────────────────────

fig, (ax_main, ax_diff) = plt.subplots(2, 1, figsize=(7.5, 7),
                                        gridspec_kw={"height_ratios": [3, 1.5]})
fig.subplots_adjust(hspace=0.35)

# ── Panel A: magnitude response ───────────────────────────────────────────────
ax_main.plot(f_mhz, H_float_db, color="#1f77b4", linewidth=2.0,
             label="Ideal (float coefficients)")
ax_main.plot(f_mhz, H_q15_db,   color="#d62728", linewidth=1.5,
             linestyle="--", label="Q15 quantised (FPGA)")

ax_main.axhline(-6, color="gray", linewidth=0.8, linestyle=":", alpha=0.7,
                label=r"$-6$ dB corner")
ax_main.axvline(FC / 1e6, color="gray", linewidth=0.8, linestyle=":", alpha=0.7)

# Annotate stopband floors
ax_main.axhline(floor_flt, xmin=2*FC/FS*2, color="#1f77b4",
                linewidth=0.7, linestyle="--", alpha=0.5)
ax_main.axhline(floor_q15, xmin=2*FC/FS*2, color="#d62728",
                linewidth=0.7, linestyle="--", alpha=0.5)

# Shaded stopband
ax_main.axvspan(2 * FC / 1e6, FS / 2e6, color="#ffe0e0", alpha=0.25, zorder=0)
ax_main.text(FS / 2e6 * 0.72, floor_q15 + 5,
             f"Q15 median stopband\n{floor_q15:.0f} dB", color="#d62728",
             fontsize=8, ha="center",
             bbox=dict(fc="white", ec="none", alpha=0.8))
ax_main.text(FS / 2e6 * 0.72, floor_flt - 9,
             f"Float median stopband\n{floor_flt:.0f} dB", color="#1f77b4",
             fontsize=8, ha="center",
             bbox=dict(fc="white", ec="none", alpha=0.8))

ax_main.set_ylabel("|H(f)|  (dB)", fontsize=10)
ax_main.set_title(
    f"(A)  Float vs Q15 frequency response  "
    f"({WIN.capitalize()} window, $N={NTAPS}$, $f_c={FC/1e6:.0f}$ MHz)",
    loc="left", fontsize=10, fontweight="bold")
ax_main.legend(fontsize=9)
ax_main.set_xlim(0, FS / 2e6)
ax_main.set_ylim(-165, 5)
ax_main.grid(True, alpha=0.3, which="both")
ax_main.set_xticklabels([])   # shared with lower panel

# ── Panel B: difference ────────────────────────────────────────────────────────
ax_diff.plot(f_mhz, diff_db, color="#ff7f0e", linewidth=1.5)
ax_diff.axhline(0, color="gray", linewidth=0.8, linestyle="--")
ax_diff.axvspan(2 * FC / 1e6, FS / 2e6, color="#ffe0e0", alpha=0.25, zorder=0)

ax_diff.set_xlabel("Frequency  (MHz)", fontsize=10)
ax_diff.set_ylabel("Difference  (dB)", fontsize=10)
ax_diff.set_title(
    "(B)  Float − Q15  (positive = float has deeper null)",
    loc="left", fontsize=10, fontweight="bold")
ax_diff.set_xlim(0, FS / 2e6)
ax_diff.grid(True, alpha=0.3)

out = os.path.join(os.path.dirname(__file__), "fir_ideal_vs_q15.pdf")
fig.savefig(out, bbox_inches="tight")
print(f"Saved {out}")
