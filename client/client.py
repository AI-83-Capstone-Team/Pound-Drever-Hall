import argparse
import os
from copy import deepcopy

import numpy as np

import matplotlib
matplotlib.use("Qt5Agg")

import matplotlib.pyplot as plt
import paramiko

REMOTE_BUILD_DIR = "Pound-Drever-Hall/core/sw/build"
REMOTE_SRC_DIR = "Pound-Drever-Hall/core/sw"
EXECUTABLE_NAME = "lock_in_test"
REMOTE_EXECUTABLE = f"./{EXECUTABLE_NAME}"
REMOTE_CSV_NAME = "lockin_log.csv"
LOCAL_DATA_DIR = "data"


def run_remote(ssh: paramiko.SSHClient, cmd: str):
    stdin, stdout, stderr = ssh.exec_command(cmd)
    exit_status = stdout.channel.recv_exit_status()
    out = stdout.read().decode()
    err = stderr.read().decode()
    return exit_status, out, err


def ensure_remote_binary(ssh: paramiko.SSHClient, executable: str = EXECUTABLE_NAME) -> None:
    check_cmd = (
        f"cd {REMOTE_BUILD_DIR} && [ -x ./{executable} ] && echo FOUND || echo MISSING"
    )
    status, out, err = run_remote(ssh, check_cmd)
    found = out.strip() == "FOUND"

    if found:
        print("Remote binary found.")
        return

    print("Remote binary missing, running make...")
    make_cmd = f"cd {REMOTE_SRC_DIR} && make {executable}"
    status, out, err = run_remote(ssh, make_cmd)

    if out:
        print("make stdout:\n", out)
    if err:
        print("make stderr:\n", err)

    if status != 0:
        raise RuntimeError(f"`make` failed with exit status {status}")

    # Verify again
    status, out, err = run_remote(ssh, check_cmd)
    if out.strip() != "FOUND":
        raise RuntimeError(f"{executable} still not found after make.")


def fetch_remote_csv(ssh: paramiko.SSHClient, local_name: str) -> str:
    os.makedirs(LOCAL_DATA_DIR, exist_ok=True)
    remote_csv = f"{REMOTE_BUILD_DIR}/{REMOTE_CSV_NAME}"
    local_csv = os.path.join(LOCAL_DATA_DIR, local_name)

    sftp = ssh.open_sftp()
    try:
        sftp.get(remote_csv, local_csv)
    finally:
        sftp.close()

    return local_csv


def plot_csv(local_csv: str) -> None:
    data = np.loadtxt(local_csv, delimiter=",")

    x = data[:, 0]
    y1 = data[:, 1]
    y2 = data[:, 2]

    plt.plot(x, y1, label="Unnormalized")
    plt.plot(x, y2, label="Normalized")

    plt.legend()
    plt.grid(True)
    plt.show()


def lock_in(args, local_csv_name: str) -> str:
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(args.ip, username=args.user, password=args.pwd)

    try:
        ensure_remote_binary(ssh, EXECUTABLE_NAME)

        print("Running lock_in_test...")
        cmd = (
            f"cd {REMOTE_BUILD_DIR} && {REMOTE_EXECUTABLE} "
            f"{args.dac_low} {args.dac_high} {args.dac_step}"
        )
        status, out, err = run_remote(ssh, cmd)

        if out:
            print("lock_in_test stdout:\n", out)
        if err:
            print("lock_in_test stderr:\n", err)

        if status != 0:
            raise RuntimeError(f"{EXECUTABLE_NAME} failed with exit status {status}")

        return fetch_remote_csv(ssh, local_csv_name)
    finally:
        ssh.close()


def hysteresis_scan(args) -> None:
    # forward
    forward_csv = lock_in(args, "forward.csv")

    # reverse scan (clone and flip DAC range)
    rev_args = deepcopy(args)
    rev_args.dac_low *= -1
    rev_args.dac_high *= -1
    rev_args.dac_step *= -1
    reverse_csv = lock_in(rev_args, "reverse.csv")

    fwd = np.loadtxt(forward_csv, delimiter=",")
    rev = np.loadtxt(reverse_csv, delimiter=",")

    x = fwd[:, 0]
    y1 = fwd[:, 1]
    y2 = fwd[:, 2]
    y3 = rev[:, 1]
    y4 = rev[:, 2]

    plt.plot(x, y1, label="Unnormalized forward")
    plt.plot(x, y2, label="Normalized forward")
    plt.plot(x, y3, label="Unnormalized reverse")
    plt.plot(x, y4, label="Normalized reverse")

    plt.legend()
    plt.grid(True)
    plt.show()


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ip", type=str, default="10.42.0.62")
    parser.add_argument("--user", type=str, default="root")
    parser.add_argument("--pwd", type=str, default="root")
    parser.add_argument("--dac_low", type=float, default=-1.0)
    parser.add_argument("--dac_high", type=float, default=1.0)
    parser.add_argument("--dac_step", type=float, default=0.01)
    return parser.parse_args()


if __name__ == "__main__":
    args = parse_args()
    hysteresis_scan(args)
