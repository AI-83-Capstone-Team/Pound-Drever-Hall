import math
import argparse
from pathlib import Path

def gen_qtr_wave(n: int, dw: int):
    """
    Quarter-wave LUT: sin(theta) for theta in [0, pi/2).
    Returns unsigned integers in [0, 2^dw - 1].
    """
    maxv = (1 << dw) - 1

    # Use midpoint sampling to reduce edge bias (nice for DDS/LUT use)
    # theta_k = (k + 0.5) * (pi/2) / n
    vals = []
    for k in range(n):
        theta = (k + 0.5) * (math.pi / 2.0) / n
        s = math.sin(theta)
        v = int(round(s * maxv))
        if v < 0: v = 0
        if v > maxv: v = maxv
        vals.append(v)
    return vals

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--entries", type=int, default=4096, help="Quarter-wave entries (default 4096)")
    ap.add_argument("--dw", type=int, default=15, help="Data width bits (default 16)")
    ap.add_argument("--out", type=str, default="rtl/sine_qtr_4096_16b.mem", help="Output .mem path")
    args = ap.parse_args()

    n = args.entries
    dw = args.dw
    outp = Path(args.out)
    outp.parent.mkdir(parents=True, exist_ok=True)

    vals = gen_qtr_wave(n, dw)

    # Hex digits per entry
    hex_digits = (dw + 3) // 4

    with outp.open("w") as f:
        for v in vals:
            f.write(f"{v:0{hex_digits}X}\n")

    print(f"Wrote {n} entries, {dw}-bit unsigned hex -> {outp}")

if __name__ == "__main__":
    main()
