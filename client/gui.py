"""
PDH Controller GUI — interactive hardware configuration via tkinter.

Every subsystem reachable from the Python API is reachable here.
API calls run in background daemon threads; results are posted back
to the main thread via root.after(0, callback).

Layout: three side-by-side columns of labelled panels, all visible at once.
  Col 0: System, IO Routing
  Col 1: NCO, Rotation, FIR
  Col 2: PID, Frame Capture

Usage:
    cd client
    python gui.py
"""
from __future__ import annotations

import datetime
import threading
import tkinter as tk
from tkinter import messagebox, ttk

import matplotlib
matplotlib.use("TkAgg")
import matplotlib.pyplot as plt

import pdh_api as api
from pdh_api.fir_design import design_lowpass, FS

# ── Constants ─────────────────────────────────────────────────────────────────

DEFAULT_IP   = "10.42.0.62"
DEFAULT_PORT = 5555
FIR_NTAPS    = 32       # must match compiled FPGA


# ── Helpers ───────────────────────────────────────────────────────────────────

def _update_connection(ip: str, port: int) -> None:
    """Push IP/port into the api module (called before every command)."""
    api.api.SERVER_IP   = ip
    api.api.SERVER_PORT = port


# ── Application ───────────────────────────────────────────────────────────────

class _App(tk.Tk):
    """Main application window."""

    def __init__(self) -> None:
        super().__init__()
        self.title("PDH Controller")
        self.resizable(True, True)
        self._build_settings_bar()
        self._build_panels()
        self._build_status_bar()
        self._status_timer: str | None = None

    # ── Settings bar ──────────────────────────────────────────────────────────

    def _build_settings_bar(self) -> None:
        bar = ttk.Frame(self, padding=4)
        bar.pack(side=tk.TOP, fill=tk.X)

        ttk.Label(bar, text="IP:").pack(side=tk.LEFT)
        self._ip_var = tk.StringVar(value=DEFAULT_IP)
        ttk.Entry(bar, textvariable=self._ip_var, width=14).pack(side=tk.LEFT, padx=(2, 8))

        ttk.Label(bar, text="Port:").pack(side=tk.LEFT)
        self._port_var = tk.StringVar(value=str(DEFAULT_PORT))
        ttk.Entry(bar, textvariable=self._port_var, width=7).pack(side=tk.LEFT, padx=(2, 0))

    def _connection_params(self) -> tuple[str, int]:
        try:
            port = int(self._port_var.get())
        except ValueError:
            port = DEFAULT_PORT
        return self._ip_var.get().strip() or DEFAULT_IP, port

    # ── Panels ────────────────────────────────────────────────────────────────

    def _build_panels(self) -> None:
        content = ttk.Frame(self, padding=4)
        content.pack(fill=tk.BOTH, expand=True)
        content.columnconfigure(0, weight=1)
        content.columnconfigure(1, weight=1)
        content.columnconfigure(2, weight=1)
        content.rowconfigure(0, weight=1)

        # Three column frames so panels within each column stack cleanly.
        col0 = ttk.Frame(content)
        col1 = ttk.Frame(content)
        col2 = ttk.Frame(content)
        col0.grid(row=0, column=0, sticky=tk.NSEW, padx=(0, 4))
        col1.grid(row=0, column=1, sticky=tk.NSEW, padx=4)
        col2.grid(row=0, column=2, sticky=tk.NSEW, padx=(4, 0))

        # Col 0: System, IO Routing
        _SystemPanel(col0, self).pack(fill=tk.X, pady=(0, 4))
        _IORoutingPanel(col0, self).pack(fill=tk.X)

        # Col 1: NCO, Rotation, FIR
        _NCOPanel(col1, self).pack(fill=tk.X, pady=(0, 4))
        _RotationPanel(col1, self).pack(fill=tk.X, pady=(0, 4))
        _FIRPanel(col1, self).pack(fill=tk.X)

        # Col 2: PID, Frame Capture
        _PIDPanel(col2, self).pack(fill=tk.X, pady=(0, 4))
        _FrameCapturePanel(col2, self).pack(fill=tk.X)

    # ── Status bar ────────────────────────────────────────────────────────────

    def _build_status_bar(self) -> None:
        self._status_var = tk.StringVar(value="Ready")
        self._status_lbl = ttk.Label(
            self, textvariable=self._status_var,
            relief=tk.SUNKEN, anchor=tk.W, padding=(4, 2),
        )
        self._status_lbl.pack(side=tk.BOTTOM, fill=tk.X)

    def set_status(self, msg: str, ok: bool = True) -> None:
        self._status_var.set(msg)
        self._status_lbl.configure(foreground="green" if ok else "red")
        if self._status_timer:
            self.after_cancel(self._status_timer)
        self._status_timer = self.after(10_000, self._clear_status)

    def _clear_status(self) -> None:
        self._status_var.set("Ready")
        self._status_lbl.configure(foreground="")
        self._status_timer = None

    # ── Threading helper ──────────────────────────────────────────────────────

    def run_in_bg(self, fn, on_success, on_error) -> None:
        """Run fn() in a daemon thread; call on_success(result) or on_error(exc) on main thread."""
        def worker():
            try:
                result = fn()
                self.after(0, lambda: on_success(result))
            except Exception as exc:
                self.after(0, lambda: on_error(exc))
        threading.Thread(target=worker, daemon=True).start()


