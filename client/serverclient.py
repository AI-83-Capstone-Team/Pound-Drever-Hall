#!/usr/bin/env python3
import re
import time
import os
import socket
import math
import paramiko
import numpy as np
import matplotlib
matplotlib.use("Qt5Agg")
import matplotlib.pyplot as plt

SERVER_IP = "10.42.0.62"
SERVER_PORT = 5555
RP_CH_1 = 0
NUM_SAMPLES = 100


REMOTE_BUILD_DIR = "sw/build"
REMOTE_CSV_NAME = "dma_log.csv"
LOCAL_DATA_DIR = "data"

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


def reset_fpga():
    cmds = [
    (
        "CMD:reset_fpga\n"
    )
    ]
    execute_cmd_seq(cmds)

def set_led(value: int):
    cmds = [
    (
        "CMD:set_led\n"
        f"U:{value}\n"
    )
    ]
    execute_cmd_seq(cmds)

def set_dac(value: float, dac_sel: bool):
    cmds = [
    (
        "CMD:set_dac\n"
        f"U:{dac_sel},1\n"
        f"F:{value}\n"
    )
    ]
    execute_cmd_seq(cmds)

def get_adc():
    cmds = [
    (
        "CMD:get_adc\n"
    )
    ]
    execute_cmd_seq(cmds)

def check_signed(adc_sel: bool):
    cmds = [
    (
        "CMD:check_signed\n"
        f"U:{adc_sel}\n"
    )
    ]
    execute_cmd_seq(cmds)

def set_rotation(theta_deg: float):
    theta_rad = math.pi * theta_deg / 180.0
    cmds = [
    (
        "CMD:set_rotation\n"
        f"F:{theta_rad}\n"
    )
    ]
    execute_cmd_seq(cmds)

def get_frame(decimation, frame_code):
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
    x = np.arrange(0, 0x20000/8, 1)
    y1 = data[:, 0]
    y2 = data[:, 1]
    y3 = data[:, 2]
    y4 = data[:, 3]
    plt.plot(x, y1, label="y1")
    plt.plot(x, y2, label="y2")
    plt.plot(x, y3, label="y3")
    plt.plot(x, y4, label="y4")
    plt.legend()
    plt.grid(True)
    plt.show()




def test_frame(byte_offset):
    cmds = [
    (
        "CMD:test_frame\n"
        f"U:{byte_offset}\n"
    )
    ]
    execute_cmd_seq(cmds)


if __name__ == "__main__":
    reset_fpga()
    ##while True:

    #get_adc()
    #set_led(67)
    set_dac(1.0, 0)
    set_dac(-1.0, 1)
    get_frame(1000, 0)

    #check_signed(8)
    #check_signed(9)

    test_frame(0x00000) #garbage until 0X80000 sharp idk why
    test_frame(0x000001FFF8) #garbage until 0X80000 sharp idk why

    #get_adc()



    #set_rotation(0)
    check_signed(6)
    check_signed(7)
    get_adc()

    #set_rotation(180)
    #check_signed(6)
    #check_signed(7)


