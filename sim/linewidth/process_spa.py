"""
Parse an Anritsu .spa spectrum file and extract TRACE A data.

Usage:
    python parse_spa.py linewidth_1.spa -o spectrum.csv
    python parse_spa.py linewidth_1.spa 
"""

import argparse
import csv
import re
from typing import List, Tuple


TRACE_A_HEADER = "# Begin TRACE A Data"
DATA_DONE_MARKER = "# Data Done"
0
# Regex for lines like:
# P_0=10.128000 , 0.000001 MHz
P_LINE_RE = re.compile(
    r"""P_(\d+)\s*=\s*([-+0-9.Ee]+)\s*,\s*([-+0-9.Ee]+)\s*MHz"""
)


def parse_trace_a(spa_path: str) -> Tuple[List[float], List[float]]:
    """
    Parse TRACE A from an Anritsu .spa file.

    Returns:
        freqs_mhz: list of frequencies in MHz
        powers_dbm: list of powers in dBm
    """
    with open(spa_path, "r", encoding="ascii", errors="ignore") as f:
        lines = f.readlines()

    in_trace = False
    freqs_mhz: List[float] = []
    powers_dbm: List[float] = []

    for line in lines:
        if TRACE_A_HEADER in line:
            in_trace = True
            continue

        if in_trace:
            # End of data block
            if line.strip().startswith(DATA_DONE_MARKER):
                break

            m = P_LINE_RE.search(line)
            if m:
                idx_str, power_str, freq_str = m.groups()
                power_dbm = float(power_str)
                freq_mhz = float(freq_str)
                powers_dbm.append(power_dbm)
                freqs_mhz.append(freq_mhz)

    if not freqs_mhz:
        raise RuntimeError("No TRACE A data found in file.")

    return freqs_mhz, powers_dbm


def main():
    parser = argparse.ArgumentParser(
        description="Parse Anritsu .spa spectrum (TRACE A) into CSV."
    )
    parser.add_argument("spa_file", help=".spa file path")
    parser.add_argument(
        "-o", "--out",
        help="Output CSV file (default: print to stdout)",
        default=None,
    )
    parser.add_argument(
        "--hz",
        action="store_true",
        help="Output frequency in Hz instead of MHz",
    )
    args = parser.parse_args()

    freqs_mhz, powers_dbm = parse_trace_a(args.spa_file)

    if args.hz:
        freqs = [f * 1e6 for f in freqs_mhz]
        freq_header = "freq_Hz"
    else:
        freqs = freqs_mhz
        freq_header = "freq_MHz"

    rows = list(zip(freqs, powers_dbm))

    if args.out:
        with open(args.out, "w", newline="") as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow([freq_header, "power_dBm"])
            writer.writerows(rows)
    else:
        # Print as CSV to stdout
        print(f"{freq_header},power_dBm")
        for f, p in rows:
            print(f"{f},{p}")


if __name__ == "__main__":
    main()
