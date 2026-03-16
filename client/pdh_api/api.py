"""
PDH Python client API.

Each api_* function sends one command to the C server, parses the response,
and returns a typed result dataclass.  The @api_cmd decorator enforces
status == 0, raising RuntimeError on any non-zero return code.

CSV files (dma_log.csv, sweep_log.csv) are fetched from the Red Pitaya via
SFTP and saved locally under LOCAL_DATA_DIR.  Plotting is left to the caller.
"""
from __future__ import annotations

import math
import os
import socket
from functools import wraps

import numpy as np
import paramiko

from .types import (
    CsSel, ConfigDemodResult, ConfigIoResult, ControlMetricsResult,
    DacDatSel, DacSel, DemodInSel, DemodRefSel, FRAME_COLUMNS,
    FirInputSel, FrameCode, FrameResult, LockPointResult, PidDatSel, PSDResult,
    SWEEP_COLUMNS, SWEEP_DEMOD_COLUMNS,
    AdcResult, CheckSignedResult, ResetResult, SetDacResult,
    SetFirResult, SetLedResult, SetNcoResult, SetPidResult, SetRotResult,
    SweepRampResult,
)


# ── Hardware constants ─────────────────────────────────────────────────────────

FPGA_FS = 125e6     # base FPGA sample rate (Hz)


# ── Connection parameters ──────────────────────────────────────────────────────

SERVER_IP   = "10.42.0.62"
SERVER_PORT = 5555
SSH_USER    = "root"
SSH_PASS    = "root"

LOCAL_DATA_DIR   = "data"
REMOTE_DMA_CSV   = "sw/build/dma_log.csv"
REMOTE_SWEEP_CSV = "sw/build/sweep_log.csv"


# ── Public API surface ─────────────────────────────────────────────────────────

__all__ = [
    # constants
    "FPGA_FS",
    # enums / types re-exported for one-stop import
    "FrameCode", "DacSel", "DacDatSel", "PidDatSel", "CsSel", "FirInputSel",
    "DemodRefSel", "DemodInSel",
    "SWEEP_COLUMNS", "SWEEP_DEMOD_COLUMNS",
    # result dataclasses
    "ResetResult", "SetLedResult", "AdcResult", "SetDacResult",
    "CheckSignedResult", "SetRotResult", "SetPidResult",
    "SetNcoResult", "SetFirResult", "ConfigIoResult", "ConfigDemodResult",
    "FrameResult", "SweepRampResult",
    "LockPointResult", "PSDResult", "ControlMetricsResult",
    # API functions
    "api_reset_fpga", "api_set_led", "api_get_adc", "api_set_dac",
    "api_check_signed", "api_set_rotation", "api_set_nco",
    "api_set_pid", "api_set_fir", "api_set_fir_coeffs", "api_config_io",
    "api_config_demod",
    "api_get_frame", "api_sweep_ramp", "compute_lockpoint", "api_psd",
    "api_control_metrics",
]


# ── Transport ──────────────────────────────────────────────────────────────────

def _parse_response(text: str) -> dict:
    """Parse 'key:value\\n...' server response into a dict, coercing numerics."""
    result: dict = {}
    for line in text.strip().splitlines():
        if ":" not in line:
            continue
        key, _, val = line.partition(":")
        key = key.strip()
        val = val.strip()
        try:
            result[key] = int(val)
        except ValueError:
            try:
                result[key] = float(val)
            except ValueError:
                result[key] = val
    return result


CMD_TIMEOUT_S = 5.0   # seconds to wait for a server response before failing

