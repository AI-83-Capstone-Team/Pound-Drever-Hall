#!/usr/bin/env python3
"""
Run the PDH controller Bode analysis simulation and plot the results.

Artifacts are written to sim/artifacts/bode/bode_N/ with auto-incrementing N.

Usage:
    python run_bode.py                                  # defaults
    python run_bode.py --kp 0.8 --ki 0.1 --kd 0.0
    python run_bode.py --alpha 3 --satwidth 28
    python run_bode.py --delay 6.2                      # 6.2 ns line delay
    python run_bode.py --dec 10,100,1000 --plot-dec 100
    python run_bode.py --sim icarus --csv path/to/coeffs.csv
    python run_bode.py --no-sim                         # replot existing JSON
"""

import argparse
import csv
import json
import math
import os
import pathlib
import subprocess
import sys

import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

HERE      = pathlib.Path(__file__).resolve().parent
PROJ_ROOT = HERE.parents[4]

_bin = pathlib.Path(sys.executable).parent
_result = subprocess.run([str(_bin / "cocotb-config"), "--makefiles"],
                         capture_output=True, text=True)
if _result.returncode != 0:
    sys.exit(f"cocotb-config failed: {_result.stderr.strip()}")
COCOTB_MAKEFILES = _result.stdout.strip()

CONFIG_LABELS = {
    "direct_adc":   "Direct ADC (SAT_A_16S)",
    "rotation":     "Rotation 0° (I_FEED)",
    "fir_adc":      "FIR + ADC (FIR_OUT, input=ADC1)",
    "fir_rotation": "FIR + Rotation (FIR_OUT, input=I_FEED)",
}
CONFIG_COLORS = {
    "direct_adc":   "#1f77b4",
    "rotation":     "#ff7f0e",
    "fir_adc":      "#2ca02c",
    "fir_rotation": "#d62728",
}


def _make_artifact_dir(sim_name: str) -> pathlib.Path:
    base = HERE / "artifacts" / sim_name
    base.mkdir(parents=True, exist_ok=True)
    n = 1
    while True:
        d = base / f"{sim_name}_{n}"
        if not d.exists():
            d.mkdir()
            return d
        n += 1


def _autolim(values, pad_frac=0.08, ref_vals=()):
    """Return (lo, hi) axis limits that cover values and ref_vals with padding."""
    all_v = [v for v in list(values) + list(ref_vals) if math.isfinite(v)]
    if not all_v:
        return None, None
    lo, hi = min(all_v), max(all_v)
    span = hi - lo or 1.0
    return lo - pad_frac * span, hi + pad_frac * span


