# Pound-Drever-Hall Laser Locking System

A hardware/software co-design implementation of a **Pound-Drever-Hall (PDH) laser frequency locking system** on a Red Pitaya STEMlab 125-14 (Xilinx Zynq XC7Z010 SoC). Real-time signal processing runs on the FPGA (PL), a TCP control server runs on the ARM cores (PS), and a Python client API provides remote operation from a host machine.

---

## Table of Contents

1. [Repository Structure](#repository-structure)
2. [Build and Deploy](#build-and-deploy)
3. [Controller GUI](#controller-gui)
4. [System Architecture](#system-architecture)
5. [Command Dispatch and Receive Architecture](#command-dispatch-and-receive-architecture)
6. [NCO Design](#nco-design)
7. [PID Controller Design](#pid-controller-design)
8. [FIR Filter Design](#fir-filter-design)
9. [IQ Demodulator Design](#iq-demodulator-design)
10. [DMA Live Capture Design](#dma-live-capture-design)
11. [Signal Representation and Fixed-Point Conventions](#signal-representation-and-fixed-point-conventions)
12. [IO Routing](#io-routing)
13. [Notable Design Choices](#notable-design-choices)
14. [Python Analysis Functions](#python-analysis-functions)

---

## Repository Structure

```
core/
  hw/
    prj/pdh_core/
      rtl/               FPGA RTL (SystemVerilog)
        pdh_top.sv       Board-level wrapper: clocking, PS instantiation, HP0 AXI tie-offs
        pdh_core.sv      Main control module: command decoder, IQ rotation, IQ demod, DMA orchestrator
        nco.sv           Numerically controlled oscillator (dual-output, quarter-ROM)
        iq_demod.sv      Instantaneous-product demodulator (configurable ref × input, Q15)
        pid_core.sv      Discrete PID with EMA derivative, anti-windup, decimation
        bram_controller.sv  Dual-clock BRAM capture buffer (pdh_clk write / fclk0 read)
        dma_controller.sv   AXI4 master for HP0 DDR burst writes
        sine_qtr_rom.sv  4096-entry, 16-bit quarter-sine ROM
        posedge_detector.sv  Rising-edge detector utility
      tb/                Testbenches (SystemVerilog, Icarus)
  sim/                   All RTL simulations (cocotb)
    run_regression.py      Runner for the command-echo regression test
    run_bode.py            Open-loop Bode analysis simulation
    run_fir_freq.py        FIR frequency-response verification simulation
    utils/
      test_pdh_core.py     cocotb test module — command-echo checks for every RTL register
      test_bode.py         cocotb test module — open-loop Bode measurement
      test_fir_freq_response.py  cocotb test module — FIR frequency response vs ideal
    Makefile.cocotb        cocotb regression make rules
    Makefile.bode          Bode sim make rules
    Makefile.fir_freq      FIR freq-response make rules
    artifacts/             Auto-indexed simulation output (JSON, CSV, PNG)
  sw/
    server.c             TCP server: two-thread interrupt-driven architecture (command thread + callback thread)
    control/
      control.c          Command handler implementations (cmd_*_send / cmd_*_cb split functions)
      hw_common.c        Hardware abstraction: mmap of AXI GP0 and HP0 DMA region; UIO interrupt primitives
      inc/
        server.h         cmd_ctx_t, cmd_entry_t, output_item_t definitions
        hw_common.h      pdh_cmd_t, pdh_callback_t, dma_frame_t packed unions; all enums
        control.h        Command handler declarations
  dts/
    pdh_irq.dts          Device Tree overlay — binds /dev/uio/pdh_uio to IRQ_F2P[1] (GIC SPI 62)
client/
  gui.py                 Interactive tkinter GUI — full hardware control without scripting
  pdh_api/
    api.py               Python API functions wrapping each TCP command
    types.py             Enums and result dataclasses mirroring the C/RTL types
    fir_design.py        Windowed-sinc FIR lowpass design (shared by GUI and sim tooling)
  test.py                Board-level regression test and visualization script
sim/
  pdh.m                  MATLAB PDH signal simulation
  power_terms.m          MATLAB power series analysis
pcb/                     PCB design files
```

---

## Build and Deploy

To build the fpga image:

```bash
cd core/hw && make pdh_core
```
Note that Vivado must be installed and appropriately configured for this to work.



To deploy the system, the fpga image must be loaded, the device tree overlay must be mounted, and the server executable must be running. Locate the IP mapping to your Red Pitaya's MAC address via your client's ARP table or any other method. SCP the following files onto it:


1. `core/hw/build/boot.bin`
1. `core/sw/`
1. `core/mount_dts.sh`

Then ssh into the server and do the following:

```bash
fpgautil -b boot.bin
./mount_dts.sh #may need to call chmod +x here
cd sw && make server && cd build && ./server
```

Then from the project root:
```bash
pip install -r requirements.txt
cd client
python gui.py
```


### RTL Simulation (cocotb)

All simulations live in `core/hw/sim/`. Each is driven by a Python runner script that invokes cocotb/Verilator (or Icarus) via make, writes artifacts to a numbered subdirectory of `sim/artifacts/`, and exits non-zero on any test failure.

Requires `cocotb`, and either Verilator or Icarus Verilog (`iverilog`).

#### Command-Echo Regression (`run_regression.py`)

Exercises every command that has an observable callback echo and checks that the FPGA register round-trips match what was sent. DMA/frame capture is out of scope — inputs are tied to benign static values.

Checks covered (98 in total):
- **Reset**: `led_o` and `rst_o` cleared
- **Set LED**: `led_o` pin value and callback echo
- **Config IO**: DAC1/DAC2 source select and PID input select echoes
- **Set DAC**: both DAC1 and DAC2 code echoes (with server-side negation)
- **Get ADC**: ADC_A and ADC_B raw code echoes
- **Set NCO**: stride, shift, inv, sub, enable echoes; frequency quantization check
- **Set Rotation**: cos θ and sin θ coefficient echoes (within ±0.01 of float, limited by 14-bit CB truncation)
- **Set PID**: kp, kd, ki, dec, sp, alpha, sat, en, gain, bias, egain — all 11 coefficients
- **Set FIR**: address, coefficient, input select, write-enable, chain-write-enable echoes
- **Check Signed**: ADC_A, ADC_B, I feed, Q feed, IO routing register reads
- **Interrupt**: `irq_o` fires within expected cycles after strobe for a non-`CMD_GET_FRAME` command
- **Config Demod**: ref_sel, in_sel, and alpha echo checks (including 2 alpha echo checks)
- **IQ Demod functional**: in-phase product (NCO1×NCO1 → DC mean > 5000 counts) and quadrature product (NCO1×NCO2 → mean ≈ 0, std > 100 counts) checks via `dma_data_o`; demod_lpf DC check in `CAPTURE_DEMOD` test
- **EMA LPF routing**: 3 checks in section 18 verifying `demod_lpf` as a routing source for FIR and PID inputs

```bash
cd core/hw/sim
python run_regression.py                   # Verilator (default)
python run_regression.py --sim icarus
```

#### Open-Loop Bode Analysis (`run_bode.py`)

Programs a complete PID + FIR configuration into the RTL and measures the open-loop frequency response by injecting a swept sine at the error input and recording the controller output. Produces a Bode plot (magnitude + phase) comparing the RTL against an ideal analytical model.

```bash
cd core/hw/sim
python run_bode.py                              # defaults: Kp=0.5, Ki=0.0, Kd=0.0
python run_bode.py --kp 0.8 --ki 0.1 --kd 0.0
python run_bode.py --alpha 3 --satwidth 28
python run_bode.py --delay 6.2                  # 6.2 ns DAC→ADC line delay
python run_bode.py --dec 1,10,100,1000          # sweep multiple decimation codes
python run_bode.py --no-sim                     # replot most recent result
python run_bode.py --csv path/to/fir_coeffs.csv --sim icarus
```

Key arguments:

| Flag | Default | Description |
|------|---------|-------------|
| `--kp/ki/kd` | 0.5/0.0/0.0 | PID gains |
| `--alpha` | 2 | EMA derivative exponent |
| `--satwidth` | 31 | Integrator saturation width |
| `--delay NS` | 2500.0 | Physical DAC→ADC round-trip delay (ns) |
| `--dec` | 1,2,5,…,1000 | Comma-separated decimation codes to test |
| `--plot-dec` | median | Which decimation to display in Bode panels |
| `--csv PATH` | `test_resources/fir_coeffs_example.csv` | FIR coefficient CSV |
| `--no-sim` | — | Skip simulation; replot existing JSON |

Artifacts written to `sim/artifacts/bode/bode_N/`: `bode_results.json`, `bode_plot.png`.

#### FIR Frequency-Response Verification (`run_fir_freq.py`)

Designs a windowed-sinc lowpass filter, writes coefficients to a CSV, loads them into the RTL via cocotb, sweeps a sine through it, and overlays the measured RTL response against the ideal `sinc × window` response.

```bash
cd core/hw/sim
python run_fir_freq.py                              # 32 taps, 5 MHz corner, Hann
python run_fir_freq.py --ntaps 64 --corner 2e6
python run_fir_freq.py --corner 10e6 --window hamming
python run_fir_freq.py --no-sim                     # replot most recent result
```

Artifacts written to `sim/artifacts/fir_freq/fir_freq_N/`: `fir_coeffs.csv`, `fir_freq_results.json`, `fir_rtl_response.csv`, `fir_freq_response.png`.


## System Architecture

```
Host PC (Python)
    |
    | TCP port 5555 (text protocol)
    v
ARM Cortex-A9 (Linux)  [two pthreads]
    Thread 1 (command)   ── accept(), parse, cmd_*_send(), push pending queue
    Thread 2 (callback)  ── blocks on /dev/uio/pdh_uio read(); on interrupt:
                              reads GP0 callback, runs cmd_*_cb(), sends TCP response
    hw_common.c          ── mmap of AXI GP0 (commands+callbacks) and HP0 DDR region
    |
    | AXI GP0 (0x42000000)  32-bit command/callback
    | AXI HP0 (0x10000000)  64-bit DMA write port
    | IRQ_F2P[1] / GIC SPI 62  EDGE_RISING interrupt → /dev/uio/pdh_uio
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

## NCO

The numerically controlled oscillator generates two phase-coherent sinusoidal outputs for use as local oscillators in IQ demodulation and as DAC drive signals.

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
| gain      | Q10    | [−32, +32)   | output gain multiplier; int16 / 1024.0  |
| bias      | Q13    | [−1, +1)     | DC offset added to output; int14 / 8192.0 |
| egain     | Q10    | [−32, +32)   | input (error) gain multiplier; int16 / 1024.0 |

---

## FIR Filter Design

A programmable FIR lowpass filter sits between the IQ demodulation stage and the PID controller. It is implemented across two RTL modules (`fir.sv` / `fir_tap.sv`) and designed in software via `client/pdh_api/fir_design.py`.

### Signal Placement

```
ADC A/B          ──┐
I Feed           ──┤
Q Feed           ──┼─► [FIR input mux] ──► fir.sv ──► FIR Out ──► [PID input mux] ──► pid_core
IQ Demod Out     ──┤
IQ Demod LPF Out ──┘
```

The FIR input source and whether the PID consumes the FIR output are both controlled independently via `config_io`. The filter can be programmed and its input/output routed at any time without affecting other subsystems.

`client/pdh_api/fir_design.py` implements a windowed-sinc lowpass design:

```
h[n] = sinc(ωc × (n − nc) / π)  ×  w[n]
```

where `ωc = 2π × f_corner / fs` is the normalized cutoff and `nc = (NTAPS − 1) / 2` is the centre tap. The window `w[n]` trades passband ripple against stopband attenuation:

| Window | Passband ripple | Stopband attenuation | Transition width |
|--------|----------------|----------------------|-----------------|
| Rectangular | ±0.9 dB | ~21 dB | Narrowest |
| Hann | ±0.06 dB | ~44 dB | Moderate |
| Hamming | ±0.02 dB | ~41 dB | Moderate |
| Blackman | ±0.002 dB | ~74 dB | Widest |

`design_lowpass` raises `ValueError` if `max|coeff| ≥ 1.0` — the Q15 range would be exceeded. This happens at very high corner frequencies (approaching `fs/2`) where the ideal sinc kernel values approach 1. The fix is to lower the corner frequency or choose a windowed design; for standard PDH demodulation bandwidths (< 10 MHz) it does not arise.

`f_corner` is the half-amplitude (−6 dB) point of the ideal sinc kernel, not the −3 dB point. The actual −3 dB frequency shifts with window choice.

### Frequency-Response Simulation

`core/hw/sim/run_fir_freq.py` designs a filter, writes the coefficients to a CSV, runs the cocotb/Verilator RTL simulation, and overlays the ideal frequency response against the measured RTL response:

```bash
cd core/hw/sim
python run_fir_freq.py                              # 32 taps, 5 MHz corner, Hann
python run_fir_freq.py --ntaps 64 --corner 2e6
python run_fir_freq.py --corner 10e6 --window hamming
python run_fir_freq.py --no-sim                     # replot most recent result
```

Artifacts (coefficient CSV, results JSON, response PNG) are written to `sim/artifacts/fir_freq/fir_freq_N/` with an auto-incrementing index so successive runs never overwrite each other.

![FIR ideal vs RTL](figures/fir_ideal_vs_rtl.png)

*Ideal DTFT response overlaid with the RTL-measured response (32-tap Hann, 5 MHz corner), confirming negligible deviation from quantization and pipeline rounding across the passband.*

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

| Code | Name             | Source                                        |
|------|------------------|-----------------------------------------------|
| 0    | I_FEED           | I channel of IQ demodulation (rotated ADC A)  |
| 1    | Q_FEED           | Q channel of IQ demodulation (rotated ADC B)  |
| 2    | ADC_A            | Raw ADC channel A (adc_dat_a_16s)             |
| 3    | ADC_B            | Raw ADC channel B (adc_dat_b_16s)             |
| 4    | FIR_OUT          | FIR filter output                             |
| 5    | IQ_DEMOD_OUT     | IQ demodulator product output                 |
| 6    | IQ_DEMOD_LPF     | EMA low-pass filtered demodulator output      |

For standard PDH locking, the PID is fed from `I_FEED` (the demodulated error signal). For direct DC locking or loopback tests, `ADC_A` or `ADC_B` provides the unprocessed voltage. `FIR_OUT` inserts the programmable FIR lowpass between the demodulation stage and the PID, which is the normal operating mode for bandwidth-limited locking. `IQ_DEMOD_OUT` routes the instantaneous-product demodulator output directly to the PID, bypassing the rotation matrix. `IQ_DEMOD_LPF` routes the EMA-filtered demodulator output, providing an alternative integrated lowpass path.

### FIR Input Selection

The FIR filter input can be sourced from:

| Code | Name             | Source                                        |
|------|------------------|-----------------------------------------------|
| 0    | ADC1             | Raw ADC channel A (adc_dat_a_16s)             |
| 1    | ADC2             | Raw ADC channel B (adc_dat_b_16s)             |
| 2    | I_FEED           | I channel of IQ demodulation (rotated ADC A)  |
| 3    | Q_FEED           | Q channel of IQ demodulation (rotated ADC B)  |
| 4    | IQ_DEMOD_OUT     | IQ demodulator product output                 |
| 5    | IQ_DEMOD_LPF     | EMA low-pass filtered demodulator output      |

---

## Python Analysis Functions

The `client/pdh_api/api.py` module provides three higher-level analysis functions that sit above the raw command API.

### `api_psd(decimation, frame_code)` → `PSDResult`

Captures a DMA frame and computes a one-sided power spectral density for every channel in the frame using the Wiener–Khinchin theorem (FFT of the autocorrelation function).

```python
from pdh_api import api, FrameCode

r = api.api_psd(decimation=10, frame_code=FrameCode.ADC_DATA_IN)
# r.freqs     — frequency bins in Hz, shape (N_freq,)
# r.psd       — PSD in counts²/Hz, shape (N_freq, N_cols)
# r.fs        — effective sample rate: 125e6 / decimation
# r.columns   — channel names per FRAME_COLUMNS[frame_code]
# r.raw_data  — raw DMA data used for the PSD, shape (16384, N_cols)
```

Effective sample rate: `125e6 / decimation` Hz. Nyquist: `62.5e6 / decimation` Hz. Frequency resolution: `fs / (2N − 1)` where N = 16384.

The GUI exposes this via the **PSD** tab in the Frame Capture panel.

---

### `api_control_metrics(decimation, pid_params)` → `ControlMetricsResult`

Captures a `PID_IO` frame (columns: `pid_in`, `err`, `pid_out`) and computes a comprehensive set of controller performance metrics:

| Metric | Description |
|--------|-------------|
| PSD | One-sided PSD for pid_in, err, pid_out |
| RMS error | `err` RMS across the capture window |
| Peak error | Maximum `|err|` in the capture window |
| Settling time | Time (in samples) for `|err|` to fall and stay below 5% of its initial value |
| Overshoot | Peak `err` above steady state in the first 20% of the capture, as a percentage of the transient range |
| CCF peak | Cross-correlation peak between `pid_out` and `err` — negative value near −1 indicates well-regulated negative feedback; lag gives loop delay |
| CCF lag | Lag at CCF peak in samples and seconds |

```python
pid_params = {"kp": 0.5, "ki": 0.1, "kd": 0.0, "dec": 10, "sp": 0.0}
r = api.api_control_metrics(decimation=10, pid_params=pid_params)
# r.raw_data       — shape (16384, 3): pid_in, err, pid_out
# r.fs             — 125e6 / decimation
# r.freqs / r.psd  — PSD result
# r.rms_err        — float
# r.peak_err       — float
# r.settling_time  — float (seconds), or None if never settled
# r.overshoot      — float
# r.ccf_peak       — float
# r.ccf_peak_lag   — float (samples)
# r.pid_params     — dict passed in (for labelling plots)
```

`pid_params` is not sent to hardware — it is embedded in the result for plot labelling only. The FPGA must already be running with the desired PID configuration before calling this function.

The GUI exposes this via the **Compute Control Metrics** button in the PID tab.

---

### `compute_lockpoint(data, sign_sel, invert_delta, window)` → `LockPointResult`

Post-processes a sweep-ramp capture to determine the optimal IQ rotation angle and PDH lock point.

```python
# Standard sweep (demod_mode=0): captures dac_v, adc_a, adc_b, i_feed, q_feed
sweep = api.api_sweep_ramp(v0=-1.0, v1=1.0, num_points=500,
                            dac_sel=DacSel.DAC_1, demod_mode=0)

# Internal demodulation sweep (demod_mode=1): captures dac_v, demod_lpf
sweep = api.api_sweep_ramp(v0=-1.0, v1=1.0, num_points=500,
                            dac_sel=DacSel.DAC_1, demod_mode=1)

result = api.compute_lockpoint(sweep.data, sign_sel="I", window=10)
# result.G                  — "golden" demodulated signal in volts, shape (N,)
# result.optimal_angle_deg  — IQ rotation angle that maximises the PDH slope
# result.lock_point         — DAC voltage at the steepest zero crossing
```

**Algorithm**: For each candidate rotation angle θ in [−180°, +180°], the I and Q channels are rotated and the resulting signal's maximum slope magnitude is measured via a sliding window of `window` points. The angle that maximises this slope is `optimal_angle_deg`. The `lock_point` is the DAC voltage at the steepest zero crossing of `G = cos(θ)·I + sin(θ)·Q`.

`sign_sel` (default `"I"`) selects which rotated channel to use for the golden signal. `invert_delta` flips the sign of the slope criterion, useful when the PDH dispersion signal has the opposite polarity.

The GUI exposes this via the **Compute Lock Point** button in the Sweep Ramp tab.