# ── Panel base ────────────────────────────────────────────────────────────────

class _Panel(ttk.LabelFrame):
    """Base class for all control panels. Each panel is a LabelFrame."""

    def __init__(self, parent, app: _App, title: str) -> None:
        super().__init__(parent, text=title, padding=8)
        self.app = app

    def _conn(self) -> tuple[str, int]:
        return self.app._connection_params()

    def _prep_call(self, ip: str, port: int) -> None:
        _update_connection(ip, port)

    def _busy(self, btn: ttk.Button, label: str = "Working…") -> None:
        btn.configure(state=tk.DISABLED, text=label)

    def _unbusy(self, btn: ttk.Button, label: str) -> None:
        btn.configure(state=tk.NORMAL, text=label)

    def ok(self, msg: str) -> None:
        self.app.set_status(msg, ok=True)

    def err(self, exc: Exception) -> None:
        self.app.set_status(str(exc), ok=False)


# ── System panel ──────────────────────────────────────────────────────────────

class _SystemPanel(_Panel):
    def __init__(self, parent, app):
        super().__init__(parent, app, "System")
        self._build()

    def _build(self) -> None:
        # Reset
        reset_frm = ttk.LabelFrame(self, text="Reset", padding=6)
        reset_frm.pack(fill=tk.X, pady=(0, 6))
        self._reset_btn = ttk.Button(reset_frm, text="Reset FPGA", command=self._on_reset)
        self._reset_btn.pack(anchor=tk.W)

        # ADC readback
        adc_frm = ttk.LabelFrame(self, text="ADC", padding=6)
        adc_frm.pack(fill=tk.X, pady=(0, 6))
        self._read_adc_btn = ttk.Button(adc_frm, text="Read ADC", command=self._on_read_adc)
        self._read_adc_btn.grid(row=0, column=0, columnspan=2, sticky=tk.W, pady=(0, 4))
        ttk.Label(adc_frm, text="IN1:").grid(row=1, column=0, sticky=tk.W)
        self._in1_var = tk.StringVar(value="—")
        ttk.Label(adc_frm, textvariable=self._in1_var, width=12).grid(row=1, column=1, sticky=tk.W)
        ttk.Label(adc_frm, text="IN2:").grid(row=2, column=0, sticky=tk.W)
        self._in2_var = tk.StringVar(value="—")
        ttk.Label(adc_frm, textvariable=self._in2_var, width=12).grid(row=2, column=1, sticky=tk.W)

        # DAC
        dac_frm = ttk.LabelFrame(self, text="DAC Register", padding=6)
        dac_frm.pack(fill=tk.X, pady=(0, 6))
        ttk.Label(dac_frm, text="DAC1 [-1,1]:").grid(row=0, column=0, sticky=tk.W)
        self._dac1_var = tk.StringVar(value="0.0")
        ttk.Entry(dac_frm, textvariable=self._dac1_var, width=8).grid(row=0, column=1, padx=4)
        self._dac1_btn = ttk.Button(dac_frm, text="Set DAC1", command=self._on_set_dac1)
        self._dac1_btn.grid(row=0, column=2)
        ttk.Label(dac_frm, text="DAC2 [-1,1]:").grid(row=1, column=0, sticky=tk.W, pady=(4, 0))
        self._dac2_var = tk.StringVar(value="0.0")
        ttk.Entry(dac_frm, textvariable=self._dac2_var, width=8).grid(row=1, column=1, padx=4, pady=(4, 0))
        self._dac2_btn = ttk.Button(dac_frm, text="Set DAC2", command=self._on_set_dac2)
        self._dac2_btn.grid(row=1, column=2, pady=(4, 0))

        # LED
        led_frm = ttk.LabelFrame(self, text="LED", padding=6)
        led_frm.pack(fill=tk.X)
        ttk.Label(led_frm, text="Pattern (0–255):").grid(row=0, column=0, sticky=tk.W)
        self._led_var = tk.StringVar(value="0")
        ttk.Entry(led_frm, textvariable=self._led_var, width=8).grid(row=0, column=1, padx=4)
        self._led_btn = ttk.Button(led_frm, text="Set LED", command=self._on_set_led)
        self._led_btn.grid(row=0, column=2)

    def _on_reset(self) -> None:
        if not messagebox.askyesno("Reset FPGA", "Reset the FPGA? This will clear all state."):
            return
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._reset_btn)
        self.app.run_in_bg(
            api.api_reset_fpga,
            lambda r: (self._unbusy(self._reset_btn, "Reset FPGA"),
                       self.ok(f"FPGA reset  rst_raw_cb={r.rst_raw_cb}")),
            lambda e: (self._unbusy(self._reset_btn, "Reset FPGA"), self.err(e)),
        )

    def _on_read_adc(self) -> None:
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._read_adc_btn)
        def on_ok(r):
            self._in1_var.set(f"{r.in1_v:.4f} V")
            self._in2_var.set(f"{r.in2_v:.4f} V")
            self._unbusy(self._read_adc_btn, "Read ADC")
            self.ok(f"ADC  IN1={r.in1_v:.4f} V  IN2={r.in2_v:.4f} V")
        def on_err(e):
            self._unbusy(self._read_adc_btn, "Read ADC")
            self.err(e)
        self.app.run_in_bg(api.api_get_adc, on_ok, on_err)

    def _dac_call(self, var, sel, btn, label) -> None:
        try:
            v = float(var.get())
        except ValueError:
            self.err(ValueError(f"Invalid DAC value: {var.get()!r}")); return
        if not (-1.0 <= v <= 1.0):
            self.err(ValueError(f"DAC value {v} out of range [-1, 1]")); return
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(btn)
        self.app.run_in_bg(
            lambda: api.api_set_dac(v, sel),
            lambda r: (self._unbusy(btn, label),
                       self.ok(f"{label}: dac1_code={r.dac1_code_cb}  dac2_code={r.dac2_code_cb}")),
            lambda e: (self._unbusy(btn, label), self.err(e)),
        )

    def _on_set_dac1(self) -> None:
        self._dac_call(self._dac1_var, api.DacSel.DAC_1, self._dac1_btn, "Set DAC1")

    def _on_set_dac2(self) -> None:
        self._dac_call(self._dac2_var, api.DacSel.DAC_2, self._dac2_btn, "Set DAC2")

    def _on_set_led(self) -> None:
        raw = self._led_var.get().strip()
        try:
            v = int(raw, 0)
        except ValueError:
            self.err(ValueError(f"Invalid LED value: {raw!r}")); return
        if not (0 <= v <= 255):
            self.err(ValueError(f"LED value {v} out of range [0, 255]")); return
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._led_btn)
        self.app.run_in_bg(
            lambda: api.api_set_led(v),
            lambda r: (self._unbusy(self._led_btn, "Set LED"),
                       self.ok(f"LED  code={r.led_code_cb:#04x}")),
            lambda e: (self._unbusy(self._led_btn, "Set LED"), self.err(e)),
        )


