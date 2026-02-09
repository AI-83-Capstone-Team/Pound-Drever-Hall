#!/usr/bin/env python3
import re
import time
import socket
import math

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

def get_frame(decimation):
    cmds = [
    (
        "CMD:get_frame\n"
        f"U:{decimation}\n"
    )
    ]
    execute_cmd_seq(cmds)


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

    #get_frame(10000)

    #check_signed(8)
    #check_signed(9)

    #test_frame(0x00000) #garbage until 0X80000 sharp idk why
    #test_frame(0x000001FFF8) #garbage until 0X80000 sharp idk why

    #get_adc()


    set_dac(1.0, 0)
    set_dac(-1.0, 1)
    #set_rotation(0)
    check_signed(6)
    check_signed(7)
    get_adc()

    #set_rotation(180)
    #check_signed(6)
    #check_signed(7)


