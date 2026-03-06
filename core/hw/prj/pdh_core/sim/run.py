#!/usr/bin/env python3
"""Run the pdh_core cocotb regression test.

Artifacts (run info) are written to sim/artifacts/pdh_core/pdh_core_N/.

Usage:
    python run.py                   # Verilator (default)
    python run.py --sim icarus
    SIM=icarus python run.py
"""

import argparse
import datetime
import json
import os
import pathlib
import subprocess
import sys

HERE = pathlib.Path(__file__).resolve().parent

_bin = pathlib.Path(sys.executable).parent
_cocotb_config = _bin / "cocotb-config"
_result = subprocess.run([str(_cocotb_config), "--makefiles"],
                         capture_output=True, text=True)
if _result.returncode != 0:
    sys.exit(f"cocotb-config failed: {_result.stderr.strip()}")
COCOTB_MAKEFILES = _result.stdout.strip()


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
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--sim",
        default=os.environ.get("SIM", "verilator"),
        choices=["icarus", "verilator"],
    )
    args = parser.parse_args()

    artifact_dir = _make_artifact_dir("pdh_core")

    env = os.environ.copy()
    env["SIM"] = args.sim

    started = datetime.datetime.now().isoformat()
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

    info = {
        "sim":       args.sim,
        "started":   started,
        "finished":  datetime.datetime.now().isoformat(),
        "exit_code": result.returncode,
    }
    with open(artifact_dir / "run_info.json", "w") as f:
        json.dump(info, f, indent=2)

    print(f"\nArtifacts written to: {artifact_dir}")
    sys.exit(result.returncode)


if __name__ == "__main__":
    main()