# ── IO Routing panel ──────────────────────────────────────────────────────────

class _IORoutingPanel(_Panel):
    def __init__(self, parent, app):
        super().__init__(parent, app, "IO Routing")
        self._build()

    def _build(self) -> None:
        d1_frm = ttk.LabelFrame(self, text="DAC1 Source", padding=6)
        d1_frm.pack(fill=tk.X, pady=(0, 6))
        self._dac1_sel = tk.IntVar(value=int(api.DacDatSel.REGISTER))
        for label, val in [("Register", api.DacDatSel.REGISTER), ("PID", api.DacDatSel.PID),
                           ("NCO 1",    api.DacDatSel.NCO_1),   ("NCO 2", api.DacDatSel.NCO_2)]:
            ttk.Radiobutton(d1_frm, text=label, variable=self._dac1_sel,
                            value=int(val)).pack(side=tk.LEFT, padx=2)

        d2_frm = ttk.LabelFrame(self, text="DAC2 Source", padding=6)
        d2_frm.pack(fill=tk.X, pady=(0, 6))
        self._dac2_sel = tk.IntVar(value=int(api.DacDatSel.REGISTER))
        for label, val in [("Register", api.DacDatSel.REGISTER), ("PID", api.DacDatSel.PID),
                           ("NCO 1",    api.DacDatSel.NCO_1),   ("NCO 2", api.DacDatSel.NCO_2)]:
            ttk.Radiobutton(d2_frm, text=label, variable=self._dac2_sel,
                            value=int(val)).pack(side=tk.LEFT, padx=2)

        pid_frm = ttk.LabelFrame(self, text="PID Input", padding=6)
        pid_frm.pack(fill=tk.X, pady=(0, 6))
        self._pid_sel = tk.IntVar(value=int(api.PidDatSel.I_FEED))
        for label, val in [("I Feed",  api.PidDatSel.I_FEED), ("Q Feed",  api.PidDatSel.Q_FEED),
                           ("ADC A",   api.PidDatSel.ADC_A),  ("ADC B",   api.PidDatSel.ADC_B),
                           ("FIR Out", api.PidDatSel.FIR_OUT)]:
            ttk.Radiobutton(pid_frm, text=label, variable=self._pid_sel,
                            value=int(val)).pack(side=tk.LEFT, padx=2)

        self._apply_btn = ttk.Button(self, text="Apply", command=self._on_apply)
        self._apply_btn.pack(anchor=tk.W, pady=(4, 2))
        self._fb_var = tk.StringVar(value="")
        ttk.Label(self, textvariable=self._fb_var, foreground="gray").pack(anchor=tk.W)

    def _on_apply(self) -> None:
        d1  = api.DacDatSel(self._dac1_sel.get())
        d2  = api.DacDatSel(self._dac2_sel.get())
        pid = api.PidDatSel(self._pid_sel.get())
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._apply_btn)
        def on_ok(r):
            self._fb_var.set(
                f"dac1={api.DacDatSel(r.dac1_dat_sel_cb).name}  "
                f"dac2={api.DacDatSel(r.dac2_dat_sel_cb).name}  "
                f"pid={api.PidDatSel(r.pid_dat_sel_cb).name}"
            )
            self._unbusy(self._apply_btn, "Apply")
            self.ok("IO routing applied")
        def on_err(e):
            self._unbusy(self._apply_btn, "Apply")
            self.err(e)
        self.app.run_in_bg(lambda: api.api_config_io(d1, d2, pid), on_ok, on_err)


