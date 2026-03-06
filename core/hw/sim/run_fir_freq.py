#!/usr/bin/env python3
"""Plot the frequency response of the FIR RTL for a designed lowpass filter.

Designs a windowed-sinc lowpass FIR from --ntaps and --corner, writes the
coefficient CSV to the artifact folder, runs the cocotb/Verilator sim, then
plots ideal vs RTL response.

Artifacts written to sim/artifacts/fir_freq/fir_freq_N/ with auto-incrementing N.

Usage:
    python run_fir_freq.py                              # 32 taps, 5 MHz corner, Hann
    python run_fir_freq.py --ntaps 64 --corner 2e6
    python run_fir_freq.py --ntaps 32 --corner 10e6 --window hamming
    python run_fir_freq.py --sim icarus
    python run_fir_freq.py --no-sim                     # replot existing JSON
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

HERE = pathlib.Path(__file__).resolve().parent

FS = 125e6      # hardware sample rate (Hz)

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


def _design_lowpass(ntaps: int, corner_hz: float, window: str) -> list:
    """Windowed-sinc lowpass FIR. Returns list of ntaps float coefficients.

    corner_hz is the half-amplitude (-6 dB) cutoff of the ideal sinc kernel.
    """
    wc = 2.0 * math.pi * corner_hz / FS
    nc = (ntaps - 1) / 2.0
    coeffs = []
    for n in range(ntaps):
        x = n - nc
        h = wc / math.pi if abs(x) < 1e-10 else math.sin(wc * x) / (math.pi * x)
        if window == "hann":
            w = 0.5 * (1.0 - math.cos(2.0 * math.pi * n / (ntaps - 1)))
        elif window == "hamming":
            w = 0.54 - 0.46 * math.cos(2.0 * math.pi * n / (ntaps - 1))
        elif window == "blackman":
            w = (0.42
                 - 0.50 * math.cos(2.0 * math.pi * n / (ntaps - 1))
                 + 0.08 * math.cos(4.0 * math.pi * n / (ntaps - 1)))
        else:   # rectangular
            w = 1.0
        coeffs.append(h * w)
    if max(abs(c) for c in coeffs) >= 1.0:
        sys.exit("Error: coefficient magnitude ≥ 1 — exceeds Q15 range. "
                 "Reduce corner frequency.")
    return coeffs


def main() -> None:
    parser = argparse.ArgumentParser(
        description="FIR RTL frequency-response measurement and plot."
    )
    parser.add_argument("--sim", default=os.environ.get("SIM", "verilator"),
                        choices=["icarus", "verilator"])
    parser.add_argument("--ntaps",  type=int,   default=32,
                        help="Number of FIR taps (default 32)")
    parser.add_argument("--corner", type=float, default=5e6,
                        help="Lowpass corner frequency in Hz (default 5e6)")
    parser.add_argument("--window", default="hann",
                        choices=["hann", "hamming", "blackman", "rectangular"],
                        help="Window function (default hann)")
    parser.add_argument("--no-sim", action="store_true",
                        help="Skip simulation; replot most recent existing JSON")
    args = parser.parse_args()

    artifact_dir = _make_artifact_dir("fir_freq")
    results_file = artifact_dir / "fir_freq_results.json"

    if not args.no_sim:
        # ── Design filter ──────────────────────────────────────────────────────
        coeffs = _design_lowpass(args.ntaps, args.corner, args.window)
        dc_gain = sum(coeffs)
        print(f"Designed {args.ntaps}-tap {args.window} LPF  "
              f"corner={args.corner/1e6:.3f} MHz")
        print(f"  DC gain:     {dc_gain:.6f}")
        print(f"  max|coeff|:  {max(abs(c) for c in coeffs):.6f}")

        # ── Write coefficient CSV to artifact dir ──────────────────────────────
        coeff_csv = artifact_dir / "fir_coeffs.csv"
        with open(coeff_csv, "w") as f:
            for c in coeffs:
                f.write(f"{c:.10f}\n")
        print(f"  Coefficients → {coeff_csv}")

        # AW = number of address bits needed to index ntaps entries
        aw = max(1, (args.ntaps - 1).bit_length())

        env = os.environ.copy()
        env["SIM"]          = args.sim
        env["COEFF_CSV"]    = str(coeff_csv)
        env["RESULTS_JSON"] = str(results_file)
        env["CORNER_HZ"]    = str(args.corner)

        ret = subprocess.run(
            [
                "make", "-f", "Makefile.fir_freq",
                f"SIM={args.sim}",
                f"COCOTB_MAKEFILES={COCOTB_MAKEFILES}",
                f"PYTHON_BIN={sys.executable}",
                f"COEFF_CSV={env['COEFF_CSV']}",
                f"RESULTS_JSON={env['RESULTS_JSON']}",
                f"CORNER_HZ={args.corner}",
                f"NTAPS={args.ntaps}",
                f"AW={aw}",
            ],
            cwd=HERE,
            env=env,
        )
        if ret.returncode != 0:
            sys.exit(ret.returncode)

        # ── Annotate JSON with filter design parameters ────────────────────────
        with open(results_file) as f:
            r = json.load(f)
        r["corner_hz"] = args.corner
        r["window"]    = args.window
        with open(results_file, "w") as f:
            json.dump(r, f)

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

    corner_hz = r.get("corner_hz", 0.0)
    window    = r.get("window", "unknown")

    # ── Write RTL response CSV ─────────────────────────────────────────────────
    rtl_csv = artifact_dir / "fir_rtl_response.csv"
    with open(rtl_csv, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["freq_hz", "gain_mag", "gain_db"])
        for freq, mag in zip(r["freqs_rtl"], r["H_rtl"]):
            w.writerow([freq, mag, _db(mag)])

    # ── Plot ───────────────────────────────────────────────────────────────────
    fig, ax = plt.subplots(figsize=(11, 5))
    ax.semilogx(r["freqs_ideal"][1:], [_db(h) for h in r["H_ideal"][1:]],
                "b-", linewidth=1.5, label="Ideal (from coefficients)")
    ax.semilogx(r["freqs_rtl"], [_db(h) for h in r["H_rtl"]],
                "r--", linewidth=1.5, marker="x", markersize=5,
                label="RTL simulation (measured)")
    ax.axhline(-3,  color="gray", linestyle=":",  linewidth=0.8, label="-3 dB")
    ax.axhline(-40, color="gray", linestyle="--", linewidth=0.8, label="-40 dB")
    if corner_hz > 0:
        ax.axvline(corner_hz, color="purple", linestyle=":", linewidth=0.9,
                   label=f"corner={corner_hz/1e6:.3f} MHz")
    ax.set_xlabel("Frequency (Hz)")
    ax.set_ylabel("Magnitude (dB)")
    corner_str = f"{corner_hz/1e6:.3f} MHz" if corner_hz > 0 else "N/A"
    ax.set_title(
        f"FIR Frequency Response — {r['ntaps']} taps, "
        f"fs = {r['fs']/1e6:.0f} MHz\n"
        f"Corner: {corner_str},  Window: {window}"
    )
    ax.legend()
    ax.grid(True, which="both", alpha=0.4)
    ax.set_xlim(r["freqs_ideal"][1], r["f_max"] * 1.05)

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
    if not args.no_sim:
        print(f"  fir_coeffs.csv")
    print(f"  fir_freq_results.json")
    print(f"  fir_rtl_response.csv")
    print(f"  fir_freq_response.png")

    plt.show()


if __name__ == "__main__":
    main()
