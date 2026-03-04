#!/usr/bin/env python3
"""Run the pdh_core cocotb simulation.

Usage:
    python run.py                   # Icarus Verilog (default)
    python run.py --sim verilator
    SIM=verilator python run.py
"""

import argparse
import os
import subprocess
import sys
import pathlib

HERE = pathlib.Path(__file__).resolve().parent


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--sim",
        default=os.environ.get("SIM", "verilator"),
        choices=["icarus", "verilator"],
    )
    args = parser.parse_args()

    env = os.environ.copy()
    env["SIM"] = args.sim

    result = subprocess.run(["make", "-f", "Makefile.cocotb", f"SIM={args.sim}"],
                            cwd=HERE, env=env)
    sys.exit(result.returncode)


if __name__ == "__main__":
    main()