# ── NCO panel ─────────────────────────────────────────────────────────────────

class _NCOPanel(_Panel):
    def __init__(self, parent, app):
        super().__init__(parent, app, "NCO")
        self._build()

    def _build(self) -> None:
        ttk.Label(self, text="Frequency (Hz):").grid(row=0, column=0, sticky=tk.W)
        self._freq_var = tk.StringVar(value="2000000")
        ttk.Entry(self, textvariable=self._freq_var, width=14).grid(row=0, column=1, padx=4)

        ttk.Label(self, text="Phase shift (°):").grid(row=1, column=0, sticky=tk.W, pady=(4, 0))
        self._shift_var = tk.StringVar(value="0")
        ttk.Entry(self, textvariable=self._shift_var, width=14).grid(row=1, column=1, padx=4, pady=(4, 0))

        self._en_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(self, text="Enable", variable=self._en_var).grid(
            row=2, column=0, columnspan=2, sticky=tk.W, pady=(4, 0))

        self._apply_btn = ttk.Button(self, text="Apply", command=self._on_apply)
        self._apply_btn.grid(row=3, column=0, columnspan=2, sticky=tk.W, pady=(6, 0))

        self._fb_var = tk.StringVar(value="")
        ttk.Label(self, textvariable=self._fb_var, foreground="gray",
                  wraplength=300).grid(row=4, column=0, columnspan=2, sticky=tk.W, pady=(4, 0))

    def _on_apply(self) -> None:
        try:
            freq  = float(self._freq_var.get())
            shift = float(self._shift_var.get())
        except ValueError as e:
            self.err(e); return
        if freq <= 0 or freq > FS / 2:
            self.err(ValueError(f"Frequency must be in (0, {FS/2:.0f}]")); return
        if not (-180 <= shift <= 180):
            self.err(ValueError("Phase shift must be in [-180, 180]")); return
        en = int(self._en_var.get())
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._apply_btn)
        def on_ok(r):
            self._fb_var.set(
                f"freq={r.registered_freq:.1f} Hz  err={r.registered_freq_error:.2f} Hz  "
                f"shift={r.registered_phase_shift:.2f}°  en={r.en_cb}"
            )
            self._unbusy(self._apply_btn, "Apply")
            self.ok(f"NCO set  freq≈{r.registered_freq:.1f} Hz  en={r.en_cb}")
        def on_err(e):
            self._unbusy(self._apply_btn, "Apply")
            self.err(e)
        self.app.run_in_bg(lambda: api.api_set_nco(freq, shift, en), on_ok, on_err)


