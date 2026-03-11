"""
PDH system type definitions: enums mirroring hw_common.h and result dataclasses.
"""
from __future__ import annotations

from dataclasses import dataclass
from enum import IntEnum
import numpy as np


# ── Enums (mirror hw_common.h / control code) ─────────────────────────────────

class FrameCode(IntEnum):
    ADC_DATA_IN = 0b0000
    PID_ERR_TAPS     = 0b0001
    IO_SUM_ERR       = 0b0010
    OSC_INSPECT      = 0b0011
    OSC_ADDR_CHECK   = 0b0100
    LOOPBACK         = 0b0101
    FIR_IO           = 0b0110
    PID_IO           = 0b0111


class FirInputSel(IntEnum):
    """FIR filter input source selection (mirrors fir_input_sel_t in RTL)."""
    ADC1   = 0b000
    ADC2   = 0b001
    I_FEED = 0b010
    Q_FEED = 0b011


class DacSel(IntEnum):
    DAC_1 = 0
    DAC_2 = 1


class DacDatSel(IntEnum):
    """Source selection for each DAC output (config_io dac*_dat_sel)."""
    REGISTER = 0b000   # manual register value
    PID      = 0b001   # PID controller output
    NCO_1    = 0b010   # NCO 1 output
    NCO_2    = 0b011   # NCO 2 output


class PidDatSel(IntEnum):
    """Error input source for the PID controller (config_io pid_dat_sel)."""
    I_FEED  = 0b000
    Q_FEED  = 0b001
    ADC_A   = 0b010
    ADC_B   = 0b011
    FIR_OUT = 0b100


class CsSel(IntEnum):
    """Register select codes for cmd_check_signed."""
    ADC_A   = 0b00000
    ADC_B   = 0b00001
    I_FEED  = 0b00110
    Q_FEED  = 0b00111
    IO      = 0b10000


# ── Frame column metadata ──────────────────────────────────────────────────────

# Column labels for each frame type, in CSV column order (matches C fprintf order).
FRAME_COLUMNS: dict[FrameCode, list[str]] = {
    FrameCode.ADC_DATA_IN: ["adc_a",    "adc_b",     "i_feed",    "q_feed"],
    FrameCode.PID_ERR_TAPS:    ["err",       "perr",      "derr",      "ierr"],
    FrameCode.IO_SUM_ERR:      ["err",       "pid_out",   "sum_err"],           # 3 cols
    FrameCode.OSC_INSPECT:     ["nco_out1",  "nco_out2",  "nco_feed1", "nco_feed2"],
    FrameCode.OSC_ADDR_CHECK:  ["phi1",      "phi2",      "addr1",     "addr2"],
    FrameCode.LOOPBACK:        ["dac1_feed", "dac2_feed", "adc_a",     "adc_b"],
    FrameCode.FIR_IO:          ["fir_in",    "fir_out"],
    FrameCode.PID_IO:          ["pid_in",    "err",       "pid_out"],
}


# ── Result dataclasses ─────────────────────────────────────────────────────────

@dataclass
class ResetResult:
    status: int
    rst_raw_cb: int


@dataclass
class SetLedResult:
    status: int
    led_code_cb: int
    cmd_cb: int


@dataclass
class AdcResult:
    status: int
    in1_code: int
    in2_code: int
    in1_v: float
    in2_v: float


@dataclass
class SetDacResult:
    status: int
    dac1_code_cb: int
    dac2_code_cb: int


@dataclass
class CheckSignedResult:
    status: int
    reg_sel_cb: int
    reg_value_cb: int


@dataclass
class SetRotResult:
    status: int
    cos_cb: float
    sin_cb: float
    i_feed_cb: float
    q_feed_cb: float


@dataclass
class SetPidResult:
    status: int
    kp_cb: float
    kd_cb: float
    ki_cb: float
    dec_cb: int
    sp_cb: float
    alpha_cb: int
    sat_cb: int
    en_cb: int
    gain_cb: float      # echoed gain in Q10 float (gain_i / 1024.0)
    bias_cb: float      # echoed bias in volts (bias_i / 8191.0)


@dataclass
class SetNcoResult:
    status: int
    stride_cb: int
    shift_cb: int
    sub_cb: int
    inv_cb: int
    en_cb: int
    registered_freq: float
    registered_freq_error: float
    registered_phase_shift: float
    registered_shift_error: float


@dataclass
class ConfigIoResult:
    status: int
    dac1_dat_sel_cb: int
    dac2_dat_sel_cb: int
    pid_dat_sel_cb: int


@dataclass
class FrameResult:
    status: int
    dma_engaged: int
    decimation_cb: int
    frame_code_cb: int
    data: np.ndarray
    columns: list[str]


SWEEP_COLUMNS = ["dac_v", "adc_a", "adc_b", "i_feed", "q_feed"]


@dataclass
class SweepRampResult:
    status:        int
    num_points_cb: int
    data:          np.ndarray   # shape (num_points, 5): dac_v, adc_a, adc_b, i_feed, q_feed
    columns:       list[str]    # always SWEEP_COLUMNS


@dataclass
class LockPointResult:
    G:                 np.ndarray   # shape (num_points,), golden signal in volts
    optimal_angle_deg: float        # degrees, for api_set_rotation
    lock_point:        float        # recommended DAC voltage (zero crossing)


@dataclass
class SetFirResult:
    status: int
    input_sel_cb: int
    mem_wen_en_cb: int
    mem_wen_dis_cb: int
    chain_wen_cb: int


@dataclass
class PSDResult:
    status:   int
    freqs:    np.ndarray   # shape (N_freq,), Hz
    psd:      np.ndarray   # shape (N_freq, N_cols): one column per channel in `columns`
    fs:       float        # effective sample rate: 125e6 / decimation
    columns:  list[str]    # channel names matching FRAME_COLUMNS[frame_code]
    raw_data: np.ndarray   # shape (N_samples, N_cols): raw DMA frame used for PSD


@dataclass
class ControlMetricsResult:
    # Raw DMA capture (N × 3: pid_in, err, pid_out)
    raw_data:      np.ndarray
    columns:       list[str]    # ["pid_in", "err", "pid_out"]
    fs:            float        # 125e6 / decimation
    decimation:    int

    # PSD computed from the capture (all 3 channels)
    freqs:         np.ndarray   # shape (N_freq,), Hz
    psd:           np.ndarray   # shape (N_freq, 3)

    # Metrics (all in raw ADC counts unless noted)
    rms_error:     float        # RMS of err column
    settling_time: float        # seconds; nan if not settled within capture window
    overshoot:     float        # % — (peak − steady) / peak-to-peak range × 100
    ctrl_rms:      float        # RMS of pid_out
    ctrl_max:      float        # max |pid_out|
    ctrl_p95:      float        # 95th-percentile |pid_out|
    ctrl_slew_rms: float        # RMS of np.diff(pid_out)

    # Controller parameters read from GUI fields at time of capture
    pid_params:    dict
