#!/usr/bin/env python3
"""Plot the frequency response of the FIR RTL for a given coefficient CSV.

Runs the cocotb/Verilator sim, collects measured data, then plots ideal vs
RTL response and calls plt.show() in this (normal display-capable) process.

Usage:
    python run_fir_freq.py                          # use default example CSV
    python run_fir_freq.py --csv path/to/coeffs.csv
    python run_fir_freq.py --sim icarus
"""

import argparse
import json
import math
import os
import pathlib
import subprocess
import sys
import tempfile

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
    args = parser.parse_args()

    results_file = HERE / "fir_freq_results.json"
    results_file.unlink(missing_ok=True)

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

    with open(results_file) as f:
        r = json.load(f)

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
    ax.set_ylim(bottom=-80, top=5)
    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
