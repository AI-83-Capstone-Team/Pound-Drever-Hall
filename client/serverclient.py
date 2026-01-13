#!/usr/bin/env python3
import re
import time
import socket

SERVER_IP = "10.42.0.62"
SERVER_PORT = 5555
RP_CH_1 = 0
NUM_SAMPLES = 100

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

def set_led(value: int, strobe: bool = 0):
    cmds = [
    (
        "CMD:set_led\n"
        f"U:{value},{strobe}\n"
    )
    ]
    execute_cmd_seq(cmds)

def set_dac(value: float, dac_sel: bool, strobe: bool = 0):
    cmds = [
    (
        "CMD:set_dac\n"
        f"U:{dac_sel},{strobe}\n"
        f"F:{value}\n"
    )
    ]
    execute_cmd_seq(cmds)



if __name__ == "__main__":
    reset_fpga()
    while True:
        set_led(6, 0)
        set_led(7, 0)
        set_dac(-0.15, 0, 0)
        set_dac(1.45, 1, 0)