# ── Rotation panel ────────────────────────────────────────────────────────────

class _RotationPanel(_Panel):
    def __init__(self, parent, app):
        super().__init__(parent, app, "IQ Rotation")
        self._build()

    def _build(self) -> None:
        ttk.Label(self, text="Angle (°):").grid(row=0, column=0, sticky=tk.W)
        self._angle_var = tk.DoubleVar(value=0.0)
        ttk.Entry(self, textvariable=self._angle_var, width=10).grid(row=0, column=1, padx=4)

        scale = ttk.Scale(self, from_=-180, to=180, orient=tk.HORIZONTAL,
                          variable=self._angle_var)
        scale.grid(row=1, column=0, columnspan=2, sticky=tk.EW, pady=(4, 0))

        self._apply_btn = ttk.Button(self, text="Apply", command=self._on_apply)
        self._apply_btn.grid(row=2, column=0, columnspan=2, sticky=tk.W, pady=(6, 0))

        self._fb_var = tk.StringVar(value="")
        ttk.Label(self, textvariable=self._fb_var, foreground="gray",
                  wraplength=300).grid(row=3, column=0, columnspan=2, sticky=tk.W, pady=(4, 0))

    def _on_apply(self) -> None:
        try:
            theta = float(self._angle_var.get())
        except (ValueError, tk.TclError) as e:
            self.err(e); return
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._apply_btn)
        def on_ok(r):
            self._fb_var.set(
                f"cos={r.cos_cb:.4f}  sin={r.sin_cb:.4f}  "
                f"i={r.i_feed_cb:.4f}  q={r.q_feed_cb:.4f}"
            )
            self._unbusy(self._apply_btn, "Apply")
            self.ok(f"Rotation set  θ={theta:.1f}°")
        def on_err(e):
            self._unbusy(self._apply_btn, "Apply")
            self.err(e)
        self.app.run_in_bg(lambda: api.api_set_rotation(theta), on_ok, on_err)


