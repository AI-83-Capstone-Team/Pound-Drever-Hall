#!/usr/bin/env python3
import time
import socket

SERVER_IP = "10.42.0.62"
SERVER_PORT = 5555
RP_CH_1 = 0
NUM_SAMPLES = 100

def set_led(value):

    cmds = [
    (
        "CMD:reset_fpga\n"
        "U:1\n"
    ),
    (
        "CMD:reset_fpga\n"
        "U:0\n"
    ),
    (
        "CMD:set_led\n"
        f"U:{value}\n"
    ),
    (
        "CMD:strobe_fpga\n"
    ),
    ]

    for cmd in cmds:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((SERVER_IP, SERVER_PORT))
            s.sendall(cmd.encode("ascii"))
            data = s.recv(4096)

        print("Response from server:")
        print(data.decode("ascii", errors="replace"))

if __name__ == "__main__":
    while True:
        set_led(6)
        time.sleep(1)
        set_led(7)
        time.sleep(1)
