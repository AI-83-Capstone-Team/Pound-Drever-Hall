#!/usr/bin/env python3

import socket

SERVER_IP = "10.42.0.62"
SERVER_PORT = 5555
RP_CH_1 = 0
NUM_SAMPLES = 100

def main():
    '''
    cmds = [(
        "CMD:lock_in\n"
        "F:1.0,-1.0,0.01\n"
        "U:0,0\n"
    ),
    (
        "CMD:rf_read\n"
        "U:0,100\n"
    ),
    (
        "CMD:rf_write\n"
        "U:0,5,1\n"
        "F:0.67,0,0.1\n"
    ),
    (
        "CMD:rf_read\n"
        "U:0,100\n"
    )
    ]
    '''

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
        "U:67\n"
    )
    ]

    for cmd in cmds:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((SERVER_IP, SERVER_PORT))
            s.sendall(cmd.encode("ascii"))
            data = s.recv(4096)

        print("Response from server:")
        print(data.decode("ascii", errors="replace"))

if __name__ == "__main__":
    main()