# ── FIR panel ─────────────────────────────────────────────────────────────────

class _FIRPanel(_Panel):
    def __init__(self, parent, app):
        super().__init__(parent, app, "FIR")
        self._build()

    def _build(self) -> None:
        in_frm = ttk.LabelFrame(self, text="Input", padding=6)
        in_frm.pack(fill=tk.X, pady=(0, 6))
        self._input_sel = tk.IntVar(value=int(api.FirInputSel.ADC1))
        for label, val in [("ADC 1",  api.FirInputSel.ADC1), ("ADC 2",  api.FirInputSel.ADC2),
                           ("I Feed", api.FirInputSel.I_FEED), ("Q Feed", api.FirInputSel.Q_FEED)]:
            ttk.Radiobutton(in_frm, text=label, variable=self._input_sel,
                            value=int(val)).pack(side=tk.LEFT, padx=2)

        des_frm = ttk.LabelFrame(self, text="Filter Design", padding=6)
        des_frm.pack(fill=tk.X, pady=(0, 6))

        ttk.Label(des_frm, text="NTAPS:").grid(row=0, column=0, sticky=tk.W)
        ttk.Label(des_frm, text=str(FIR_NTAPS)).grid(row=0, column=1, sticky=tk.W, padx=4)

        ttk.Label(des_frm, text="Corner (Hz):").grid(row=1, column=0, sticky=tk.W, pady=(4, 0))
        self._corner_var = tk.StringVar(value="5000000")
        ttk.Entry(des_frm, textvariable=self._corner_var, width=12).grid(row=1, column=1, padx=4, pady=(4, 0))

        ttk.Label(des_frm, text="Window:").grid(row=2, column=0, sticky=tk.W, pady=(4, 0))
        self._window_var = tk.StringVar(value="hann")
        ttk.Combobox(des_frm, textvariable=self._window_var, width=10,
                     values=["hann", "hamming", "blackman", "rectangular"],
                     state="readonly").grid(row=2, column=1, padx=4, pady=(4, 0))

        self._apply_btn = ttk.Button(self, text="Apply", command=self._on_apply)
        self._apply_btn.pack(anchor=tk.W, pady=(4, 2))
        self._fb_var = tk.StringVar(value="")
        ttk.Label(self, textvariable=self._fb_var, foreground="gray").pack(anchor=tk.W)

    def _on_apply(self) -> None:
        try:
            corner = float(self._corner_var.get())
        except ValueError:
            self.err(ValueError(f"Invalid corner frequency: {self._corner_var.get()!r}")); return
        if corner <= 0 or corner >= FS / 2:
            self.err(ValueError(f"Corner must be in (0, {FS/2:.0f})")); return
        window = self._window_var.get()
        try:
            coeffs = design_lowpass(FIR_NTAPS, corner, window)
        except ValueError as e:
            self.err(e); return
        dc_gain  = sum(coeffs)
        max_coef = max(abs(c) for c in coeffs)
        sel = api.FirInputSel(self._input_sel.get())
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._apply_btn)
        def on_ok(r):
            self._fb_var.set(
                f"DC gain={dc_gain:.4f}  max|c|={max_coef:.4f}  input={sel.name}"
            )
            self._unbusy(self._apply_btn, "Apply")
            self.ok(f"FIR set  corner={corner/1e6:.3f} MHz  window={window}  input={sel.name}")
        def on_err(e):
            self._unbusy(self._apply_btn, "Apply")
            self.err(e)
        self.app.run_in_bg(lambda: api.api_set_fir_coeffs(coeffs, sel), on_ok, on_err)