def main() -> None:
    parser = argparse.ArgumentParser(
        description="PDH controller open-loop Bode analysis."
    )
    parser.add_argument("--sim",  default=os.environ.get("SIM", "verilator"),
                        choices=["icarus", "verilator"])
    parser.add_argument("--csv",  default=str(PROJ_ROOT / "test_resources" /
                                              "fir_coeffs_example.csv"),
                        metavar="PATH", help="FIR coefficient CSV")
    parser.add_argument("--kp",       default="0.5",    help="Kp (default 0.5)")
    parser.add_argument("--ki",       default="0.0",    help="Ki (default 0.0)")
    parser.add_argument("--kd",       default="0.0",    help="Kd (default 0.0)")
    parser.add_argument("--alpha",    default="2",      help="EMA alpha exponent (default 2)")
    parser.add_argument("--satwidth", default="31",     help="Integrator sat width (default 31)")
    parser.add_argument("--delay",    default="2500.0", metavar="NS",
                        help="Physical DAC→ADC line delay in nanoseconds (default 2500.0)")
    parser.add_argument("--dec",  default="1,2,5,10,20,50,100,200,500,1000",
                        help="Comma-separated decimation codes")
    parser.add_argument("--plot-dec", default=None, type=int, dest="plot_dec",
                        help="Decimation code shown in Bode diagram panels "
                             "(default: median of --dec list)")
    parser.add_argument("--no-sim", action="store_true",
                        help="Skip simulation; load existing bode_results.json")
    args = parser.parse_args()

    artifact_dir = _make_artifact_dir("bode")
    results_file = artifact_dir / "bode_results.json"

    if not args.no_sim:
        env = os.environ.copy()
        env["SIM"]           = args.sim
        env["COEFF_CSV"]     = str(pathlib.Path(args.csv).resolve())
        env["RESULTS_JSON"]  = str(results_file)
        env["KP_TEST"]       = args.kp
        env["KI_TEST"]       = args.ki
        env["KD_TEST"]       = args.kd
        env["ALPHA_TEST"]    = args.alpha
        env["SATWIDTH_TEST"] = args.satwidth
        env["LINE_DELAY_NS"] = args.delay
        env["DEC_CODES"]     = args.dec

        ret = subprocess.run(
            [
                "make", "-f", "Makefile.bode",
                f"SIM={args.sim}",
                f"COCOTB_MAKEFILES={COCOTB_MAKEFILES}",
                f"PYTHON_BIN={sys.executable}",
                f"COEFF_CSV={env['COEFF_CSV']}",
                f"RESULTS_JSON={env['RESULTS_JSON']}",
                f"KP_TEST={env['KP_TEST']}",
                f"KI_TEST={env['KI_TEST']}",
                f"KD_TEST={env['KD_TEST']}",
                f"ALPHA_TEST={env['ALPHA_TEST']}",
                f"SATWIDTH_TEST={env['SATWIDTH_TEST']}",
                f"LINE_DELAY_NS={env['LINE_DELAY_NS']}",
                f"DEC_CODES={env['DEC_CODES']}",
            ],
            cwd=HERE,
            env=env,
        )
        if ret.returncode != 0:
            sys.exit(ret.returncode)
    else:
        # --no-sim: find the most recent bode run and use its JSON
        bode_base = HERE / "artifacts" / "bode"
        candidates = sorted(bode_base.glob("bode_*/bode_results.json"),
                            key=lambda p: p.stat().st_mtime, reverse=True)
        if not candidates:
            sys.exit("No existing bode_results.json found in artifacts/bode/")
        results_file = candidates[0]
        artifact_dir = results_file.parent
        print(f"Reusing results from {results_file}")

    with open(results_file) as f:
        r = json.load(f)

    fs       = r["fs"]
    kp       = r["kp"]
    ki       = r.get("ki", 0.0)
    kd       = r.get("kd", 0.0)
    alpha    = r.get("alpha", 2)
    satwidth = r.get("satwidth", 31)
    delay_ns = r.get("line_delay_ns", 0.0)
    ntaps    = r["ntaps"]
    configs  = r["configs"]

    dec_codes_all = sorted(
        {int(k) for cfg in configs.values() for k in cfg["data"]},
        key=int
    )

    if args.plot_dec is not None:
        plot_dec = args.plot_dec
    else:
        plot_dec = dec_codes_all[len(dec_codes_all) // 2]

    pid_str = f"Kp={kp}"
    if ki != 0.0: pid_str += f"  Ki={ki}"
    if kd != 0.0: pid_str += f"  Kd={kd}"
    if alpha != 2: pid_str += f"  α={alpha}"
    if satwidth != 31: pid_str += f"  sat={satwidth}"
    pid_str += f"  delay={delay_ns} ns"

    # ── Write stability_margins.csv ──────────────────────────────────────────────
    margins_csv = artifact_dir / "stability_margins.csv"
    with open(margins_csv, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["config", "decimation", "f_control_hz",
                    "f_gc_hz", "pm_deg", "f_pc_hz", "gm_db"])
        for cfg_name, cfg_data in configs.items():
            for dec_str in sorted(cfg_data["data"].keys(), key=int):
                d = cfg_data["data"][dec_str]
                dec = int(dec_str)
                w.writerow([
                    cfg_name, dec, fs / dec,
                    d["f_gc"] if d["f_gc"] is not None else "",
                    d["pm_deg"] if d["pm_deg"] is not None else "",
                    d["f_pc"] if d["f_pc"] is not None else "",
                    d["gm_db"] if d["gm_db"] is not None else "",
                ])

    # ── Figure 1: Bode diagrams (2×2, one panel per config) ─────────────────────
    fig1, axes = plt.subplots(2, 2, figsize=(14, 9))
    fig1.suptitle(
        f"Open-loop Bode diagrams  —  D={plot_dec},  {pid_str}\n"
        f"fs={fs/1e6:.0f} MHz,  {ntaps} taps,  "
        f"coeffs: {pathlib.Path(r['csv_path']).name}",
        fontsize=10,
    )

    for ax, (cfg_name, cfg_data) in zip(axes.flat, configs.items()):
        label = CONFIG_LABELS.get(cfg_name, cfg_name)
        color = CONFIG_COLORS.get(cfg_name, "black")

        dec_str = str(plot_dec)
        if dec_str not in cfg_data["data"]:
            ax.set_title(f"{label}\n(dec={plot_dec} not in results)")
            continue

        d = cfg_data["data"][dec_str]
        freqs      = d["freqs"]
        gain_db    = d["gain_db"]
        phase_deg  = d["phase_deg"]
        phase_unwrapped = list(
            np.unwrap(np.deg2rad(phase_deg)) * 180.0 / math.pi
        )

        ax2 = ax.twinx()
        ax.semilogx(freqs, gain_db, color=color, lw=1.8, label="Gain (dB)")
        ax2.semilogx(freqs, phase_unwrapped, color=color, lw=1.8,
                     linestyle="--", label="Phase (°)")

        ax.axhline(0,    color="gray",   lw=0.7, linestyle=":", label="0 dB")
        ax2.axhline(-180, color="purple", lw=0.8, linestyle=":", label="-180°")

        if d["f_gc"] is not None:
            ax.axvline(d["f_gc"], color="blue", lw=0.9, linestyle=":",
                       label=f"f_gc={d['f_gc']/1e3:.1f} kHz")
        if d["f_pc"] is not None:
            ax.axvline(d["f_pc"], color="red", lw=0.9, linestyle=":",
                       label=f"f_pc={d['f_pc']/1e3:.1f} kHz")

        pm_str = f"{d['pm_deg']:.1f}°"  if d["pm_deg"] is not None else "∞ (no G.C.)"
        gm_str = f"{d['gm_db']:.1f} dB" if d["gm_db"] is not None else "N/A"
        ax.set_title(f"{label}\nPM={pm_str}  GM={gm_str}", fontsize=9)

        ax.set_xlabel("Frequency (Hz)")
        ax.set_ylabel("Gain (dB)", color=color)
        ax2.set_ylabel("Phase (°)", color=color)

        # Data-driven axis limits
        g_lo, g_hi = _autolim(gain_db, ref_vals=[0])
        p_lo, p_hi = _autolim(phase_unwrapped, ref_vals=[-180])
        if g_lo is not None: ax.set_ylim(g_lo, g_hi)
        if p_lo is not None: ax2.set_ylim(p_lo, p_hi)

        ax.grid(True, which="both", alpha=0.3)

        lines1, labs1 = ax.get_legend_handles_labels()
        lines2, labs2 = ax2.get_legend_handles_labels()
        ax.legend(lines1 + lines2, labs1 + labs2, fontsize=7, loc="lower left")

    plt.tight_layout()
    fig1.savefig(str(artifact_dir / f"bode_diagrams_D{plot_dec}.png"),
                 dpi=150, bbox_inches="tight")

    # ── Figure 2: stability margins vs control frequency ────────────────────────
    fig2, (ax_pm, ax_gm) = plt.subplots(2, 1, figsize=(11, 8), sharex=True)
    fig2.suptitle(
        f"Stability margins vs control frequency  —  {pid_str}\n"
        f"fs={fs/1e6:.0f} MHz,  {ntaps} taps,  "
        f"coeffs: {pathlib.Path(r['csv_path']).name}",
        fontsize=10,
    )

    all_pm, all_gm = [], []

    for cfg_name, cfg_data in configs.items():
        label = CONFIG_LABELS.get(cfg_name, cfg_name)
        color = CONFIG_COLORS.get(cfg_name, "black")

        dec_list = sorted(cfg_data["data"].keys(), key=int)
        f_ctrl   = [fs / int(d) for d in dec_list]
        pm_vals  = [cfg_data["data"][d]["pm_deg"] for d in dec_list]
        gm_vals  = [cfg_data["data"][d]["gm_db"]  for d in dec_list]

        pm_plot = [v if v is not None else float("nan") for v in pm_vals]
        gm_plot = [v if v is not None else float("nan") for v in gm_vals]
        all_pm.extend(v for v in pm_vals if v is not None)
        all_gm.extend(v for v in gm_vals if v is not None)

        ax_pm.semilogx(f_ctrl, pm_plot, color=color, marker="o",
                       markersize=4, lw=1.5, label=label)
        ax_gm.semilogx(f_ctrl, gm_plot, color=color, marker="o",
                       markersize=4, lw=1.5, label=label)

    ax_pm.axhline(0,  color="red",   lw=1.0, linestyle="--", label="0° (unstable)")
    ax_pm.axhline(45, color="green", lw=0.8, linestyle=":",  label="45° (target)")
    ax_gm.axhline(0,  color="red",   lw=1.0, linestyle="--", label="0 dB (unstable)")
    ax_gm.axhline(6,  color="green", lw=0.8, linestyle=":",  label="6 dB (target)")

    # Data-driven axis limits (always include reference lines)
    pm_lo, pm_hi = _autolim(all_pm, ref_vals=[0, 45])
    gm_lo, gm_hi = _autolim(all_gm, ref_vals=[0, 6])
    if pm_lo is not None:
        ax_pm.set_ylim(pm_lo, pm_hi)
    else:
        ax_pm.set_ylim(-10, 50)
        ax_pm.text(0.5, 0.5, "No gain crossover found\n(|H| < 1 for all frequencies —\ntry increasing Kp or adding Ki)",
                   transform=ax_pm.transAxes, ha="center", va="center",
                   fontsize=9, color="gray",
                   bbox=dict(boxstyle="round", fc="white", alpha=0.7))
    if gm_lo is not None:
        ax_gm.set_ylim(gm_lo, gm_hi)
    else:
        ax_gm.set_ylim(-5, 20)

    ax_pm.set_ylabel("Phase margin (°)")
    ax_gm.set_ylabel("Gain margin (dB)")
    ax_gm.set_xlabel("Control frequency = fs / D  (Hz)")

    def _f_to_dec(f): return fs / f
    def _dec_to_f(d): return fs / d

    secx = ax_gm.secondary_xaxis("top", functions=(_f_to_dec, _dec_to_f))
    secx.set_xlabel("Decimation code D")
    secx.xaxis.set_major_formatter(
        ticker.FuncFormatter(lambda x, _: f"{int(round(x))}")
    )

    for ax in (ax_pm, ax_gm):
        ax.grid(True, which="both", alpha=0.3)
        ax.legend(fontsize=8, loc="upper right")

    plt.tight_layout()
    fig2.savefig(str(artifact_dir / "stability_margins.png"),
                 dpi=150, bbox_inches="tight")

    print(f"\nArtifacts written to: {artifact_dir}")
    print(f"  bode_results.json")
    print(f"  stability_margins.csv")
    print(f"  bode_diagrams_D{plot_dec}.png")
    print(f"  stability_margins.png")

    plt.show()


if __name__ == "__main__":
    main()
