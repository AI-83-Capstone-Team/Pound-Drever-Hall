# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a hardware/software co-design project implementing a **Pound-Drever-Hall (PDH) laser frequency locking system** on a Red Pitaya (Zynq-7000 SoC). The system uses an FPGA for real-time signal processing and a C server running on the ARM cores for control, with a Python client API for remote operation from a host machine.

## Build Commands

### RTL (FPGA Bitstream)
```bash
cd core/hw
make pdh_core       # Builds boot.bin via Vivado in batch mode
make clean
```
Requires Vivado and `bootgen` in PATH. Output: `build/boot.bin`.

### Software (C Server)
```bash
cd core/sw
make server         # Builds server binary (cross-compiled for ARM)
make clean
```
The server binary is statically linked (`-static -lm -lstdc++ -lpthread`).

### Deploy (build + transfer + run on Red Pitaya)
```bash
cd core
./deploy.sh         # Copies boot.bin and sw/ to RP, programs FPGA, builds and runs server
```
Target: `root@10.42.0.62` (password: `root`). Requires `sshpass` and `scp`.

### Client (Python)
```bash
cd client
python test.py      # Example script using the pdh_api
```
Requires: `paramiko`, `numpy`, `matplotlib` (Qt5Agg backend).

## Architecture

### System Flow
```
Host PC (Python client) --TCP:5555--> ARM Server (C) --AXI/mmap--> FPGA (SystemVerilog)
                                                       <--AXI/mmap--
                                       DMA (HP0 port, 0x10000000) <-- FPGA capture buffer
```

### FPGA RTL (`core/hw/rtl/`)
- `pdh_top.sv` — Top-level, instantiates everything, connects to Red Pitaya AXI GP0 slave and HP0 DMA master
- `pdh_core.sv` — Main control FSM; decodes 32-bit commands from ARM, drives submodules, writes 32-bit callbacks back to ARM
- `nco.sv` — Numerically controlled oscillator (LO generation); uses `sine_qtr_rom.sv` with `sine_qtr_4096_16b.mem`
- `pid_core.sv` — PID controller with configurable Kp/Ki/Kd/setpoint/decimation/saturation (Q15 fixed-point)
- `bram_controller.sv` / `dc_bram.sv` — BRAM-based capture buffer for DMA transfers
- `dma_controller.sv` — Streams BRAM data to DDR via HP0 AXI port
- `posedge_detector.sv` — Edge detection utility

### C Server (`core/sw/`)
- `server.c` — TCP server on port 5555; single-threaded, one connection per command. Parses text protocol, dispatches to command handlers, sends response.
- `control/control.c` — All command handler implementations (`cmd_*` functions)
- `control/hw_common.c` — Hardware abstraction: `mmap`s `/dev/mem` for AXI GP0 (commands at `0x42000000+8`, readback at `+0`) and HP0 DMA region (`0x10000000`, 128KB)
- `control/inc/server.h` — `cmd_ctx_t` struct (input args + output items), `cmd_entry_t` dispatch table entry
- `control/inc/hw_common.h` — All hardware type definitions: `pdh_cmd_t` (32-bit packed union for commands), `pdh_callback_t` (32-bit packed union for FPGA readback), `dma_frame_t` (64-bit packed union for DMA data), all command/status enums

### Python Client (`client/pdh_api/api.py`)
- Wraps each command as a TCP send/receive to the server
- `@api_cmd` decorator enforces `status:0` checks and raises `RuntimeError` on failure
- `SERVER_IP = "10.42.0.62"` is hardcoded; change here to target a different device
- `api_get_frame` and `api_lock_in` additionally SSH into the RP to `sftp` the resulting CSV, then plot it

## Command Protocol (Text over TCP)

```
CMD:<command_name>\n
F:<float1>,<float2>,...\n    (optional float args)
I:<int1>,<int2>,...\n        (optional signed int args)
U:<uint1>,<uint2>,...\n      (optional unsigned int args)
```

Response format:
```
type:output\nname:<cmd>\nstatus:<int>\n<key>:<value>\n...
```

## Key Design Patterns

**Command-callback pattern**: Every FPGA command is sent twice (strobe 0→1) via `pdh_execute_cmd()`. The FPGA echoes back the written values in the callback register, which the server validates against what was sent (`validate_cb`).

**Q15 fixed-point**: PID coefficients and rotation matrix values use Q15 format (divide by 32768 to get float in `[-1, 1)`). Setpoint uses Q13 (divide by 8192).

**NCO frequency**: stride = `freq / STRIDE_CONST` where `STRIDE_CONST = 125e6 / (4 * 4096) ≈ 7629.4 Hz/stride`. Valid stride range: 1–1024.

**DMA frames**: `cmd_get_frame` triggers FPGA to DMA 128KB (`0x20000` bytes) of 64-bit samples to `0x10000000`. The server sleeps `DMA_BURST_CONST + BRAM_DEC_CONST * decimation` µs, then reads the region and writes a CSV. Frame type selects which signals are packed into each 64-bit word (see `frame_code_e` and `dma_frame_t` in `hw_common.h`).

**IO routing** (`cmd_config_io`): Each DAC output can be sourced from: register (manual), PID output, NCO1, or NCO2. The PID error input can be sourced from: I-feed, Q-feed, ADC_A, or ADC_B.

## Simulation (`sim/`)
MATLAB scripts (`pdh.m`, `power_terms.m`) for system-level PDH signal simulation. Not part of the build.



# KEY RULES (Invariants)
The following rules must never be broken under any circumstances.

1. Do not modify any of the RTL code (systemverilog files) unless explicitly asked or under the following constraint: If you need to modify the code you must explicitly ask to do so and detail exactly what you intend to change. Only after explicit acknowledgement by the user are you allowed to proceed.

2. Do not modify the interface between the Software and the Hardware subject to the same constraints as invariant 1.

3. Whenever a change is made that results in a difference from what is currently in the .README, update the .README accordingly.





# Python Environment
Use the following venv for running any python code:
``` bash
source myenv/bin/activate
```

If you write code that requires a new library not already inside requirements.txt, update requirements.txt accordingly











