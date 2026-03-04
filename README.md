# Pound-Drever-Hall Laser Locking System

A hardware/software co-design implementation of a **Pound-Drever-Hall (PDH) laser frequency locking system** on a Red Pitaya STEMlab 125-14 (Xilinx Zynq XC7Z010 SoC). Real-time signal processing runs on the FPGA (PL), a TCP control server runs on the ARM cores (PS), and a Python client API provides remote operation from a host machine.

---

## Table of Contents

1. [Repository Structure](#repository-structure)
2. [Build and Deploy](#build-and-deploy)
3. [System Architecture](#system-architecture)
4. [Command Dispatch and Receive Architecture](#command-dispatch-and-receive-architecture)
5. [NCO Design](#nco-design)
6. [PID Controller Design](#pid-controller-design)
7. [DMA Live Capture Design](#dma-live-capture-design)
8. [Signal Representation and Fixed-Point Conventions](#signal-representation-and-fixed-point-conventions)
9. [IO Routing](#io-routing)
10. [Notable Design Choices](#notable-design-choices)

---

## Repository Structure

```
core/
  hw/
    prj/pdh_core/
      rtl/               FPGA RTL (SystemVerilog)
        pdh_top.sv       Board-level wrapper: clocking, PS instantiation, HP0 AXI tie-offs
        pdh_core.sv      Main control module: command decoder, IQ rotation, DMA orchestrator
        nco.sv           Numerically controlled oscillator (dual-output, quarter-ROM)
        pid_core.sv      Discrete PID with EMA derivative, anti-windup, decimation
        bram_controller.sv  Dual-clock BRAM capture buffer (pdh_clk write / fclk0 read)
        dma_controller.sv   AXI4 master for HP0 DDR burst writes
        sine_qtr_rom.sv  4096-entry, 16-bit quarter-sine ROM
        posedge_detector.sv  Rising-edge detector utility
      tb/                Testbenches
  sw/
    server.c             TCP server: argument parsing, dispatch table, response formatting
    control/
      control.c          Command handler implementations (cmd_* functions)
      hw_common.c        Hardware abstraction: mmap of AXI GP0 and HP0 DMA region
      inc/
        server.h         cmd_ctx_t, cmd_entry_t, output_item_t definitions
        hw_common.h      pdh_cmd_t, pdh_callback_t, dma_frame_t packed unions; all enums
        control.h        Command handler declarations
  deploy.sh              Build, copy, and run script for the Red Pitaya
client/
  pdh_api/
    api.py               Python API functions wrapping each TCP command
    types.py             Enums and result dataclasses mirroring the C/RTL types
  test.py                Board-level regression test and visualization script
sim/
  pdh.m                  MATLAB PDH signal simulation
  power_terms.m          MATLAB power series analysis
pcb/                     PCB design files
```

---

## Build and Deploy

### RTL (FPGA Bitstream)

```bash
cd core/hw
make pdh_core       # Runs Vivado in batch mode; output: build/boot.bin
make clean
```

Requires Vivado and `bootgen` in PATH.

### Software (C Server)

```bash
cd core/sw
make server         # Cross-compiles for ARM; statically linked (-static -lm -lstdc++ -lpthread)
make clean
```

### Deploy (all-in-one)

```bash
cd core
./deploy.sh         # Copies boot.bin and sw/ to RP via scp, programs FPGA, builds and starts server
```

Target: `root@10.42.0.62` (password: `root`). Requires `sshpass` and `scp`.

### Python Client

```bash
cd client
python test.py      # Runs the board-level regression test
```

Requires: `paramiko`, `numpy`, `matplotlib`.

---

## System Architecture

```
Host PC (Python)
    |
    | TCP port 5555 (text protocol)
    v
ARM Cortex-A9 (Linux)
    server.c  ──── parses text, dispatches cmd_* handlers
    control.c ──── builds pdh_cmd_t, mmap-writes to AXI GP0
    hw_common.c     reads pdh_callback_t from AXI GP0
    |                       reads dma_frame_t from HP0 DDR region
    |
    | AXI GP0 (0x42000000)  32-bit command/callback
    | AXI HP0 (0x10000000)  64-bit DMA write port
    |
    v
FPGA (Zynq PL, 125 MHz)
    pdh_core.sv  ── command decoder, IQ rotation, IO mux, DMA orchestrator
    nco.sv       ── LO generation for IQ demodulation
    pid_core.sv  ── PID feedback controller
    bram_controller.sv  ── sample capture buffer
    dma_controller.sv   ── AXI4 master → HP0 DDR
```

### Clock Domains

| Domain    | Source                                     | Frequency  | Used for                          |
|-----------|--------------------------------------------|------------|-----------------------------------|
| `pdh_clk` | ADC differential clock (IBUFGDS + BUFG)    | 125 MHz    | All FPGA signal processing        |
| `fclk0`   | PS fabric clock (exported by block design) | configurable | DMA controller, BRAM read port  |

The ADC clock is also routed out as `dac_clk_o` to clock the DAC.

---

## Command Dispatch and Receive Architecture

This is a layered architecture: text protocol → C dispatch table → AXI packed-union commands → FPGA register → callback readback.

### TCP Text Protocol

Every command from the Python client is a single TCP transaction (connect, send, receive, close):

```
CMD:<command_name>\n
F:<f0>,<f1>,...\n     (optional float arguments)
I:<i0>,<i1>,...\n     (optional signed integer arguments)
U:<u0>,<u1>,...\n     (optional unsigned integer arguments)
```

The server responds with:

```
type:output\n
name:<command_name>\n
status:<integer_code>\n
<KEY>:<value>\n
...
```

All values are plain ASCII. The Python client's `_parse_response()` coerces each value to `int` or `float` automatically.

### Server Dispatch Table

`server.c` maintains a static table of 12 entries:

```c
static cmd_entry_t gCmds[NUM_CMDS] = {
    {"set_led",      cmd_set_led,      0, 0, 1},
    {"reset_fpga",   cmd_reset_fpga,   0, 0, 0},
    {"set_dac",      cmd_set_dac,      1, 0, 1},
    {"get_adc",      cmd_get_adc,      0, 0, 0},
    {"check_signed", cmd_check_signed, 0, 0, 1},
    {"set_rotation", cmd_set_rot,      1, 0, 0},
    {"get_frame",    cmd_get_frame,    0, 0, 2},
    {"test_frame",   cmd_test_frame,   0, 0, 1},
    {"set_pid",      cmd_set_pid,      4, 0, 4},
    {"set_nco",      cmd_set_nco,      2, 0, 1},
    {"config_io",    cmd_config_io,    0, 0, 3},
    {"lock_in",      cmd_lock_in,      0, 0, 5},
};
```

Each entry holds: `{name, handler_fn, required_floats, required_ints, required_uints}`. Dispatch performs a linear name search; argument-count mismatches return an error before calling the handler. The handler receives a `cmd_ctx_t` containing the parsed arguments and writes its response into `ctx->output`.

### AXI GP0 Command Register

The ARM accesses the FPGA through a 32-bit AXI GPIO peripheral mapped at physical address `0x42000000`. Write offset `+8` drives the FPGA input register; read offset `+0` reads the FPGA callback register.

The 32-bit command word (`pdh_cmd_t`) is a packed union with this layout:

```
Bit 31   : rst     (async reset, bypasses strobe mechanism)
Bit 30   : strobe  (rising edge triggers command latching in FPGA)
Bits 29:26: cmd    (4-bit command opcode)
Bits 25:0 : data   (26-bit command payload)
```

### Strobe Protocol

Every FPGA command is written twice:

```c
static inline pdh_callback_t pdh_execute_cmd(pdh_cmd_t cmd)
{
    cmd.strobe.val = 0;
    pdh_send_cmd(cmd);      // write with strobe=0
    cmd.strobe.val = 1;
    pdh_send_cmd(cmd);      // write with strobe=1 → triggers FPGA action
    pdh_callback_t cb = { 0 };
    pdh_get_callback(&cb);  // read callback register
    return cb;
}
```

The first write establishes the payload with a known strobe level, preventing spurious edges. The second write asserts the strobe. `pdh_send_cmd` writes to the mmap'd register with a `__sync_synchronize()` memory barrier to ensure the store completes before proceeding.

### FPGA Synchronization and Latching

The AXI bus is in the PS clock domain; `pdh_core` runs at `pdh_clk` (125 MHz). A 3-stage flip-flop chain synchronizes the AXI input:

```
axi_from_ps_i → axi_1ff_r → axi_2ff_r → axi_3ff_r  (all clocked on pdh_clk)
```

A `posedge_detector` on `axi_3ff_r[30]` (the strobe bit) produces a single-cycle pulse `strobe_edge_w`. On that pulse, the synchronized value is latched into `axi_from_ps_r`, which drives the command decoder:

```
next_axi_from_ps_w = strobe_edge_w ? axi_3ff_r : axi_from_ps_r;
```

The command opcode and data payload are then extracted:

```
cmd_w  = axi_from_ps_r[29:26]   // selects which register to update
data_w = axi_from_ps_r[25:0]    // payload value
```

Every register update is a purely combinational assign guarded by `cmd_w`. All guarded values are registered in the main `always_ff` block on the next rising edge of `pdh_clk`.

### Callback Register

The FPGA drives a separate 32-bit output register (`axi_to_ps_o`) whose value is determined combinationally by `cmd_w`. Each command has its own callback word format (defined in `pdh_callback_t`) that echoes back the just-written register values, encoded with the command opcode in bits `[31:28]`.

For example, the `CMD_SET_NCO` callback:

```
Bits 31:28 : cmd opcode (0b1001)
Bit  27    : (padding)
Bits 26:15 : nco_stride_r   (12 bits)
Bits 14:3  : nco_shift_r    (12 bits)
Bit  2     : nco_sub_r
Bit  1     : nco_inv_r
Bit  0     : nco_en_r
```

### Callback Validation

`validate_cb()` in `control.c` compares each echoed field to the intended value and returns a non-zero error code on mismatch:

```c
static inline int validate_cb(void* cb, void* expected, int tag,
    const char* func, const char* name, int success_code, int fail_code);
```

Because the callback reflects the FPGA's registered state immediately after the strobe, a mismatch indicates either a write failure or an AXI contention issue.

### PID Coefficient Multiplexing

Because the PID has eight independent coefficients (kp, kd, ki, dec, sp, alpha, sat, en) but the command word has only 26 payload bits, each coefficient is written in a separate transaction. The 4-bit `coeff_sel` field in bits `[19:16]` of the data word selects which coefficient register is updated. The corresponding callback echoes the selected coefficient's current value. `cmd_set_pid` in `control.c` issues eight sequential `pdh_execute_cmd` calls, one per coefficient.

The NCO uses the same multiplexing scheme with a 3-bit `coeff_sel` for five parameters: stride, shift, invert, sub, enable.

---

## NCO Design

The numerically controlled oscillator generates two phase-coherent sinusoidal outputs for use as local oscillators in IQ demodulation and as DAC drive signals.

### Phase Accumulator

A 14-bit accumulator `phi1_r` increments by `stride_r` on every clock cycle when the NCO is enabled:

```
phi1_r[k+1] = phi1_r[k] + stride_r
```

The accumulator wraps naturally at 2^14 = 16384, representing one full cycle. The output frequency is:

```
f_out = stride × (125 MHz / 16384) = stride × 7629.394... Hz
```

This constant (`STRIDE_CONST = 125e6 / (4 × 4096) = 7629.394 Hz/stride`) is used in `cmd_set_nco` to convert from Hz to stride counts. Valid stride range is 1–1024, giving an output frequency range of approximately **7.6 kHz to 7.8 MHz**.

### Second Output and Phase Offset

The second phase accumulator `phi2_w` is derived from the primary accumulator with an optional phase offset:

```
phi2_w = phi1_r + shift_r    (sub=0, default)
phi2_w = phi1_r - shift_r    (sub=1)
```

`shift_r` is a 12-bit value representing a phase angle within one quadrant (0 to π/2). The full phase offset in degrees is encoded with `sub` and `invert` flags (see the phase-offset encoding in `cmd_set_nco`).

### Quarter-Sine ROM and Quadrant Folding

Rather than storing a full 16384-entry sine table, only the first quadrant (4096 entries, 16-bit signed output) is stored in `sine_qtr_rom`. Quadrant folding reconstructs the full sine wave:

```
phi[13:12] determines the quadrant (Q1=00, Q2=01, Q3=10, Q4=11)

ROM address:
  Q1, Q3 (ascending):   addr = phi[11:0]
  Q2, Q4 (descending):  addr = 4095 - phi[11:0]

Sign:
  Q1, Q2 (positive half): output = +ROM[addr]
  Q3, Q4 (negative half): output = -ROM[addr]
```

This reduces ROM footprint to 1/4 of a full-cycle table with no loss of precision. The `invert` flag additionally negates `out2_o`, allowing the second output to be inverted with respect to its base phase-shifted waveform.

### Pipeline Latency

The ROM has one registered read stage, giving a total pipeline depth of three clock cycles from `phi1_r` update to valid `out1_o`/`out2_o`:

```
Cycle 0: phi1_r updated, phi2_w computed
Cycle 1: phi1_pipe1_r, phi2_pipe1_r registered; addr1_r, addr2_r computed and registered
Cycle 2: ROM outputs registered (rom1_lookup_w read)
Cycle 3: rom1_signed_r, rom2_signed_r registered → out1_o, out2_o valid
```

### DAC Output Conversion

The NCO outputs are 16-bit signed values in Q15 format (range ±32767). The Red Pitaya DAC expects 14-bit unsigned offset-binary (0 = −1 V, 8191 = 0 V, 16383 = +1 V). The conversion function `s16_to_u14` performs:

```sv
function automatic logic [13:0] s16_to_u14(input logic signed [15:0] in);
    logic signed [15:0] t1 = -(in >> 2) + 16'sd8191;
    s16_to_u14 = t1[13:0];
endfunction
```

The arithmetic shift right by 2 scales from the Q15 range (±32767) to the DAC's 14-bit range (±8191), the negation corrects for the DAC's inverted output polarity, and the +8191 offset centers the waveform at 0 V.

---

## PID Controller Design

The PID controller (`pid_core.sv`) is a fully discrete-time, fixed-point implementation with configurable decimation, EMA derivative filtering, anti-windup integration, and output saturation.

### Inputs and Error Signal

```
error_w = dat_i − sp_r
```

- `dat_i`: 16-bit signed process variable (from IQ demodulation or raw ADC, per IO routing)
- `sp_r`: 14-bit signed setpoint in Q13 format (range [−8192, +8191], divide by 8192 to get volts)

Both are in the same numerical scale: the ADC conversion `adc_16s = −(adc_raw − 8192)` produces values in [−8192, +8192], which is identical to the Q13 setpoint scale. At steady state, `dat_i == sp_r` exactly (modulo quantization), which means the setpoint value in volts equals the measured voltage.

### Decimation

The PID update rate is divided down from the 125 MHz clock by `decimate_r`:

```
tick1_w = enable_i && (cnt_r == 0)
cnt_r[k+1] = (cnt_r >= decimate_r − 1) ? 0 : cnt_r + 1
```

P, D, and I terms are only recomputed when `tick2_r` is high (one cycle after `tick1_r`). This reduces effective bandwidth and allows the controller to operate at rates suitable for slower physical systems without changing the FPGA clock.

### Proportional Term

```
p_error_w = kp_r × error_pipe1_r      (32-bit product)
p_error_shifted_w = p_error_r >>> 15  (Q15 normalization)
```

`error_pipe1_r` is `error_w` delayed by one tick to pipeline the multiply. `kp_r` is Q15 signed (range [−1, +1), stored as a 16-bit integer; divide by 32768 to recover the float value).

### Derivative Term with EMA Filter

Raw finite-difference differentiation is noise-sensitive. The derivative is instead computed as the difference between the raw error and an exponential moving average (EMA) of the error, which acts as a low-pass filter:

```
yk_w = ((error_w − yk_r) >>> alpha_r) + yk_r
     = 2^(−alpha) × error_w + (1 − 2^(−alpha)) × yk_r
```

This is the standard first-order IIR (EMA) update. The derivative signal is:

```
d_error_w = kd_r × (error_w − yk_r)   (the high-frequency error component)
d_error_shifted_w = d_error_r >>> 15
```

`alpha_r` controls the EMA time constant: larger `alpha` → longer memory → more smoothing → lower derivative cutoff frequency. Valid range: 0–15.

### Integral Term with Anti-Windup

The integrator accumulates the error each tick:

```
sum_error_wide_w = sum_error_r + error_w     (33-bit to detect overflow)
sum_error1_w = apply_satwidth_truncation(sum_error_wide_w, 2^satwidth_r)
```

`satwidth_r` (range 15–31) bounds the integrator state to ±2^satwidth, preventing excessive windup.

Anti-windup is implemented by freezing the integrator when the output is already saturated and the new error would further increase the accumulator in the same direction:

```c
sum_error2_w = (tick1_r
    && !(pid_out == 0x3FFF && sum_error1_w > sum_error_r)   // saturated high
    && !(pid_out == 0      && sum_error1_w < sum_error_r))  // saturated low
    ? sum_error1_w : sum_error_r;
```

The integral output is:

```
i_error_w = ki_r × sum_error_r
i_error_shifted_w = i_error_r >>> satwidth_r
```

Using `satwidth_r` for both the integrator bound and the I-term right-shift links the two: a tighter saturation also scales down the I-term contribution, keeping gains consistent across different saturation settings.

### Output Mapping

The three terms are summed and mapped to the 14-bit unsigned DAC format:

```
total_error_wide_w = p_error_shifted_w + d_error_shifted_w + i_error_shifted_w
pid_out = sat_unsigned_from_signed(total_error_wide_w + 8191)
```

`sat_unsigned_from_signed` maps the signed 20-bit sum to [0, 16383]:

- Negative input → 0 (minimum DAC, −1 V)
- Input > 16383 → 16383 (maximum DAC, +1 V)
- Otherwise → lower 14 bits

Adding 8191 before the conversion centers the zero-error output at mid-scale (0 V at DAC output), so the controller is unbiased when the error is zero.

### Coefficient Encoding Summary

| Parameter | Format | Range        | Encoding                        |
|-----------|--------|--------------|---------------------------------|
| kp, kd, ki | Q15  | [−1, +1)     | int16 / 32768.0                 |
| sp        | Q13    | [−1, +1)     | int14 / 8192.0                  |
| dec       | uint14 | [1, 16383]   | clock cycles per PID update     |
| alpha     | uint4  | [0, 15]      | EMA time constant exponent      |
| sat       | uint5  | [15, 31]     | integrator bound and I-shift    |

---

## DMA Live Capture Design

The DMA system provides a triggered snapshot of 16384 consecutive 64-bit samples from any combination of internal signals, transferred to DDR and read back by the ARM as a CSV file.

### Overview

The capture pipeline has three stages operating across two clock domains:

```
[pdh_clk] FPGA signals → bram_controller (dual-clock BRAM write)
                          ↓ (CDC: dual-port BRAM)
[fclk0]                dma_controller (AXI4 master → HP0 DDR at 0x10000000)
                          ↓ (memory barrier)
[ARM]                  mmap read → CSV write → SFTP to host
```

### Stage 1: BRAM Capture (`bram_controller.sv`)

A 16384×64-bit dual-clock BRAM (`dc_bram`) captures the FPGA data stream:

- **Write port**: `pdh_clk` domain. The `bram_controller` FSM has two states:
  - `ST_IDLE`: `bram_ready_o = 1`. On a rising edge of `enable_i` (synchronized via `posedge_detector`), the latched `decimation_code_i` is captured and the FSM transitions to `ST_CAPTURE_DATA`.
  - `ST_CAPTURE_DATA`: `bram_ready_o = 0`. A 22-bit counter divides the 125 MHz clock by `decimation_code_r`, writing one sample to BRAM per `decimation_code_r` clocks. The write address advances until `addr_r == 16383`, at which point the FSM returns to `ST_IDLE`.

- **Read port**: `fclk0` domain, driven by the `dma_controller`'s `bram_addr_o`.

The dual-clock BRAM provides inherent clock domain crossing between `pdh_clk` (write) and `fclk0` (read).

**Capture time** (at decimation = N): `N × 16384 / 125 MHz = N × 131 µs`

### Stage 2: AXI4 Burst Transfer (`dma_controller.sv`)

The DMA controller is a custom AXI4 master that sequences 16-beat INCR bursts from the BRAM to HP0 DDR:

| AXI Parameter  | Value                    |
|----------------|--------------------------|
| Burst type     | INCR (2'b01)             |
| Burst length   | 16 beats (AWLEN = 15)    |
| Beat size      | 8 bytes / 64 bits        |
| Write strobes  | 0xFF (all bytes valid)   |
| Base address   | 0x10000000               |
| Total size     | 0x20000 = 128 KB         |
| Total bursts   | 128 KB / 128 B = 1024    |

The FSM has four states:

1. **ST_IDLE**: resets address to base, asserts `dma_ready_o`. Transitions to `ST_SET_ADDR_AWAIT_ACK` on a rising edge of `enable_i` (3-stage synchronized from `pdh_clk`).
2. **ST_SET_ADDR_AWAIT_ACK**: drives `AWVALID` and holds the current burst address. Transitions to `ST_SET_DATA_AWAIT_ACK` when `AWREADY` is asserted.
3. **ST_SET_DATA_AWAIT_ACK**: drives `WVALID` and streams 16 beats, incrementing `beat_r` on each `WREADY`. Asserts `WLAST` on the final beat. Transitions to `ST_AWAIT_RESP` after the 16th beat is acknowledged.
4. **ST_AWAIT_RESP**: waits for `BVALID`. On a successful response (`BRESP == 2'b00`), increments the address by 128 bytes. If the final address is reached, returns to `ST_IDLE` (where `dma_ready_o` is reasserted); otherwise returns to `ST_SET_ADDR_AWAIT_ACK` for the next burst.

The BRAM read address is computed from the current burst address and beat counter:

```sv
bram_addr_o = ((addr_r - HP0_BASE_ADDR) >> 3) + beat_r
```

This sequentially reads BRAM locations 0–16383 as the DMA sweeps through DDR.

### Stage 3: DMA Orchestrator FSM (`pdh_core.sv`)

`pdh_core` coordinates BRAM capture and DMA transfer through a 4-state FSM in the `pdh_clk` domain:

```
DMA_ARMED ──[CMD_GET_FRAME received && bram_ready]──> DMA_RUN_BRAM
              (asserts bram_enable_o)
DMA_RUN_BRAM ──[bram capture done && dma_ready]──> DMA_RUN_AXI
               (asserts dma_enable_o)
DMA_RUN_AXI ──[dma_ready posedge (DMA complete)]──> DMA_STALE
DMA_STALE ──[CMD_GET_FRAME no longer active]──> DMA_ARMED
```

`dma_ready_i` (from `fclk0` domain) is synchronized through a 3-stage flip-flop chain (`dma_ready_1ff/2ff/3ff`) before being used in the `pdh_clk` domain. Edge detection on `dma_ready_3ff` generates the DMA-complete pulse.

The `bram_ready_i` signal (from `pdh_clk` domain `bram_controller`) is registered once and also edge-detected. The `bram_edge_acquired_r` flag ensures the transition to `DMA_RUN_AXI` waits for a bram-complete edge that occurred *after* the current capture started.

### Stage 4: Software Readback

After issuing `CMD_GET_FRAME`, the C server waits for the full DMA to complete using a timed sleep:

```c
#define DMA_BURST_CONST  330   // µs — accounts for AXI transfer overhead
#define BRAM_DEC_CONST   140   // µs per decimation unit (16384 / 125 MHz × 1.07)

usleep(DMA_BURST_CONST + BRAM_DEC_CONST * decimation_code);
```

Then it reads the DDR region sequentially through `gDmaMap` (a `mmap` of the 128 KB HP0 region at `0x10000000`) with a full memory barrier (`__sync_synchronize()`) to ensure coherency:

```c
uint64_t dma_get_frame(uint32_t byte_offset) {
    __sync_synchronize();
    return *((volatile uint64_t*)((uint8_t*)gDmaMap + byte_offset));
}
```

The 16384 64-bit words are formatted as CSV and written to `dma_log.csv`, which the Python client retrieves via SFTP.

### Frame Types

The `frame_code` field selects which internal signals are packed into each 64-bit DMA word. The CSV column order matches the `fprintf` order in `cmd_get_frame`:

| Frame Code          | CSV Columns (in order)                              | Description                         |
|---------------------|-----------------------------------------------------|-------------------------------------|
| `ANGLES_AND_ESIGS`  | adc_a, adc_b, i_feed, q_feed                       | Raw ADC (signed) and IQ demod       |
| `PID_ERR_TAPS`      | err, perr, derr, ierr                               | PID error, P, D, I contributions    |
| `IO_SUM_ERR`        | err, pid_out, sum_err                               | Error, DAC code, integrator state   |
| `OSC_INSPECT`       | nco_out1, nco_out2, nco_feed1, nco_feed2           | NCO signed outputs and DAC codes    |
| `OSC_ADDR_CHECK`    | phi1, phi2, addr1, addr2                            | Phase accumulators and ROM addresses|
| `LOOPBACK`          | dac1_feed, dac2_feed, adc_a, adc_b                  | DAC outputs vs. raw ADC (offset-binary) |

---

## Signal Representation and Fixed-Point Conventions

### ADC Conversion

The Red Pitaya ADC outputs 14-bit unsigned offset-binary samples (0 = −1 V, 8192 = 0 V, 16383 = +1 V). The FPGA converts each channel to a centered 16-bit signed integer:

```sv
adc_dat_a_16s_w = sign_extend(-(adc_dat_a_i - 8192))
```

The negation is required because the Red Pitaya ADC output is inverted relative to the input voltage. The resulting range is [−8192, +8192] ≈ [−1 V, +1 V] at 1/8192 V per LSB.

### Q15 Fixed-Point (Gains)

PID gains `kp`, `kd`, `ki` and rotation matrix coefficients use Q15 format: the integer value is a 16-bit two's-complement word representing a number in [−1, +1). To recover the float: `value / 32768.0`. Products of two Q15 numbers are right-shifted by 15 to renormalize.

### Q13 Fixed-Point (Setpoint)

The PID setpoint uses Q13 format: a 14-bit signed word representing [−1, +1). To recover the float: `value / 8192.0`. This is the same numerical scale as the 16-bit signed ADC representation (both have 1 LSB = 1/8192 V), so the error signal `dat_i − sp_r` is dimensionally consistent and the setpoint can be directly interpreted as a voltage target.

### DAC Output

The FPGA DAC output is 14-bit unsigned offset-binary (same convention as ADC input: 0 = −1 V, 8191 = 0 V, 16383 = +1 V). The PID output `sat_unsigned_from_signed(total + 8191)` and the NCO-to-DAC conversion `s16_to_u14` both produce values in this format.

---

## IO Routing

`cmd_config_io` configures three independent muxes:

### DAC Source Selection

Each DAC output can be sourced from one of four signals:

| Code | Name       | Source                       |
|------|------------|------------------------------|
| 0    | REGISTER   | Direct register (set_dac)    |
| 1    | PID        | PID controller output        |
| 2    | NCO_1      | NCO output 1 (via s16_to_u14)|
| 3    | NCO_2      | NCO output 2 (via s16_to_u14)|

DAC1 and DAC2 are selected independently via `dac1_dat_sel_r` and `dac2_dat_sel_r`.

### PID Input Selection

The PID error input `dat_i` can be sourced from:

| Code | Name     | Source                                        |
|------|----------|-----------------------------------------------|
| 0    | I_FEED   | I channel of IQ demodulation (rotated ADC A)  |
| 1    | Q_FEED   | Q channel of IQ demodulation (rotated ADC B)  |
| 2    | ADC_A    | Raw ADC channel A (adc_dat_a_16s)             |
| 3    | ADC_B    | Raw ADC channel B (adc_dat_b_16s)             |

For standard PDH locking, the PID is fed from `I_FEED` (the demodulated error signal). For direct DC locking or loopback tests, `ADC_A` or `ADC_B` provides the unprocessed voltage.

---

## Notable Design Choices

### Synchronized Reset Pipeline

Rather than using asynchronous resets throughout (which can cause glitches at power-on), `pdh_core`, `bram_controller`, and `dma_controller` all implement a two-stage synchronous reset pipeline:

```sv
always_ff @(posedge clk or posedge rst_i) begin
    if(rst_i) {rst_sync_r, rst_pipe1_r} <= 2'b11;
    else       {rst_sync_r, rst_pipe1_r} <= {rst_pipe1_r, 1'b0};
end
```

The reset is asserted asynchronously (so it takes effect immediately on any power event) but deasserted synchronously (so the downstream logic sees a clean, glitch-free release aligned to the clock edge). A separate negedge-domain reset pipeline handles the DAC output registers, which are clocked on the falling edge.

### Atomic IQ Rotation Matrix Commit

The IQ rotation matrix requires updating cos θ and sin θ simultaneously; writing them one at a time would cause a half-updated state where neither the old nor the new rotation is in effect. This is handled with a two-stage commit:

1. `CMD_SET_ROT_COEFFS` writes cos θ or sin θ to staging registers (`cos_theta_r`, `sin_theta_r`) selected by a bit in the data word.
2. `CMD_COMMIT_ROT_COEFFS` atomically copies both staging registers to the active registers (`rot_cos_theta_r`, `rot_sin_theta_r`) in a single clock cycle.

The active registers drive the IQ rotation computation, guaranteeing that the matrix is always internally consistent.

### Command-Callback Echo for Write Verification

Every FPGA register write is immediately followed by a readback of the FPGA's state via the callback register. Because the callback is driven combinationally from the FPGA's registered state, reading it one AXI transaction after the strobe confirms that the register was latched correctly. `validate_cb` compares each echoed field to the intended value and returns a non-zero status on mismatch, surfacing write failures to the Python client via the `status` field.

### Three-Stage AXI Synchronizer

The AXI GP0 bus operates in the PS clock domain while `pdh_core` runs at the ADC clock (125 MHz). A 3-stage synchronizer (rather than the minimum 2-stage) reduces the probability of metastability to approximately `(Tsetup/T_mtbf)^3`, providing adequate margin for a safety-critical control application.

### Quarter-Sine ROM

Storing only the first-quadrant sine values (0 to π/2) reduces the ROM size by 4× compared to a full-cycle table. The quadrant folding logic (`addr = 4095 − phi[11:0]` for Q2/Q4, sign inversion for Q3/Q4) reconstructs the full sinusoid with a single extra cycle of registered logic, adding only one pipeline stage.

### LOOPBACK Frame for Self-Test

The `LOOPBACK` frame type captures the DAC feed signals and raw ADC inputs simultaneously in the same 64-bit word:

```sv
LOOPBACK: dma_data_w = {2'b0, dac1_feed_w, 2'b0, dac2_feed_w, 2'b0, adc_dat_a_i, 2'b0, adc_dat_b_i};
```

When DAC1 is physically connected to ADC1 and DAC2 to ADC2, this frame makes it trivial to verify signal path integrity: any waveform programmed on the DAC should appear on the corresponding ADC column with the expected scaling. The `test.py` regression script uses this frame to provide a visual loopback verification plot.

### PID Integrator and Saturation Width Coupling

The parameter `satwidth` serves two purposes: it sets the integrator saturation bound (`±2^satwidth`) and the I-term right-shift (`>>> satwidth`). This coupling is intentional: as the saturation narrows (smaller `satwidth`), the integrator accumulates less history, and the I-term contribution is also scaled down proportionally, preventing a large clamped integrator from dominating the output when saturation is tight. The valid range 15–31 is enforced in the RTL (`next_satwidth_w = (satwidth_i between 15 and 31) ? satwidth_i : 31`).

