import re
import os
import socket
import math
import paramiko
import numpy as np
import matplotlib
matplotlib.use("Qt5Agg")
import matplotlib.pyplot as plt
import re
import io
from functools import wraps
from contextlib import redirect_stdout
SERVER_IP = "10.42.0.62"
SERVER_PORT = 5555



__all__ = [
    'api_reset_fpga',
    'api_set_led',
    'api_get_adc',
    'api_set_dac',
    'api_set_pid',
    'api_set_rotation',
    'api_get_frame',
    'api_check_signed'
]




REMOTE_BUILD_DIR = "sw/build"
REMOTE_CSV_NAME = "dma_log.csv"
LOCAL_DATA_DIR = "data"


_STATUS = re.compile(r'(?mi)^\s*status\s*:\s*(-?\d+)\s*$')

def _st(s: str):
    m = _STATUS.search(s)
    return int(m.group(1)) if m else None

def api_cmd(fn):
    @wraps(fn)
    def w(*a, **k):
        buf = io.StringIO()
        with redirect_stdout(buf):
            r = fn(*a, **k)
        out = buf.getvalue()
        st = _st(out)
        if st is None:
            raise RuntimeError(f"{fn.__name__}: no status")
        if st != 0:
            raise RuntimeError(f"{fn.__name__}: status {st}")
        print(out, end="")  # preserve original prints
        return r
    return w


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


def execute_cmd_seq(cmds):
    for cmd in cmds:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((SERVER_IP, SERVER_PORT))
            s.sendall(cmd.encode("ascii"))
            data = s.recv(4096)

        print("Response from server:")
        print(data.decode("ascii", errors="replace"))


@api_cmd
def api_reset_fpga():
    cmds = [
    (
        "CMD:reset_fpga\n"
    )
    ]
    execute_cmd_seq(cmds)


@api_cmd
def api_set_led(value: int):
    cmds = [
    (
        "CMD:set_led\n"
        f"U:{value}\n"
    )
    ]
    execute_cmd_seq(cmds)


@api_cmd
def api_set_dac(value: float, dac_sel: bool):
    cmds = [
    (
        "CMD:set_dac\n"
        f"U:{dac_sel}\n"
        f"F:{value}\n"
    )
    ]
    execute_cmd_seq(cmds)


@api_cmd
def api_get_adc():
    cmds = [
    (
        "CMD:get_adc\n"
    )
    ]
    execute_cmd_seq(cmds)


@api_cmd
def api_check_signed(adc_sel):
    cmds = [
    (
        "CMD:check_signed\n"
        f"U:{adc_sel}\n"
    )
    ]
    execute_cmd_seq(cmds)


@api_cmd
def api_set_rotation(theta_deg: float):
    theta_rad = math.pi * theta_deg / 180.0
    cmds = [
    (
        "CMD:set_rotation\n"
        f"F:{theta_rad}\n"
    )
    ]
    execute_cmd_seq(cmds)


@api_cmd
def api_get_frame(decimation, frame_code):
    cmds = [
    (
        "CMD:get_frame\n"
        f"U:{decimation},{frame_code}\n"
    )
    ]
    execute_cmd_seq(cmds)
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect("10.42.0.62", username="root", password="root")
    csv = fetch_remote_csv(ssh, f"dma_{frame_code}.csv")
    ssh.close()
    data = np.loadtxt(csv, delimiter=",")
    x = np.arange(16384)
    y1 = data[:, 0]
    y2 = data[:, 1]
    y3 = data[:, 2]
    if frame_code != 2:
        y4 = data[:, 3]
    plt.plot(x, y1, label="y1")
    plt.plot(x, y2, label="y2")
    plt.plot(x, y3, label="y3")
    if frame_code != 2:
        plt.plot(x, y4, label="y4")
    plt.legend()
    plt.grid(True)
    plt.show()


@api_cmd
def api_test_frame(byte_offset):
    cmds = [
    (
        "CMD:test_frame\n"
        f"U:{byte_offset}\n"
    )
    ]
    execute_cmd_seq(cmds)


@api_cmd
def api_set_pid(kp, kd, ki, sp, dec, alpha, sat, en):
    cmds = [
    (
        "CMD:set_pid\n"
        f"F:{kp},{kd},{ki},{sp}\n"
        f"U:{dec},{alpha},{sat},{en}\n"
    )
    ]
    execute_cmd_seq(cmds)