def execute_cmd(cmd: str) -> dict:
    """Send a single command string to the server; return the parsed response."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(CMD_TIMEOUT_S)
        s.connect((SERVER_IP, SERVER_PORT))
        s.sendall(cmd.encode("ascii"))
        raw = s.recv(4096)
    return _parse_response(raw.decode("ascii", errors="replace"))


def _ssh_fetch_csv(remote_path: str, local_name: str) -> str:
    """Fetch a file from the Red Pitaya via SFTP; return the local path."""
    os.makedirs(LOCAL_DATA_DIR, exist_ok=True)
    local_path = os.path.join(LOCAL_DATA_DIR, local_name)
    ssh = paramiko.SSHClient()
    ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    ssh.connect(SERVER_IP, username=SSH_USER, password=SSH_PASS)
    try:
        sftp = ssh.open_sftp()
        sftp.get(remote_path, local_path)
        sftp.close()
    finally:
        ssh.close()
    return local_path


# ── Decorator ──────────────────────────────────────────────────────────────────

def api_cmd(fn):
    """Enforce status == 0 on the returned result dataclass.  Raises on failure."""
    @wraps(fn)
    def wrapper(*args, **kwargs):
        result = fn(*args, **kwargs)
        status = getattr(result, "status", None)
        if status is None:
            raise RuntimeError(f"{fn.__name__}: result has no status field")
        if status != 0:
            raise RuntimeError(f"{fn.__name__}: server returned status={status}")
        return result
    return wrapper


# ── API commands ───────────────────────────────────────────────────────────────

@api_cmd
def api_reset_fpga() -> ResetResult:
    r = execute_cmd("CMD:reset_fpga\n")
    return ResetResult(
        status=r.get("status", -1),
        rst_raw_cb=r.get("RST_RAW_CB", 0),
    )


@api_cmd
def api_set_led(value: int) -> SetLedResult:
    r = execute_cmd(f"CMD:set_led\nU:{value}\n")
    return SetLedResult(
        status=r.get("status", -1),
        led_code_cb=r.get("LED_CODE_CB", 0),
        cmd_cb=r.get("CMD_CB", 0),
    )


@api_cmd
def api_get_adc() -> AdcResult:
    r = execute_cmd("CMD:get_adc\n")
    return AdcResult(
        status=r.get("status", -1),
        in1_code=r.get("IN1", 0),
        in2_code=r.get("IN2", 0),
        in1_v=r.get("IN1_V", float("nan")),
        in2_v=r.get("IN2_V", float("nan")),
    )


@api_cmd
def api_set_dac(value: float, dac_sel: DacSel | int) -> SetDacResult:
    r = execute_cmd(f"CMD:set_dac\nF:{value}\nU:{int(dac_sel)}\n")
    return SetDacResult(
        status=r.get("status", -1),
        dac1_code_cb=r.get("DAC1_CODE_CB", 0),
        dac2_code_cb=r.get("DAC2_CODE_CB", 0),
    )


@api_cmd
def api_check_signed(reg_sel: CsSel | int) -> CheckSignedResult:
    r = execute_cmd(f"CMD:check_signed\nU:{int(reg_sel)}\n")
    return CheckSignedResult(
        status=r.get("status", -1),
        reg_sel_cb=r.get("REG_SEL_CB", 0),
        reg_value_cb=r.get("REG_VALUE_CB", 0),
    )


@api_cmd
def api_set_rotation(theta_deg: float) -> SetRotResult:
    """Set the IQ rotation matrix.  theta_deg in degrees."""
    theta_rad = math.pi * theta_deg / 180.0
    r = execute_cmd(f"CMD:set_rotation\nF:{theta_rad}\n")
    return SetRotResult(
        status=r.get("status", -1),
        cos_cb=r.get("COS_CB", float("nan")),
        sin_cb=r.get("SIN_CB", float("nan")),
        i_feed_cb=r.get("I_FEED_CB", float("nan")),
        q_feed_cb=r.get("Q_FEED_CB", float("nan")),
    )


@api_cmd
def api_set_nco(freq: float, shift_deg: float, en: int) -> SetNcoResult:
    """Configure one NCO.  freq in Hz, shift_deg in degrees [-180, 180]."""
    r = execute_cmd(f"CMD:set_nco\nF:{freq},{shift_deg}\nU:{en}\n")
    return SetNcoResult(
        status=r.get("status", -1),
        stride_cb=r.get("STRIDE_CB", 0),
        shift_cb=r.get("SHIFT_CB", 0),
        sub_cb=r.get("SUB_CB", 0),
        inv_cb=r.get("INV_CB", 0),
        en_cb=r.get("EN_CB", 0),
        registered_freq=r.get("REGISTERED_FREQ", float("nan")),
        registered_freq_error=r.get("REGISTERED_FREQ_ERROR", float("nan")),
        registered_phase_shift=r.get("REGISTERED_PHASE_SHIFT", float("nan")),
        registered_shift_error=r.get("REGISTERED_SHIFT_ERROR", float("nan")),
    )


@api_cmd
def api_set_pid(
    kp: float, kd: float, ki: float, sp: float,
    dec: int, alpha: int, sat: int, en: int,
    gain: float = 1.0,
    bias: float = 0.0,
    egain: float = 1.0,
) -> SetPidResult:
    """
    Configure the PID controller.

    kp, kd, ki: gains in Q15 range [-1, 1).
    sp:         setpoint in Q13 range [-1, 1).
    dec:        decimation (1-16383).
    alpha:      IIR filter exponent (0-15).
    sat:        output saturation bits (0-31).
    en:         1 to enable, 0 to disable.
    gain:       output scalar in Q10 range [-32, 32).
    bias:       DC offset added to PID output, in volts [-1.0, 1.0].
    egain:      input gain applied to error before Kp/Ki/Kd, Q10 range [-32, 32).
    """
    r = execute_cmd(
        f"CMD:set_pid\nF:{kp},{kd},{ki},{sp},{gain},{bias},{egain}\nU:{dec},{alpha},{sat},{en}\n"
    )
    return SetPidResult(
        status=r.get("status", -1),
        kp_cb=r.get("KP_CB", float("nan")),
        kd_cb=r.get("KD_CB", float("nan")),
        ki_cb=r.get("KI_CB", float("nan")),
        dec_cb=r.get("DEC_CB", 0),
        sp_cb=r.get("SP_CB", float("nan")),
        alpha_cb=r.get("ALPHA_CB", 0),
        sat_cb=r.get("SAT_CB", 0),
        en_cb=r.get("EN_CB", 0),
        gain_cb=r.get("GAIN_CB", float("nan")),
        bias_cb=r.get("BIAS_CB", float("nan")),
        egain_cb=r.get("EGAIN_CB", float("nan")),
    )


def _send_fir_coeffs(coeffs: list[float], input_sel: FirInputSel | int) -> SetFirResult:
    """Internal: send an in-memory coefficient list to the FPGA FIR block."""
    coeff_str = ",".join(str(c) for c in coeffs)
    r = execute_cmd(
        f"CMD:set_fir\n"
        f"F:{coeff_str}\n"
        f"U:{int(input_sel)}\n"
    )
    return SetFirResult(
        status=r.get("status", -1),
        input_sel_cb=r.get("INPUT_SEL_CB", 0),
        mem_wen_en_cb=r.get("MEM_WEN_EN_CB", 0),
        mem_wen_dis_cb=r.get("MEM_WEN_DIS_CB", 0),
        chain_wen_cb=r.get("CHAIN_WEN_CB", 0),
    )


@api_cmd
def api_set_fir(csv_path: str, input_sel: FirInputSel | int) -> SetFirResult:
    """
    Load FIR tap coefficients from a CSV file and configure the FPGA FIR filter.

    csv_path:  Path to a CSV file with one Q15 float coefficient per line
               (e.g. generated by gen_fir_coeffs.py).  Values must be in [-1, 1).
    input_sel: Signal routed to the FIR input (FirInputSel enum or raw int 0-3).
    """
    coeffs: list[float] = []
    with open(csv_path) as f:
        for line in f:
            line = line.strip()
            if line:
                coeffs.append(float(line))
    return _send_fir_coeffs(coeffs, input_sel)


@api_cmd
def api_set_fir_coeffs(coeffs: list[float], input_sel: FirInputSel | int) -> SetFirResult:
    """
    Program the FPGA FIR filter from an in-memory coefficient list.

    coeffs:    List of Q15 float coefficients in [-1, 1).
    input_sel: Signal routed to the FIR input (FirInputSel enum or raw int 0-3).
    """
    return _send_fir_coeffs(coeffs, input_sel)


@api_cmd
def api_config_io(
    dac1_dat_sel: DacDatSel | int,
    dac2_dat_sel: DacDatSel | int,
    pid_dat_sel: PidDatSel | int,
) -> ConfigIoResult:
    """Route DAC outputs and PID error input."""
    r = execute_cmd(
        f"CMD:config_io\n"
        f"U:{int(dac1_dat_sel)},{int(dac2_dat_sel)},{int(pid_dat_sel)}\n"
    )
    return ConfigIoResult(
        status=r.get("status", -1),
        dac1_dat_sel_cb=r.get("DAC1_DAT_SEL_CB", 0),
        dac2_dat_sel_cb=r.get("DAC2_DAT_SEL_CB", 0),
        pid_dat_sel_cb=r.get("PID_DAT_SEL_CB", 0),
    )


@api_cmd
def api_config_demod(
    ref_sel: DemodRefSel | int,
    in_sel: DemodInSel | int,
    lpf_alpha: int = 8,
) -> ConfigDemodResult:
    """Configure the IQ demodulator reference, input signal sources, and EMA LPF alpha."""
    r = execute_cmd(
        f"CMD:config_demod\n"
        f"U:{int(ref_sel)},{int(in_sel)},{int(lpf_alpha)}\n"
    )
    return ConfigDemodResult(
        status=r.get("status", -1),
        ref_sel_cb=r.get("REF_SEL_CB", 0),
        in_sel_cb=r.get("IN_SEL_CB", 0),
        lpf_alpha_cb=r.get("LPF_ALPHA_CB", 0),
    )


@api_cmd
def api_get_frame(
    decimation: int,
    frame_code: FrameCode | int,
    remote_dir: str = "sw/build",
) -> FrameResult:
    """
    Trigger a DMA capture and retrieve the resulting CSV.

    decimation:  BRAM sample decimation (>= 1).
    frame_code:  which signals to pack into each 64-bit DMA word.
    remote_dir:  directory on the RP where dma_log.csv is written.

    Returns a FrameResult whose .data is a (16384, N_cols) ndarray.
    The column order matches FRAME_COLUMNS[frame_code].
    """
    fc = FrameCode(int(frame_code))
    r = execute_cmd(f"CMD:get_frame\nU:{decimation},{int(fc)}\n")
    status = r.get("status", -1)
    data = np.empty((0,))
    columns = FRAME_COLUMNS.get(fc, [])
    if status == 0:
        local = _ssh_fetch_csv(
            f"{remote_dir}/dma_log.csv",
            f"dma_{fc.name.lower()}.csv",
        )
        data = np.loadtxt(local, delimiter=",")
    return FrameResult(
        status=status,
        dma_engaged=r.get("DMA_ENGAGED_CB", 0),
        decimation_cb=r.get("DECIMATION_CODE_CB", 0),
        frame_code_cb=r.get("FRAME_CODE_CB", 0),
        data=data,
        columns=columns,
    )


@api_cmd
def api_sweep_ramp(
    v0: float,
    v1: float,
    num_points: int,
    dac_sel: DacSel | int,
    write_delay_us: int = 0,
    demod_mode: int = 0,
    remote_dir: str = "sw/build",
) -> SweepRampResult:
    """
    Sweep the selected DAC from v0 to v1.

    demod_mode=0 (default): reads ADC_A, ADC_B, I_FEED, Q_FEED at every step.
                             Returns (num_points, 5) array; columns = SWEEP_COLUMNS.
    demod_mode=1:           reads the EMA-filtered demod output (demod_lpf) at every step.
                             Returns (num_points, 2) array; columns = SWEEP_DEMOD_COLUMNS.
                             Requires api_config_demod to have been called first.
    """
    r = execute_cmd(
        f"CMD:sweep_ramp\n"
        f"F:{v0},{v1}\n"
        f"U:{num_points},{int(dac_sel)},{write_delay_us},{int(demod_mode)}\n"
    )
    status = r.get("status", -1)
    cols = SWEEP_DEMOD_COLUMNS if demod_mode else SWEEP_COLUMNS
    data = np.empty((0, len(cols)))
    if status == 0:
        local = _ssh_fetch_csv(f"{remote_dir}/sweep_log.csv", "sweep_log.csv")
        data  = np.loadtxt(local, delimiter=",")
    return SweepRampResult(
        status=status,
        num_points_cb=r.get("NUM_POINTS_CB", 0),
        data=data,
        columns=cols,
    )


def compute_lockpoint(
    data:         np.ndarray,   # SweepRampResult.data, shape (N, 5)
    sign_sel:     str = "I",    # "I" or "Q"
    invert_delta: bool = False,
    window:       int = 10,     # sliding-window size for slope smoothing
) -> LockPointResult:
    """
    Compute the optimal IQ rotation angle and PDH lock point from sweep data.

    data:         SweepRampResult.data — columns: dac_v, adc_a, adc_b, i_feed, q_feed
    sign_sel:     which feed sets the sign of the golden signal G ("I" or "Q")
    invert_delta: flip the orientation condition (for setups with the opposite standard shape)
    window:       number of points in the sliding window used to find the steepest-slope region
    """
    dac_v  = data[:, 0]
    i_feed = data[:, 3]
    q_feed = data[:, 4]

    sign_basis = i_feed if sign_sel == "I" else q_feed
    G = np.sqrt(i_feed**2 + q_feed**2) * np.sign(sign_basis)

    ix_max = int(np.argmax(G))
    ix_min = int(np.argmin(G))
    delta  = dac_v[ix_max] - dac_v[ix_min]

    should_flip = (delta > 0) if invert_delta else (delta < 0)
    if should_flip:
        G = -G
        ix_max, ix_min = ix_min, ix_max

    optimal_angle_deg = float(np.degrees(np.arctan2(
        np.dot(q_feed, G), np.dot(i_feed, G)
    )))
    slope_mag = np.abs(np.diff(G) / np.diff(dac_v))
    w = max(1, min(window, len(slope_mag)))
    smoothed = np.convolve(slope_mag, np.ones(w) / w, mode='valid')
    ix_peak = int(np.argmax(smoothed)) + w // 2
    lock_point = float(dac_v[ix_peak])

    return LockPointResult(G=G, optimal_angle_deg=optimal_angle_deg, lock_point=lock_point)


def api_psd(decimation: int = 1, frame_code: FrameCode = FrameCode.ADC_DATA_IN) -> PSDResult:
    """
    Capture a DMA frame and compute PSD for all channels via the Wiener-Khinchin
    theorem (FFT of the autocorrelation function).

    decimation:  BRAM decimation factor (>= 1).
                 Effective sample rate = 125e6 / decimation Hz.
                 Nyquist = 62.5e6 / decimation Hz.
    frame_code:  which DMA frame to capture (default: ADC_DATA_IN).

    Returns a PSDResult with:
        freqs   -- frequency bins in Hz, shape (N_freq,)
        psd     -- one-sided PSD in counts²/Hz, shape (N_freq, N_cols)
        fs      -- effective sample rate used
        columns -- channel names per FRAME_COLUMNS[frame_code]
    """
    r    = api_get_frame(decimation, frame_code)
    fs   = FPGA_FS / decimation
    cols = FRAME_COLUMNS[FrameCode(int(frame_code))]
    n_cols = len(cols)

    freqs = np.empty(0)
    psd   = np.empty((0, n_cols))

    if r.status == 0 and r.data.ndim == 2 and r.data.shape[1] == n_cols:
        N    = r.data.shape[0]
        psds = []
        for i in range(n_cols):
            x   = r.data[:, i].astype(float)
            x  -= x.mean()                           # remove DC offset
            acf = np.correlate(x, x, mode='full')    # autocorrelation, length 2N-1
            S   = np.abs(np.fft.rfft(acf))           # Wiener-Khinchin: PSD = FFT(ACF)
            S  /= (fs * N)                            # normalise to counts²/Hz
            psds.append(S)
        freqs = np.fft.rfftfreq(2 * N - 1, d=1.0 / fs)
        psd   = np.column_stack(psds)

    return PSDResult(status=r.status, freqs=freqs, psd=psd, fs=fs, columns=cols, raw_data=r.data)


def api_control_metrics(decimation: int, pid_params: dict) -> ControlMetricsResult:
    """
    Capture a PID_IO DMA frame and compute controller performance metrics.

    decimation:  BRAM decimation factor (>= 1).
    pid_params:  dict of controller parameters to embed in the result
                 (read from GUI fields; not sent to hardware).

    Returns a ControlMetricsResult with PSD, time-domain metrics, and pid_params.
    """
    r   = api_get_frame(decimation, FrameCode.PID_IO)
    fs  = FPGA_FS / decimation
    cols = FRAME_COLUMNS[FrameCode.PID_IO]   # ["pid_in", "err", "pid_out"]
    data = r.data
    N, n_cols = data.shape

    # ── PSD (Wiener-Khinchin, identical to api_psd) ───────────────────────────
    psds = []
    for i in range(n_cols):
        x   = data[:, i].astype(float)
        x  -= x.mean()
        acf = np.correlate(x, x, mode='full')
        S   = np.abs(np.fft.rfft(acf))
        S  /= (fs * N)
        psds.append(S)
    freqs = np.fft.rfftfreq(2 * N - 1, d=1.0 / fs)
    psd   = np.column_stack(psds)

    # ── Signal extraction ─────────────────────────────────────────────────────
    err     = data[:, 1].astype(float)
    pid_out = data[:, 2].astype(float)

    # ── RMS tracking error ────────────────────────────────────────────────────
    rms_error = float(np.sqrt(np.mean(err ** 2)))

    # ── Settling time ─────────────────────────────────────────────────────────
    # Steady state = mean of last 20 % of samples.
    # Settled = |err − steady| ≤ 5 % of max|err| for all remaining samples.
    steady    = float(np.mean(err[-(N // 5):]))
    peak_abs  = float(np.max(np.abs(err)))
    band      = 0.05 * peak_abs if peak_abs > 0 else 1.0
    in_band   = np.abs(err - steady) <= band
    settling_time = float("nan")
    for i in range(N):
        if np.all(in_band[i:]):
            settling_time = float(i) / fs
            break

    # ── Overshoot ─────────────────────────────────────────────────────────────
    # Measured over the first 20 % of the capture (transient window).
    transient = err[:N // 5]
    peak_range = float(np.max(transient) - np.min(transient))
    if peak_range > 0:
        overshoot = float((float(np.max(transient)) - steady) / peak_range * 100.0)
    else:
        overshoot = 0.0

    # ── Control output metrics ────────────────────────────────────────────────
    ctrl_rms      = float(np.sqrt(np.mean(pid_out ** 2)))
    ctrl_max      = float(np.max(np.abs(pid_out)))
    ctrl_p95      = float(np.percentile(np.abs(pid_out), 95))
    ctrl_slew_rms = float(np.sqrt(np.mean(np.diff(pid_out) ** 2)))

    # ── Cross-correlation: pid_out[t] vs err[t + lag] ─────────────────────────
    # Measures how strongly (and with what delay) the controller output causally
    # drives future error reduction.  Normalised to [-1, 1]; peak magnitude is
    # the "correction quality" score.
    po_z  = pid_out - pid_out.mean()
    er_z  = err     - err.mean()
    denom = float(np.std(po_z) * np.std(er_z))
    if denom > 0:
        ccf_raw = np.correlate(po_z, er_z, mode='full') / (denom * N)
    else:
        ccf_raw = np.zeros(2 * N - 1)
    lag_samples  = np.arange(-(N - 1), N)
    ccf_lags     = lag_samples / fs                         # seconds
    ccf_peak_idx = int(np.argmax(np.abs(ccf_raw)))
    ccf_peak     = float(ccf_raw[ccf_peak_idx])
    ccf_peak_lag = float(ccf_lags[ccf_peak_idx])

    return ControlMetricsResult(
        raw_data=data,
        columns=cols,
        fs=fs,
        decimation=decimation,
        freqs=freqs,
        psd=psd,
        rms_error=rms_error,
        settling_time=settling_time,
        overshoot=overshoot,
        ctrl_rms=ctrl_rms,
        ctrl_max=ctrl_max,
        ctrl_p95=ctrl_p95,
        ctrl_slew_rms=ctrl_slew_rms,
        ccf_lags=ccf_lags,
        ccf=ccf_raw,
        ccf_peak=ccf_peak,
        ccf_peak_lag=ccf_peak_lag,
        pid_params=pid_params,
    )
