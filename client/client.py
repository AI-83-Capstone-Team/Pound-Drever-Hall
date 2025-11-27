import argparse
import os
import numpy as np

import matplotlib
matplotlib.use("Qt5Agg")

import matplotlib.pyplot as plt
import paramiko

TARG_DIR = "Pound-Drever-Hall/core/sw/build"
TARG_TEST = "./lock_in_test"
CSV_NAME = "lockin_log.csv"
SW_DIR = "Pound-Drever-Hall/core/sw"


def ensure_remote_binary(ssh, executable):

    check_cmd = f'cd {TARG_DIR} && [ -x {TARG_TEST} ] && echo "FOUND" || echo "MISSING"'
    stdin, stdout, stderr = ssh.exec_command(check_cmd)
    status = stdout.read().decode().strip()

    if status == "FOUND":
        print("Remote binary found.")
        return

    print("Remote binary missing, running make...")
    make_cmd = f"cd {SW_DIR} && make {executable}"
    stdin, stdout, stderr = ssh.exec_command(make_cmd)

    exit_status = stdout.channel.recv_exit_status()
    out = stdout.read().decode()
    err = stderr.read().decode()

    if out:
        print("make stdout:\n", out)
    if err:
        print("make stderr:\n", err)

    if exit_status != 0:
        raise RuntimeError(f"`make` failed with exit status {exit_status}")

    stdin, stdout, stderr = ssh.exec_command(check_cmd)
    status = stdout.read().decode().strip()
    if status != "FOUND":
        raise RuntimeError("lock_in_test still not found after make.")


def lock_in(args):
    ip = args.ip
    user = args.user
    pwd = args.pwd

    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(ip, username=user, password=pwd)

    try:
        ensure_remote_binary(ssh, "lock_in_test")
        print("Running lock_in_test...")
        cmd = f"cd {TARG_DIR} && {TARG_TEST}"
        stdin, stdout, stderr = ssh.exec_command(cmd)

        exit_status = stdout.channel.recv_exit_status()
        out = stdout.read().decode()
        err = stderr.read().decode()

        if out:
            print("lock_in_test stdout:\n", out)
        if err:
            print("lock_in_test stderr:\n", err)

        if exit_status != 0:
            raise RuntimeError(f"lock_in_test failed with exit status {exit_status}")

        os.makedirs("data", exist_ok=True)

        sftp = ssh.open_sftp()
        remote_csv = f"{TARG_DIR}/{CSV_NAME}"
        local_csv = f"data/{CSV_NAME}"
        sftp.get(remote_csv, local_csv)
        sftp.close()

    finally:
        ssh.close()

    data = np.loadtxt(local_csv, delimiter=",")

    x = data[:, 0]
    y1 = data[:, 1]
    y2 = data[:, 2]

    plt.plot(x, y1, label="Unnormalized")
    plt.plot(x, y2, label="Normalized")

    plt.xlabel("x")
    plt.ylabel("y")
    plt.legend()
    plt.grid(True)
    plt.show()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--ip", type=str, default="10.42.0.62")
    parser.add_argument("--user", type=str, default="root")
    parser.add_argument("--pwd", type=str, default="root")
    args = parser.parse_args()

    lock_in(args)