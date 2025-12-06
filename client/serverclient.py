#!/usr/bin/env python3

import socket

SERVER_IP = "10.42.0.62"
SERVER_PORT = 5555

def main():
    # Build the command string exactly how your server expects it
    cmd = (
        "CMD:lock_in\n"
        "F:1.0,-1.0,0.01\n"   # dac_end, dac_start, dac_step (example)
        "U:0,1\n"           # chin=0, chout=1 (example)
    )

    # Create a TCP socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        # Connect to the server
        s.connect((SERVER_IP, SERVER_PORT))

        # Send the command as bytes
        s.sendall(cmd.encode("ascii"))

        # Receive response (up to 4 KB, adjust if needed)
        data = s.recv(4096)

    # Print what we got back
    print("Response from server:")
    print(data.decode("ascii", errors="replace"))

if __name__ == "__main__":
    main()