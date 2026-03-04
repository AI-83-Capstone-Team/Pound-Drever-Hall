#!/usr/bin/env python3
"""Run the pdh_core cocotb simulation.

Usage:
    python run.py                   # Verilator (default)
    python run.py --sim icarus
    SIM=icarus python run.py
"""

import argparse
import os
import pathlib
import subprocess
import sys

HERE = pathlib.Path(__file__).resolve().parent

# Derive cocotb makefiles path from this Python interpreter's bin dir so that
# make never needs cocotb-config on its own PATH.
_bin = pathlib.Path(sys.executable).parent
_cocotb_config = _bin / "cocotb-config"
_result = subprocess.run([str(_cocotb_config), "--makefiles"],
                         capture_output=True, text=True)
if _result.returncode != 0:
    sys.exit(f"cocotb-config failed: {_result.stderr.strip()}")
COCOTB_MAKEFILES = _result.stdout.strip()


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

    result = subprocess.run(
        [
            "make", "-f", "Makefile.cocotb",
            f"SIM={args.sim}",
            f"COCOTB_MAKEFILES={COCOTB_MAKEFILES}",
            f"PYTHON_BIN={sys.executable}",
        ],
        cwd=HERE,
        env=env,
    )
    sys.exit(result.returncode)


if __name__ == "__main__":
    main()
