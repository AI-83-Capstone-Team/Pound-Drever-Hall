"""
Generate figures/line_delay_phasor.pdf
Complex-plane diagram showing that multiplying by e^{-j*theta}
is a pure clockwise rotation — magnitude unchanged.
"""

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import os

BBOX = dict(boxstyle="round,pad=0.18", fc="white", ec="none", alpha=0.90)

fig, ax = plt.subplots(figsize=(5.5, 5.5))

# ── Unit circle ───────────────────────────────────────────────────────────────
theta_circ = np.linspace(0, 2 * np.pi, 300)
ax.plot(np.cos(theta_circ), np.sin(theta_circ), color="#dddddd",
        linewidth=1.0, zorder=1)

# ── Phasor parameters ─────────────────────────────────────────────────────────
phi0   = np.radians(125)   # initial phase of H_RTL (upper-left quadrant)
r      = 0.70              # magnitude
dtheta = np.radians(60)    # line-delay rotation angle

H_rtl = r * np.exp(1j * phi0)
H_cor = r * np.exp(1j * (phi0 - dtheta))

arrow_kw = dict(length_includes_head=True, head_width=0.033, head_length=0.028,
                zorder=3)

# ── H_RTL arrow ───────────────────────────────────────────────────────────────
ax.arrow(0, 0, H_rtl.real, H_rtl.imag,
         fc="#1f77b4", ec="#1f77b4", **arrow_kw)

# Label: anchor above-left of arrowhead, well outside the arc area
ax.annotate(
    r"$H_\mathrm{RTL}(j\omega)$",
    xy=(H_rtl.real, H_rtl.imag),
    xytext=(H_rtl.real - 0.25, H_rtl.imag + 0.18),
    fontsize=10, color="#1f77b4",
    arrowprops=dict(arrowstyle="-", color="#1f77b4", lw=0.7),
    bbox=BBOX,
)

# ── H_corrected arrow ─────────────────────────────────────────────────────────
ax.arrow(0, 0, H_cor.real, H_cor.imag,
         fc="#d62728", ec="#d62728", **arrow_kw)

# Label: anchor below-right, outside
ax.annotate(
    r"$H(j\omega) = H_\mathrm{RTL} \cdot e^{-j\,2\pi f\tau}$",
    xy=(H_cor.real, H_cor.imag),
    xytext=(H_cor.real + 0.18, H_cor.imag - 0.22),
    fontsize=10, color="#d62728",
    arrowprops=dict(arrowstyle="-", color="#d62728", lw=0.7),
    bbox=BBOX,
)

# ── Rotation arc ──────────────────────────────────────────────────────────────
arc_r = r * 0.52
arc_angles = np.linspace(phi0 - dtheta, phi0, 80)
ax.plot(arc_r * np.cos(arc_angles), arc_r * np.sin(arc_angles),
        color="#ff7f0e", linewidth=2.2, zorder=2)

# Arrowhead at the start of arc (H_corrected end)
ax.annotate("",
    xy =(arc_r * np.cos(arc_angles[0]),  arc_r * np.sin(arc_angles[0])),
    xytext=(arc_r * np.cos(arc_angles[2]), arc_r * np.sin(arc_angles[2])),
    arrowprops=dict(arrowstyle="-|>", color="#ff7f0e", lw=1.8))

# Arc label: outside the arc, at the midpoint angle
mid_angle = (phi0 + phi0 - dtheta) / 2
label_r   = arc_r + 0.22
ax.text(label_r * np.cos(mid_angle), label_r * np.sin(mid_angle),
        r"$-2\pi f\tau$",
        ha="center", va="center", color="#ff7f0e", fontsize=11,
        bbox=BBOX)

# ── |H| label on the bisector, beyond both arrowheads ────────────────────────
bisect = (phi0 + phi0 - dtheta) / 2
label_pos = (r + 0.17) * np.exp(1j * bisect)
ax.text(label_pos.real, label_pos.imag,
        r"$|H|$ unchanged",
        ha="center", va="center", color="gray", fontsize=9,
        bbox=BBOX)

# ── Axes ─────────────────────────────────────────────────────────────────────
ax.axhline(0, color="#aaaaaa", linewidth=0.7)
ax.axvline(0, color="#aaaaaa", linewidth=0.7)
ax.text( 1.10,  0.04, "Re", fontsize=10, color="#888888", ha="center")
ax.text( 0.04,  1.10, "Im", fontsize=10, color="#888888", ha="center")
# Origin dot
ax.plot(0, 0, "o", color="#888888", markersize=4, zorder=4)

ax.set_xlim(-1.30, 1.30)
ax.set_ylim(-1.30, 1.30)
ax.set_aspect("equal")
ax.axis("off")
ax.set_title("Line-delay correction as a complex rotation", fontsize=11, pad=8)

out = os.path.join(os.path.dirname(__file__), "line_delay_phasor.pdf")
fig.savefig(out, bbox_inches="tight")
print(f"Saved {out}")
