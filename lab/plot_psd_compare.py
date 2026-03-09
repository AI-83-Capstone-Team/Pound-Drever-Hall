import numpy as np
import matplotlib.pyplot as plt

PRE  = "csv/pre_pdh.csv"
POST = "csv/post_pdh.csv"
FMAX = 1e6  # Hz

pre  = np.loadtxt(PRE,  delimiter=",", skiprows=1)
post = np.loadtxt(POST, delimiter=",", skiprows=1)

mask_pre  = pre[:, 0]  <= FMAX
mask_post = post[:, 0] <= FMAX

# Normalize by total mass over full frequency sweep
norm_pre  = np.trapz(pre[:, 1],  pre[:, 0])
norm_post = np.trapz(post[:, 1], post[:, 0])

f   = pre[mask_pre, 0] / 1e3  # kHz, shared axis
ratio_db = 10 * np.log10(post[mask_post, 1] / np.where(pre[mask_pre, 1] > 0, pre[mask_pre, 1], np.nan))

fig, axes = plt.subplots(1, 3, figsize=(18, 5))

# --- Raw PSD ---
ax = axes[0]
ax.semilogy(f, pre[mask_pre,   1], label="Pre-lock")
ax.semilogy(f, post[mask_post, 1], label="Post-lock")
ax.set_xlabel("Frequency (kHz)")
ax.set_ylabel("PSD (cts²/Hz)")
ax.set_title("ADC_B PSD: Pre vs Post")
ax.legend()
ax.grid(True, alpha=0.3, which="both")

# --- Normalised PSD ---
ax = axes[1]
ax.semilogy(f, pre[mask_pre,   1] / norm_pre,  label="Pre-lock")
ax.semilogy(f, post[mask_post, 1] / norm_post, label="Post-lock")
ax.set_xlabel("Frequency (kHz)")
ax.set_ylabel("Normalised PSD (1/Hz)")
ax.set_title("Normalised to Full-Sweep Mass")
ax.legend()
ax.grid(True, alpha=0.3, which="both")

# --- Post/Pre ratio in dB ---
ax = axes[2]
ax.plot(f, ratio_db, color="C2")
ax.axhline(0, color="k", linewidth=0.8, linestyle="--")
ax.fill_between(f, ratio_db, 0, where=(ratio_db < 0), alpha=0.25, color="C0", label="Suppression")
ax.fill_between(f, ratio_db, 0, where=(ratio_db > 0), alpha=0.25, color="C3", label="Amplification")
ax.set_xlabel("Frequency (kHz)")
ax.set_ylabel("Post/Pre (dB)")
ax.set_title("Lock Gain: Post/Pre Ratio")
ax.legend()
ax.grid(True, alpha=0.3)

fig.tight_layout()
plt.show()