# ── PID panel ─────────────────────────────────────────────────────────────────

class _PIDPanel(_Panel):
    def __init__(self, parent, app):
        super().__init__(parent, app, "PID")
        self._build()

    def _build(self) -> None:
        fields = [
            ("Kp [-1, 1):",      "kp",    "0.5"),
            ("Ki [-1, 1):",      "ki",    "0.0"),
            ("Kd [-1, 1):",      "kd",    "0.0"),
            ("Setpoint [-1,1):", "sp",    "0.0"),
            ("Decimation:",      "dec",   "100"),
            ("Alpha (0–15):",    "alpha", "4"),
            ("Satwidth (15–31):","sat",   "31"),
        ]
        self._vars: dict[str, tk.StringVar] = {}
        for row, (label, key, default) in enumerate(fields):
            ttk.Label(self, text=label).grid(row=row, column=0, sticky=tk.W, pady=1)
            v = tk.StringVar(value=default)
            self._vars[key] = v
            ttk.Entry(self, textvariable=v, width=10).grid(row=row, column=1, padx=6, pady=1)

        self._en_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(self, text="Enable", variable=self._en_var).grid(
            row=len(fields), column=0, columnspan=2, sticky=tk.W, pady=(4, 0))

        self._apply_btn = ttk.Button(self, text="Apply", command=self._on_apply)
        self._apply_btn.grid(row=len(fields)+1, column=0, columnspan=2, sticky=tk.W, pady=(6, 0))

        self._fb_var = tk.StringVar(value="")
        ttk.Label(self, textvariable=self._fb_var, foreground="gray",
                  wraplength=300).grid(row=len(fields)+2, column=0, columnspan=2,
                                       sticky=tk.W, pady=(4, 0))

    def _on_apply(self) -> None:
        try:
            kp    = float(self._vars["kp"].get())
            ki    = float(self._vars["ki"].get())
            kd    = float(self._vars["kd"].get())
            sp    = float(self._vars["sp"].get())
            dec   = int(self._vars["dec"].get())
            alpha = int(self._vars["alpha"].get())
            sat   = int(self._vars["sat"].get())
        except ValueError as e:
            self.err(e); return
        for name, v, lo, hi in [("kp", kp, -1.0, 1.0), ("ki", ki, -1.0, 1.0),
                                  ("kd", kd, -1.0, 1.0), ("sp", sp, -1.0, 1.0)]:
            if not (lo <= v < hi):
                self.err(ValueError(f"{name}={v} not in [{lo}, {hi})")); return
        if not (1 <= dec <= 16383):
            self.err(ValueError(f"Decimation {dec} not in [1, 16383]")); return
        if not (0 <= alpha <= 15):
            self.err(ValueError(f"Alpha {alpha} not in [0, 15]")); return
        if not (15 <= sat <= 31):
            self.err(ValueError(f"Satwidth {sat} not in [15, 31]")); return
        en = int(self._en_var.get())
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._apply_btn)
        def on_ok(r):
            self._fb_var.set(
                f"kp={r.kp_cb:.4f}  ki={r.ki_cb:.4f}  kd={r.kd_cb:.4f}  "
                f"sp={r.sp_cb:.4f}  dec={r.dec_cb}  alpha={r.alpha_cb}  "
                f"sat={r.sat_cb}  en={r.en_cb}"
            )
            self._unbusy(self._apply_btn, "Apply")
            self.ok(f"PID set  kp={r.kp_cb:.4f}  en={r.en_cb}")
        def on_err(e):
            self._unbusy(self._apply_btn, "Apply")
            self.err(e)
        self.app.run_in_bg(
            lambda: api.api_set_pid(kp, kd, ki, sp, dec, alpha, sat, en), on_ok, on_err
        )


