#!/usr/bin/env python3
"""Plot the frequency response of the FIR RTL for a given coefficient CSV.

Runs the cocotb/Verilator sim, collects measured data, then plots ideal vs
RTL response and calls plt.show() in this (normal display-capable) process.

Artifacts are written to sim/artifacts/fir_freq/fir_freq_N/ with
auto-incrementing N.

Usage:
    python run_fir_freq.py                          # use default example CSV
    python run_fir_freq.py --csv path/to/coeffs.csv
    python run_fir_freq.py --sim icarus
    python run_fir_freq.py --no-sim                 # replot existing JSON
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

HERE      = pathlib.Path(__file__).resolve().parent
PROJ_ROOT = HERE.parents[4]

_bin = pathlib.Path(sys.executable).parent
_result = subprocess.run([str(_bin / "cocotb-config"), "--makefiles"],
                         capture_output=True, text=True)
if _result.returncode != 0:
    sys.exit(f"cocotb-config failed: {_result.stderr.strip()}")
COCOTB_MAKEFILES = _result.stdout.strip()


def _db(v):
    return 20.0 * math.log10(max(v, 1e-6))


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


def main() -> None:
    parser = argparse.ArgumentParser(
        description="FIR RTL frequency-response measurement and plot."
    )
    parser.add_argument("--sim", default=os.environ.get("SIM", "verilator"),
                        choices=["icarus", "verilator"])
    parser.add_argument("--csv",
                        default=str(PROJ_ROOT / "test_resources" / "fir_coeffs_example.csv"),
                        metavar="PATH",
                        help="Coefficient CSV (one Q15 float per line)")
    parser.add_argument("--no-sim", action="store_true",
                        help="Skip simulation; replot most recent existing JSON")
    args = parser.parse_args()

    artifact_dir = _make_artifact_dir("fir_freq")
    results_file = artifact_dir / "fir_freq_results.json"

    if not args.no_sim:
        env = os.environ.copy()
        env["SIM"]          = args.sim
        env["COEFF_CSV"]    = str(pathlib.Path(args.csv).resolve())
        env["RESULTS_JSON"] = str(results_file)

        ret = subprocess.run(
            [
                "make", "-f", "Makefile.fir_freq",
                f"SIM={args.sim}",
                f"COCOTB_MAKEFILES={COCOTB_MAKEFILES}",
                f"PYTHON_BIN={sys.executable}",
                f"COEFF_CSV={env['COEFF_CSV']}",
                f"RESULTS_JSON={env['RESULTS_JSON']}",
            ],
            cwd=HERE,
            env=env,
        )
        if ret.returncode != 0:
            sys.exit(ret.returncode)
    else:
        fir_base = HERE / "artifacts" / "fir_freq"
        candidates = sorted(fir_base.glob("fir_freq_*/fir_freq_results.json"),
                            key=lambda p: p.stat().st_mtime, reverse=True)
        if not candidates:
            sys.exit("No existing fir_freq_results.json found in artifacts/fir_freq/")
        results_file = candidates[0]
        artifact_dir = results_file.parent
        print(f"Reusing results from {results_file}")

    with open(results_file) as f:
        r = json.load(f)

    # ── Write RTL response CSV ───────────────────────────────────────────────────
    rtl_csv = artifact_dir / "fir_rtl_response.csv"
    with open(rtl_csv, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["freq_hz", "gain_mag", "gain_db"])
        for freq, mag in zip(r["freqs_rtl"], r["H_rtl"]):
            w.writerow([freq, mag, _db(mag)])

    # ── Plot ─────────────────────────────────────────────────────────────────────
    fig, ax = plt.subplots(figsize=(11, 5))
    ax.semilogx(r["freqs_ideal"][1:], [_db(h) for h in r["H_ideal"][1:]],
                "b-", linewidth=1.5, label="Ideal (from coefficients)")
    ax.semilogx(r["freqs_rtl"], [_db(h) for h in r["H_rtl"]],
                "r--", linewidth=1.5, marker="x", markersize=5,
                label="RTL simulation (measured)")
    ax.axhline(-3,  color="gray", linestyle=":",  linewidth=0.8, label="-3 dB")
    ax.axhline(-40, color="gray", linestyle="--", linewidth=0.8, label="-40 dB")
    ax.set_xlabel("Frequency (Hz)")
    ax.set_ylabel("Magnitude (dB)")
    ax.set_title(
        f"FIR Frequency Response — {r['ntaps']} taps, "
        f"fs = {r['fs']/1e6:.0f} MHz\n"
        f"Coefficients: {pathlib.Path(r['csv_path']).name}"
    )
    ax.legend()
    ax.grid(True, which="both", alpha=0.4)
    ax.set_xlim(r["freqs_ideal"][1], r["f_max"] * 1.05)

    # Data-driven ylim (include -3 dB and -40 dB reference lines)
    all_db = [_db(h) for h in r["H_ideal"][1:]] + [_db(h) for h in r["H_rtl"]]
    finite_db = [v for v in all_db if math.isfinite(v)]
    if finite_db:
        lo = min(finite_db + [-40]) - 5
        hi = max(finite_db + [-3, 0]) + 3
        ax.set_ylim(lo, hi)

    plt.tight_layout()
    fig.savefig(str(artifact_dir / "fir_freq_response.png"),
                dpi=150, bbox_inches="tight")

    print(f"\nArtifacts written to: {artifact_dir}")
    print(f"  fir_freq_results.json")
    print(f"  fir_rtl_response.csv")
    print(f"  fir_freq_response.png")

    plt.show()


if __name__ == "__main__":
    main()
