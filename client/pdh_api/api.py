"""
PDH Python client API.

Each api_* function sends one command to the C server, parses the response,
and returns a typed result dataclass.  The @api_cmd decorator enforces
status == 0, raising RuntimeError on any non-zero return code.

CSV files (dma_log.csv, lockin_log.csv) are fetched from the Red Pitaya via
SFTP and saved locally under LOCAL_DATA_DIR.  Plotting is left to the caller.
"""
from __future__ import annotations

import math
import os
import socket
from functools import wraps
from typing import Optional

import numpy as np
import paramiko

from .types import (
    CsSel, ConfigIoResult, DacDatSel, DacSel, FRAME_COLUMNS,
    FrameCode, FrameResult, LockInResult, PidDatSel,
    AdcResult, CheckSignedResult, ResetResult, SetDacResult,
    SetLedResult, SetNcoResult, SetPidResult, SetRotResult,
)


# ── Connection parameters ──────────────────────────────────────────────────────

SERVER_IP   = "10.42.0.62"
SERVER_PORT = 5555
SSH_USER    = "root"
SSH_PASS    = "root"

LOCAL_DATA_DIR    = "data"
REMOTE_DMA_CSV    = "sw/build/dma_log.csv"
REMOTE_LOCKIN_CSV = "sw/build/lockin_log.csv"


# ── Public API surface ─────────────────────────────────────────────────────────

__all__ = [
    # enums / types re-exported for one-stop import
    "FrameCode", "DacSel", "DacDatSel", "PidDatSel", "CsSel",
    # result dataclasses
    "ResetResult", "SetLedResult", "AdcResult", "SetDacResult",
    "CheckSignedResult", "SetRotResult", "SetPidResult",
    "SetNcoResult", "ConfigIoResult", "FrameResult", "LockInResult",
    # API functions
    "api_reset_fpga", "api_set_led", "api_get_adc", "api_set_dac",
    "api_check_signed", "api_set_rotation", "api_set_nco",
    "api_set_pid", "api_config_io", "api_get_frame", "api_lock_in",
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


def execute_cmd(cmd: str) -> dict:
    """Send a single command string to the server; return the parsed response."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
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
) -> SetPidResult:
    """
    Configure the PID controller.

    kp, kd, ki: gains in Q15 range [-1, 1).
    sp:         setpoint in Q13 range [-1, 1).
    dec:        decimation (1-16383).
    alpha:      IIR filter exponent (0-15).
    sat:        output saturation bits (0-31).
    en:         1 to enable, 0 to disable.
    """
    r = execute_cmd(
        f"CMD:set_pid\nF:{kp},{kd},{ki},{sp}\nU:{dec},{alpha},{sat},{en}\n"
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
    )


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
def api_lock_in(
    dac_select: DacSel | int,
    pid_select: PidDatSel | int,
    num_points: int,
    us_delay: int,
    log_data: bool,
    remote_dir: str = "sw/build",
) -> LockInResult:
    """
    Sweep the selected DAC across its full range, find the error-signal zero
    crossing, program that as the initial DAC value, then enable the PID.

    If log_data=True the sweep data is fetched from the RP and stored in
    LockInResult.data as a (num_points, 4) ndarray with columns:
        [dac_voltage, error_signal, setpoint, step_index]

    NOTE: this command overrides config_io and the PID setpoint.
    """
    r = execute_cmd(
        f"CMD:lock_in\n"
        f"U:{int(dac_select)},{int(pid_select)},{num_points},{us_delay},{int(log_data)}\n"
    )
    status = r.get("status", -1)
    data: Optional[np.ndarray] = None
    if status == 0 and log_data:
        local = _ssh_fetch_csv(
            f"{remote_dir}/lockin_log.csv",
            "lockin_log.csv",
        )
        data = np.loadtxt(local, delimiter=",")
    return LockInResult(
        status=status,
        dac_lock_point=r.get("DAC_LOCK_POINT", float("nan")),
        data=data,
    )