# ── Frame Capture panel ───────────────────────────────────────────────────────

class _FrameCapturePanel(_Panel):
    def __init__(self, parent, app):
        super().__init__(parent, app, "Frame Capture")
        self._build()

    def _build(self) -> None:
        ttk.Label(self, text="Frame code:").grid(row=0, column=0, sticky=tk.W)
        self._fc_var = tk.StringVar(value=api.FrameCode.ANGLES_AND_ESIGS.name)
        ttk.Combobox(self, textvariable=self._fc_var,
                     values=[fc.name for fc in api.FrameCode],
                     width=20, state="readonly").grid(row=0, column=1, padx=4)

        ttk.Label(self, text="Decimation:").grid(row=1, column=0, sticky=tk.W, pady=(4, 0))
        self._dec_var = tk.StringVar(value="1")
        ttk.Entry(self, textvariable=self._dec_var, width=8).grid(row=1, column=1, padx=4, pady=(4, 0))

        self._capture_btn = ttk.Button(self, text="Capture Frame", command=self._on_capture)
        self._capture_btn.grid(row=2, column=0, columnspan=2, sticky=tk.W, pady=(8, 0))

    def _on_capture(self) -> None:
        try:
            dec = int(self._dec_var.get())
        except ValueError:
            self.err(ValueError(f"Invalid decimation: {self._dec_var.get()!r}")); return
        if dec < 1:
            self.err(ValueError("Decimation must be >= 1")); return
        try:
            fc = api.FrameCode[self._fc_var.get()]
        except KeyError:
            self.err(ValueError(f"Unknown frame code: {self._fc_var.get()!r}")); return
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._capture_btn, "Capturing…")

        def on_ok(r: api.FrameResult):
            self._unbusy(self._capture_btn, "Capture Frame")
            self.ok(f"Captured {r.data.shape[0]} samples — {fc.name}")
            self._plot_frame(r, fc, dec)

        def on_err(e):
            self._unbusy(self._capture_btn, "Capture Frame")
            self.err(e)

        self.app.run_in_bg(lambda: api.api_get_frame(dec, fc), on_ok, on_err)

    def _plot_frame(self, r: api.FrameResult, fc: api.FrameCode, dec: int) -> None:
        cols   = r.columns
        n_cols = len(cols) if cols else 1
        data   = r.data
        ts     = datetime.datetime.now().strftime("%H:%M:%S")

        fig, axes = plt.subplots(n_cols, 1, figsize=(10, 2.5 * n_cols), squeeze=False,
                                 sharex=True)
        fig.suptitle(f"{fc.name}  dec={dec}  {ts}", fontsize=10)

        for i, ax in enumerate(axes[:, 0]):
            if data.ndim == 2 and i < data.shape[1]:
                y        = data[:, i]
                col_name = cols[i] if i < len(cols) else f"col{i}"
            elif data.ndim == 1 and i == 0:
                y        = data
                col_name = cols[0] if cols else "data"
            else:
                continue
            ax.plot(y, linewidth=0.6)
            ax.set_ylabel(col_name, fontsize=12)
            ax.set_xlabel("sample" if i == n_cols - 1 else "")
            ax.grid(True, alpha=0.3)

        fig.tight_layout()
        plt.show(block=False)


# ── Entry point ───────────────────────────────────────────────────────────────

def main() -> None:
    app = _App()
    app.mainloop()


if __name__ == "__main__":
    main()
