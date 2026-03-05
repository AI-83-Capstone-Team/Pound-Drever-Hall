#!/usr/bin/env python3
"""
Generate FIR low-pass filter tap coefficients for the PDH system FIR module.

The filter is designed using the window method (Hamming window by default).
Coefficients are written one per line as floating-point values in Q15 range
[-1, 1) and can be loaded directly by api_set_fir().

Usage:
    python gen_fir_coeffs.py --ntaps 32 --fc 100e3 --fs 125e6 --out fir_coeffs.csv
    python gen_fir_coeffs.py --ntaps 32 --fc 1e6 --window hann --out fir_1mhz.csv
"""
import argparse
import math


def hamming(n: int, N: int) -> float:
    return 0.54 - 0.46 * math.cos(2 * math.pi * n / (N - 1))


def hann(n: int, N: int) -> float:
    return 0.5 * (1.0 - math.cos(2 * math.pi * n / (N - 1)))


def rectangular(_n: int, _N: int) -> float:
    return 1.0


WINDOWS = {"hamming": hamming, "hann": hann, "rectangular": rectangular}


def design_lowpass_fir(ntaps: int, fc: float, fs: float, window: str = "hamming") -> list[float]:
    """
    Design a linear-phase FIR low-pass filter using the window method.

    ntaps:  number of taps (must match RTL NTAPS parameter, currently 32)
    fc:     cutoff frequency in Hz
    fs:     sample frequency in Hz
    window: 'hamming' | 'hann' | 'rectangular'

    Returns a list of ntaps float coefficients, symmetric around the centre.
    Each value is in the range (-1, 1) for well-chosen fc/fs pairs.
    """
    win_fn = WINDOWS.get(window)
    if win_fn is None:
        raise ValueError(f"Unknown window '{window}'. Choose from: {list(WINDOWS)}")

    cutoff = 2.0 * fc / fs          # normalised cutoff [0, 1]
    centre = (ntaps - 1) / 2.0

    coeffs: list[float] = []
    for n in range(ntaps):
        shift = n - centre
        if shift == 0.0:
            h = cutoff
        else:
            h = math.sin(math.pi * cutoff * shift) / (math.pi * shift)
        w = win_fn(n, ntaps)
        coeffs.append(h * w)

    # Normalise to unity DC gain (coefficients sum to 1.0).
    dc_gain = sum(coeffs)
    coeffs = [c / dc_gain for c in coeffs]

    # Warn if any coefficient exceeds the Q15 representable range.
    max_abs = max(abs(c) for c in coeffs)
    if max_abs >= 1.0:
        import sys
        print(
            f"WARNING: max |coeff| = {max_abs:.4f} exceeds Q15 range [-1, 1). "
            "Consider increasing NTAPS or reducing fc/fs.",
            file=sys.stderr,
        )

    return coeffs


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate FIR low-pass coefficients.")
    parser.add_argument("--ntaps",  type=int,   default=32,     help="Number of taps (default: 32)")
    parser.add_argument("--fc",     type=float, required=True,  help="Corner frequency in Hz")
    parser.add_argument("--fs",     type=float, default=125e6,  help="Sample frequency in Hz (default: 125e6)")
    parser.add_argument("--window", type=str,   default="hamming",
                        choices=list(WINDOWS),  help="Window function (default: hamming)")
    parser.add_argument("--out",    type=str,   default="fir_coeffs.csv",
                        help="Output CSV file (default: fir_coeffs.csv)")
    args = parser.parse_args()

    if args.fc <= 0 or args.fc >= args.fs / 2:
        parser.error(f"fc must be in (0, fs/2) = (0, {args.fs/2:.0f} Hz)")

    coeffs = design_lowpass_fir(args.ntaps, args.fc, args.fs, args.window)

    with open(args.out, "w") as f:
        for c in coeffs:
            f.write(f"{c:.10f}\n")

    print(f"Written {args.ntaps} coefficients to '{args.out}'")
    print(f"  fc={args.fc:.0f} Hz  fs={args.fs:.0f} Hz  window={args.window}")
    print(f"  DC gain: {sum(coeffs):.6f}  max |coeff|: {max(abs(c) for c in coeffs):.6f}")


if __name__ == "__main__":
    main()
