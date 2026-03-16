"""
PDH Controller GUI — interactive hardware configuration via tkinter.

Every subsystem reachable from the Python API is reachable here.
API calls run in background daemon threads; results are posted back
to the main thread via root.after(0, callback).

Layout: three side-by-side columns of labelled panels, all visible at once.
  Col 0: System, IO Routing, Sweep Ramp
  Col 1: NCO, Rotation, FIR
  Col 2: PID, Frame Capture, PSD

Usage:
    cd client
    python gui.py
"""
from __future__ import annotations

import datetime
import json
import threading
import tkinter as tk
import tkinter.filedialog as _fd
from tkinter import messagebox, ttk

import numpy as np

import matplotlib
matplotlib.use("TkAgg")
import matplotlib.pyplot as plt

import pdh_api as api
from pdh_api.fir_design import design_lowpass, FS

# ── Constants ─────────────────────────────────────────────────────────────────

DEFAULT_IP   = "10.42.0.62"
DEFAULT_PORT = 5555
FIR_NTAPS    = 32       # must match compiled FPGA

PLOT_LINEWIDTH = 0.9    # base line width (≈ 50% thicker than previous 0.6)
PLOT_FONTSIZE  = 20     # base font size (2× previous 10–12 pt labels)


# ── Helpers ───────────────────────────────────────────────────────────────────

def _update_connection(ip: str, port: int) -> None:
    """Push IP/port into the api module (called before every command)."""
    api.api.SERVER_IP   = ip
    api.api.SERVER_PORT = port


def _apply_figure_style(fig, linewidth: float, fontsize: float) -> None:
    """Apply uniform linewidth and fontsize to all axes in fig."""
    tick_fs = fontsize * 0.85
    for ax in fig.get_axes():
        for line in ax.get_lines():
            line.set_linewidth(linewidth)
        ax.xaxis.label.set_fontsize(fontsize)
        ax.yaxis.label.set_fontsize(fontsize)
        ax.tick_params(labelsize=tick_fs)
        legend = ax.get_legend()
        if legend:
            for t in legend.get_texts():
                t.set_fontsize(tick_fs)
    if fig.texts:
        fig.texts[0].set_fontsize(fontsize)
    fig.canvas.draw_idle()


def _add_figure_menu(fig, csv_data: np.ndarray, csv_columns: list,
                     extra_csv_data: np.ndarray | None = None,
                     extra_csv_columns: list | None = None,
                     extra_csv_label: str = "Save DMA CSV…",
                     json_data: dict | None = None) -> None:
    """Attach Style and Export menu bar to the figure's Tk window."""
    win = fig.canvas.manager.window

    menubar = tk.Menu(win)

    # ── Style cascade ──────────────────────────────────────────────────────────
    style_menu = tk.Menu(menubar, tearoff=0)

    def open_style_dialog():
        dlg = tk.Toplevel(win)
        dlg.title("Adjust Style")
        dlg.resizable(False, False)

        cur_lw = PLOT_LINEWIDTH
        cur_fs = PLOT_FONTSIZE
        lines = [l for ax in fig.get_axes() for l in ax.get_lines()]
        if lines:
            cur_lw = lines[0].get_linewidth()
        texts = [ax.xaxis.label for ax in fig.get_axes() if ax.xaxis.label.get_text()]
        if texts:
            cur_fs = texts[0].get_fontsize()

        tk.Label(dlg, text="Line width:").grid(row=0, column=0, padx=8, pady=(8, 2), sticky=tk.W)
        lw_scale = tk.Scale(dlg, from_=0.1, to=8.0, resolution=0.1,
                            orient=tk.HORIZONTAL, length=220)
        lw_scale.set(cur_lw)
        lw_scale.grid(row=0, column=1, padx=8, pady=(8, 2))

        tk.Label(dlg, text="Font size:").grid(row=1, column=0, padx=8, pady=2, sticky=tk.W)
        fs_scale = tk.Scale(dlg, from_=6, to=40, resolution=1,
                            orient=tk.HORIZONTAL, length=220)
        fs_scale.set(int(cur_fs))
        fs_scale.grid(row=1, column=1, padx=8, pady=2)

        def on_apply():
            _apply_figure_style(fig, lw_scale.get(), fs_scale.get())

        btn_frm = tk.Frame(dlg)
        btn_frm.grid(row=2, column=0, columnspan=2, pady=8)
        ttk.Button(btn_frm, text="Apply", command=on_apply).pack(side=tk.LEFT, padx=4)
        ttk.Button(btn_frm, text="Close", command=dlg.destroy).pack(side=tk.LEFT, padx=4)

    style_menu.add_command(label="Adjust…", command=open_style_dialog)
    menubar.add_cascade(label="Style", menu=style_menu)

    # ── Export cascade ─────────────────────────────────────────────────────────
    export_menu = tk.Menu(menubar, tearoff=0)

    def save_csv():
        path = _fd.asksaveasfilename(
            parent=win,
            title="Export CSV",
            defaultextension=".csv",
            filetypes=[("CSV files", "*.csv"), ("All files", "*.*")],
        )
        if path:
            header = ",".join(csv_columns)
            np.savetxt(path, csv_data, delimiter=",", header=header, comments="")

    export_menu.add_command(label="Save CSV…", command=save_csv)

    if extra_csv_data is not None and extra_csv_columns is not None:
        def save_extra_csv():
            path = _fd.asksaveasfilename(
                parent=win,
                title="Export DMA CSV",
                defaultextension=".csv",
                filetypes=[("CSV files", "*.csv"), ("All files", "*.*")],
            )
            if path:
                header = ",".join(extra_csv_columns)
                np.savetxt(path, extra_csv_data, delimiter=",", header=header, comments="")

        export_menu.add_command(label=extra_csv_label, command=save_extra_csv)

    if json_data is not None:
        def save_json():
            path = _fd.asksaveasfilename(
                parent=win,
                title="Export JSON",
                defaultextension=".json",
                filetypes=[("JSON files", "*.json"), ("All files", "*.*")],
            )
            if path:
                with open(path, "w") as f:
                    json.dump(json_data, f, indent=2)

        export_menu.add_command(label="Export JSON…", command=save_json)

    menubar.add_cascade(label="Export", menu=export_menu)

    win.config(menu=menubar)


# ── Application ───────────────────────────────────────────────────────────────

class _App(tk.Tk):
    """Main application window."""

    def __init__(self) -> None:
        super().__init__()
        self.title("Pitaya Whisperer")
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

        # Col 0: System, IO Routing, Sweep Ramp
        _SystemPanel(col0, self).pack(fill=tk.X, pady=(0, 4))
        self._io_panel = _IORoutingPanel(col0, self)
        self._io_panel.pack(fill=tk.X, pady=(0, 4))
        _SweepRampPanel(col0, self).pack(fill=tk.X)

        # Col 1: NCO, Rotation, Demod, FIR, Autolock
        _NCOPanel(col1, self).pack(fill=tk.X, pady=(0, 4))
        _RotationPanel(col1, self).pack(fill=tk.X, pady=(0, 4))
        _DemodPanel(col1, self).pack(fill=tk.X, pady=(0, 4))
        _FIRPanel(col1, self).pack(fill=tk.X, pady=(0, 4))
        _AutolockPanel(col1, self).pack(fill=tk.X)

        # Col 2: PID, Frame Capture, PSD
        self._pid_panel = _PIDPanel(col2, self)
        self._pid_panel.pack(fill=tk.X, pady=(0, 4))
        _FrameCapturePanel(col2, self).pack(fill=tk.X, pady=(0, 4))
        _PSDPanel(col2, self).pack(fill=tk.X)

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
        for label, val in [("I Feed",      api.PidDatSel.I_FEED),       ("Q Feed",  api.PidDatSel.Q_FEED),
                           ("ADC A",       api.PidDatSel.ADC_A),        ("ADC B",   api.PidDatSel.ADC_B),
                           ("FIR Out",     api.PidDatSel.FIR_OUT),
                           ("IQ Demod",    api.PidDatSel.IQ_DEMOD_OUT)]:
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


# ── IQ Demod panel ────────────────────────────────────────────────────────────

class _DemodPanel(_Panel):
    def __init__(self, parent, app):
        super().__init__(parent, app, "IQ Demod")
        self._build()

    def _build(self) -> None:
        ref_frm = ttk.LabelFrame(self, text="Reference", padding=6)
        ref_frm.pack(fill=tk.X, pady=(0, 6))
        self._ref_sel = tk.IntVar(value=int(api.DemodRefSel.NCO1))
        for label, val in [("NCO 1", api.DemodRefSel.NCO1), ("NCO 2", api.DemodRefSel.NCO2)]:
            ttk.Radiobutton(ref_frm, text=label, variable=self._ref_sel,
                            value=int(val)).pack(side=tk.LEFT, padx=2)

        in_frm = ttk.LabelFrame(self, text="Input", padding=6)
        in_frm.pack(fill=tk.X, pady=(0, 6))
        self._in_sel = tk.IntVar(value=int(api.DemodInSel.NCO2))
        for label, val in [("NCO 1",   api.DemodInSel.NCO1),   ("NCO 2",   api.DemodInSel.NCO2),
                           ("ADC A",   api.DemodInSel.ADC_A),  ("ADC B",   api.DemodInSel.ADC_B),
                           ("I Feed",  api.DemodInSel.I_FEED), ("Q Feed",  api.DemodInSel.Q_FEED),
                           ("FIR Out", api.DemodInSel.FIR_OUT)]:
            ttk.Radiobutton(in_frm, text=label, variable=self._in_sel,
                            value=int(val)).pack(side=tk.LEFT, padx=2)

        self._apply_btn = ttk.Button(self, text="Apply", command=self._on_apply)
        self._apply_btn.pack(anchor=tk.W, pady=(4, 2))
        self._fb_var = tk.StringVar(value="")
        ttk.Label(self, textvariable=self._fb_var, foreground="gray").pack(anchor=tk.W)

    def _on_apply(self) -> None:
        ref = api.DemodRefSel(self._ref_sel.get())
        inp = api.DemodInSel(self._in_sel.get())
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._apply_btn)
        def on_ok(r):
            self._fb_var.set(
                f"ref={api.DemodRefSel(r.ref_sel_cb).name}  "
                f"in={api.DemodInSel(r.in_sel_cb).name}"
            )
            self._unbusy(self._apply_btn, "Apply")
            self.ok("IQ demod configured")
        def on_err(e):
            self._unbusy(self._apply_btn, "Apply")
            self.err(e)
        self.app.run_in_bg(lambda: api.api_config_demod(ref, inp), on_ok, on_err)


# ── FIR panel ─────────────────────────────────────────────────────────────────

class _FIRPanel(_Panel):
    def __init__(self, parent, app):
        super().__init__(parent, app, "FIR")
        self._build()

    def _build(self) -> None:
        in_frm = ttk.LabelFrame(self, text="Input", padding=6)
        in_frm.pack(fill=tk.X, pady=(0, 6))
        self._input_sel = tk.IntVar(value=int(api.FirInputSel.ADC1))
        for label, val in [("ADC 1",   api.FirInputSel.ADC1),    ("ADC 2",  api.FirInputSel.ADC2),
                           ("I Feed",  api.FirInputSel.I_FEED),  ("Q Feed", api.FirInputSel.Q_FEED),
                           ("IQ Demod", api.FirInputSel.IQ_DEMOD_OUT)]:
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
            ("Gain [-32, 32):",  "gain",  "1.0"),
            ("Bias [-1V, 1V]:",  "bias",  "0.0"),
            ("Input Gain [-32, 32):", "egain", "1.0"),
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

        ttk.Separator(self, orient=tk.HORIZONTAL).grid(
            row=len(fields)+3, column=0, columnspan=2, sticky=tk.EW, pady=(8, 4))

        ttk.Label(self, text="Metrics Decimation:").grid(
            row=len(fields)+4, column=0, sticky=tk.W)
        self._metrics_dec_var = tk.StringVar(value="1")
        ttk.Entry(self, textvariable=self._metrics_dec_var, width=10).grid(
            row=len(fields)+4, column=1, padx=6, pady=1)

        self._metrics_mass_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(self, text="Show PSD mass", variable=self._metrics_mass_var).grid(
            row=len(fields)+5, column=0, columnspan=2, sticky=tk.W, pady=(4, 0))

        self._metrics_btn = ttk.Button(self, text="Compute Control Metrics",
                                       command=self._on_metrics)
        self._metrics_btn.grid(row=len(fields)+6, column=0, columnspan=2,
                               sticky=tk.W, pady=(6, 0))

    def _on_apply(self) -> None:
        try:
            kp    = float(self._vars["kp"].get())
            ki    = float(self._vars["ki"].get())
            kd    = float(self._vars["kd"].get())
            sp    = float(self._vars["sp"].get())
            dec   = int(self._vars["dec"].get())
            alpha = int(self._vars["alpha"].get())
            sat   = int(self._vars["sat"].get())
            gain  = float(self._vars["gain"].get())
            bias  = float(self._vars["bias"].get())
            egain = float(self._vars["egain"].get())
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
        if not (-32.0 <= gain < 32.0):
            self.err(ValueError(f"gain={gain} not in [-32, 32)")); return
        if not (-1.0 <= bias <= 1.0):
            self.err(ValueError(f"bias={bias} not in [-1.0, 1.0]")); return
        if not (-32.0 <= egain < 32.0):
            self.err(ValueError(f"egain={egain} not in [-32, 32)")); return
        en = int(self._en_var.get())
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._apply_btn)
        def on_ok(r):
            self._fb_var.set(
                f"kp={r.kp_cb:.4f}  ki={r.ki_cb:.4f}  kd={r.kd_cb:.4f}  "
                f"sp={r.sp_cb:.4f}  dec={r.dec_cb}  alpha={r.alpha_cb}  "
                f"sat={r.sat_cb}  en={r.en_cb}  gain={r.gain_cb:.4f}  bias={r.bias_cb:.4f}  egain={r.egain_cb:.4f}"
            )
            self._unbusy(self._apply_btn, "Apply")
            self.ok(f"PID set  kp={r.kp_cb:.4f}  en={r.en_cb}")
        def on_err(e):
            self._unbusy(self._apply_btn, "Apply")
            self.err(e)
        self.app.run_in_bg(
            lambda: api.api_set_pid(kp, kd, ki, sp, dec, alpha, sat, en, gain, bias=bias, egain=egain), on_ok, on_err
        )

    def _on_metrics(self) -> None:
        try:
            dec = int(self._metrics_dec_var.get())
        except ValueError:
            self.err(ValueError(f"Invalid metrics decimation: {self._metrics_dec_var.get()!r}")); return
        if dec < 1:
            self.err(ValueError("Metrics decimation must be >= 1")); return

        # Warn if metrics_dec < PID dec — pid_out PSD will be staircase-dominated or blank
        try:
            pid_dec = int(self._vars["dec"].get())
            if dec < pid_dec:
                import tkinter.messagebox as _mb
                proceed = _mb.askyesno(
                    "PID out PSD warning",
                    f"Metrics decimation ({dec}) < PID decimation ({pid_dec}).\n\n"
                    "pid_out updates only every PID_dec cycles, so the pid_out PSD "
                    "will be staircase-dominated or blank.\n\n"
                    f"Recommended: set metrics decimation ≥ {pid_dec}.\n\n"
                    "Proceed anyway?",
                )
                if not proceed:
                    return
        except ValueError:
            pass  # PID dec field invalid — skip the check

        # Collect PID params from GUI fields (read-only; does not re-issue set_pid)
        pid_params: dict = {}
        for key in ("kp", "ki", "kd", "sp", "dec", "alpha", "sat", "gain", "bias", "egain"):
            try:
                raw = self._vars[key].get()
                pid_params[key] = float(raw) if "." in raw else int(raw)
            except ValueError:
                pid_params[key] = self._vars[key].get()
        pid_params["en"] = int(self._en_var.get())

        show_mass = self._metrics_mass_var.get()
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._metrics_btn, "Capturing…")

        def on_ok(r: api.ControlMetricsResult):
            self._unbusy(self._metrics_btn, "Compute Control Metrics")
            self.ok(f"Metrics done  fs={r.fs/1e6:.3f} MHz  N={r.raw_data.shape[0]}")
            self._show_metrics_figure(r, show_mass)

        def on_err(e):
            self._unbusy(self._metrics_btn, "Compute Control Metrics")
            self.err(e)

        self.app.run_in_bg(lambda: api.api_control_metrics(dec, pid_params), on_ok, on_err)

    def _show_metrics_figure(self, r: api.ControlMetricsResult, show_mass: bool) -> None:
        ts   = datetime.datetime.now().strftime("%H:%M:%S")
        cols = r.columns   # ["pid_in", "err", "pid_out"]

        fig = plt.figure(figsize=(10, 18))
        gs  = fig.add_gridspec(5, 1, height_ratios=[3, 3, 3, 3, 2.2], hspace=0.45)
        psd_axes = [fig.add_subplot(gs[i]) for i in range(3)]
        ax_ccf   = fig.add_subplot(gs[3])
        ax_info  = fig.add_subplot(gs[4])

        fig.suptitle(
            f"Control Metrics  dec={r.decimation}  fs={r.fs/1e6:.3f} MHz  {ts}",
            fontsize=PLOT_FONTSIZE,
        )

        # ── PSD subplots ──────────────────────────────────────────────────────
        nyquist_khz = r.fs / 2e3
        for i, (ax, col) in enumerate(zip(psd_axes, cols)):
            psd_col  = r.psd[:, i]
            freq_khz = r.freqs / 1e3
            ax.semilogy(freq_khz, psd_col, linewidth=PLOT_LINEWIDTH)
            ax.set_ylabel(f"{col}\n(cts²/Hz)", fontsize=PLOT_FONTSIZE)
            ax.tick_params(labelsize=PLOT_FONTSIZE * 0.85)
            ax.grid(True, alpha=0.3, which="both")
            ax.set_xlim(0, nyquist_khz)

            # Centre-of-mass marker
            com_khz = float(np.sum(freq_khz * psd_col) / np.sum(psd_col))
            ymin = ax.get_ylim()[0]
            ax.plot(com_khz, ymin, "o", color="orange", markersize=8,
                    clip_on=False, zorder=5)

            if show_mass:
                mass = float(np.trapezoid(psd_col, r.freqs))
                ax.text(
                    0.98, 0.97, f"mass = {mass:.3g} cts²",
                    transform=ax.transAxes, ha="right", va="top",
                    fontsize=PLOT_FONTSIZE * 0.7,
                    bbox=dict(boxstyle="round", fc="white", alpha=0.7),
                )

        psd_axes[-1].set_xlabel("Frequency (kHz)", fontsize=PLOT_FONTSIZE)
        psd_axes[-1].tick_params(labelsize=PLOT_FONTSIZE * 0.85)

        # Shared x-axis between PSD plots
        for ax in psd_axes[:-1]:
            ax.sharex(psd_axes[-1])
            ax.tick_params(labelbottom=False)

        # ── Cross-correlation subplot ──────────────────────────────────────────
        lags_ms = r.ccf_lags * 1e3  # seconds → ms
        ax_ccf.plot(lags_ms, r.ccf, linewidth=PLOT_LINEWIDTH)
        ax_ccf.axhline(0, color="gray", linewidth=0.6, linestyle="--")
        ax_ccf.axvline(r.ccf_peak_lag * 1e3, color="red", linewidth=PLOT_LINEWIDTH,
                       linestyle="--",
                       label=f"peak={r.ccf_peak:.3f}  lag={r.ccf_peak_lag*1e3:.3f} ms")
        ax_ccf.set_ylabel("CCF\npid_out vs err", fontsize=PLOT_FONTSIZE)
        ax_ccf.set_xlabel("Lag (ms)", fontsize=PLOT_FONTSIZE)
        ax_ccf.tick_params(labelsize=PLOT_FONTSIZE * 0.85)
        ax_ccf.legend(fontsize=PLOT_FONTSIZE * 0.75)
        ax_ccf.grid(True, alpha=0.3)

        # ── Info text box ─────────────────────────────────────────────────────
        ax_info.axis("off")
        p  = r.pid_params
        st = f"{r.settling_time*1e3:.3f} ms" if not (r.settling_time != r.settling_time) else "N/A"
        info = (
            f"{'Controller Parameters':<40}{'Metrics':}\n"
            f"{'':-<70}\n"
            f"{'Kp = ' + str(p.get('kp','?')):<20}"
            f"{'Ki = ' + str(p.get('ki','?')):<20}"
            f"{'RMS Error     = ' + f'{r.rms_error:.2f}':} cts\n"
            f"{'Kd = ' + str(p.get('kd','?')):<20}"
            f"{'SP = ' + str(p.get('sp','?')):<20}"
            f"{'Settling Time = ' + st:}\n"
            f"{'Dec = ' + str(p.get('dec','?')):<20}"
            f"{'α = ' + str(p.get('alpha','?')):<20}"
            f"{'Overshoot     = ' + f'{r.overshoot:.2f}':} %\n"
            f"{'Sat = ' + str(p.get('sat','?')):<20}"
            f"{'Gain = ' + str(p.get('gain','?')):<20}"
            f"{'Ctrl RMS      = ' + f'{r.ctrl_rms:.1f}':} cts\n"
            f"{'Bias = ' + str(p.get('bias','?')):<20}"
            f"{'En = ' + str(bool(p.get('en',0))):<20}"
            f"{'Ctrl Max      = ' + f'{r.ctrl_max:.1f}':} cts\n"
            f"{'':<40}"
            f"{'Ctrl P95      = ' + f'{r.ctrl_p95:.1f}':} cts\n"
            f"{'':<40}"
            f"{'Ctrl Slew RMS = ' + f'{r.ctrl_slew_rms:.2f}':} cts/smp\n"
            f"{'':<40}"
            f"{'CCF Peak      = ' + f'{r.ccf_peak:.3f}':} (correction quality)\n"
            f"{'':<40}"
            f"{'CCF Peak Lag  = ' + f'{r.ccf_peak_lag*1e3:.3f}':} ms"
        )
        ax_info.text(
            0.02, 0.95, info,
            transform=ax_info.transAxes,
            ha="left", va="top",
            fontsize=PLOT_FONTSIZE * 0.6,
            fontfamily="monospace",
        )

        # ── Export data ───────────────────────────────────────────────────────
        psd_csv     = np.column_stack([r.freqs] + [r.psd[:, i] for i in range(3)])
        psd_cols    = ["freq_hz"] + [f"{c}_psd" for c in cols]
        def _safe(v):
            """Convert float to None if NaN (JSON has no NaN literal)."""
            if isinstance(v, float) and v != v:
                return None
            return v

        json_payload = {
            "controller_params": {k: (float(v) if isinstance(v, (int, float)) else v)
                                   for k, v in r.pid_params.items()},
            "metrics": {
                "rms_error_cts":                _safe(r.rms_error),
                "settling_time_s":              _safe(r.settling_time),
                "overshoot_pct":                _safe(r.overshoot),
                "ctrl_rms_cts":                 _safe(r.ctrl_rms),
                "ctrl_max_cts":                 _safe(r.ctrl_max),
                "ctrl_p95_cts":                 _safe(r.ctrl_p95),
                "ctrl_slew_rms_cts_per_sample": _safe(r.ctrl_slew_rms),
            },
            "decimation":     r.decimation,
            "sample_rate_hz": r.fs,
        }

        _add_figure_menu(
            fig,
            csv_data=r.raw_data, csv_columns=cols,
            extra_csv_data=psd_csv, extra_csv_columns=psd_cols,
            extra_csv_label="Save PSD CSV…",
            json_data=json_payload,
        )
        plt.show(block=False)


# ── Frame Capture panel ───────────────────────────────────────────────────────

class _FrameCapturePanel(_Panel):
    def __init__(self, parent, app):
        super().__init__(parent, app, "Frame Capture")
        self._build()

    def _build(self) -> None:
        ttk.Label(self, text="Frame code:").grid(row=0, column=0, sticky=tk.W)
        self._fc_var = tk.StringVar(value=api.FrameCode.ADC_DATA_IN.name)
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
        fig.suptitle(f"{fc.name}  dec={dec}  {ts}", fontsize=PLOT_FONTSIZE)

        for i, ax in enumerate(axes[:, 0]):
            if data.ndim == 2 and i < data.shape[1]:
                y        = data[:, i]
                col_name = cols[i] if i < len(cols) else f"col{i}"
            elif data.ndim == 1 and i == 0:
                y        = data
                col_name = cols[0] if cols else "data"
            else:
                continue
            ax.plot(y, linewidth=PLOT_LINEWIDTH)
            ax.set_ylabel(col_name, fontsize=PLOT_FONTSIZE)
            ax.set_xlabel("sample" if i == n_cols - 1 else "", fontsize=PLOT_FONTSIZE)
            ax.tick_params(labelsize=PLOT_FONTSIZE * 0.85)
            ax.grid(True, alpha=0.3)

        fig.tight_layout()
        csv_data = data if data.ndim == 2 else data.reshape(-1, 1)
        _add_figure_menu(fig, csv_data, cols)
        plt.show(block=False)


# ── Sweep Ramp panel ──────────────────────────────────────────────────────────

class _SweepRampPanel(_Panel):
    def __init__(self, parent, app):
        super().__init__(parent, app, "Sweep Ramp")
        self._build()

    def _build(self) -> None:
        dac_frm = ttk.LabelFrame(self, text="DAC", padding=6)
        dac_frm.pack(fill=tk.X, pady=(0, 6))
        self._dac_sel = tk.IntVar(value=int(api.DacSel.DAC_1))
        ttk.Radiobutton(dac_frm, text="DAC 1", variable=self._dac_sel,
                        value=int(api.DacSel.DAC_1)).pack(side=tk.LEFT, padx=4)
        ttk.Radiobutton(dac_frm, text="DAC 2", variable=self._dac_sel,
                        value=int(api.DacSel.DAC_2)).pack(side=tk.LEFT, padx=4)

        rng_frm = ttk.Frame(self)
        rng_frm.pack(fill=tk.X, pady=(0, 4))
        ttk.Label(rng_frm, text="V0:").grid(row=0, column=0, sticky=tk.W)
        self._v0_var = tk.StringVar(value="-1.0")
        ttk.Entry(rng_frm, textvariable=self._v0_var, width=8).grid(row=0, column=1, padx=4)
        ttk.Label(rng_frm, text="V1:").grid(row=0, column=2, sticky=tk.W, padx=(8, 0))
        self._v1_var = tk.StringVar(value="1.0")
        ttk.Entry(rng_frm, textvariable=self._v1_var, width=8).grid(row=0, column=3, padx=4)

        ttk.Label(rng_frm, text="Points:").grid(row=1, column=0, sticky=tk.W, pady=(4, 0))
        self._pts_var = tk.StringVar(value="1000")
        ttk.Entry(rng_frm, textvariable=self._pts_var, width=8).grid(row=1, column=1, padx=4, pady=(4, 0))
        ttk.Label(rng_frm, text="Delay µs:").grid(row=1, column=2, sticky=tk.W, padx=(8, 0), pady=(4, 0))
        self._delay_var = tk.StringVar(value="0")
        ttk.Entry(rng_frm, textvariable=self._delay_var, width=8).grid(row=1, column=3, padx=4, pady=(4, 0))

        plot_frm = ttk.LabelFrame(self, text="Plot", padding=6)
        plot_frm.pack(fill=tk.X, pady=(0, 6))
        self._plot_adc_a = tk.BooleanVar(value=True)
        self._plot_adc_b = tk.BooleanVar(value=True)
        self._plot_i     = tk.BooleanVar(value=True)
        self._plot_q     = tk.BooleanVar(value=True)
        ttk.Checkbutton(plot_frm, text="ADC A", variable=self._plot_adc_a).pack(side=tk.LEFT, padx=4)
        ttk.Checkbutton(plot_frm, text="ADC B", variable=self._plot_adc_b).pack(side=tk.LEFT, padx=4)
        ttk.Checkbutton(plot_frm, text="I Feed", variable=self._plot_i).pack(side=tk.LEFT, padx=4)
        ttk.Checkbutton(plot_frm, text="Q Feed", variable=self._plot_q).pack(side=tk.LEFT, padx=4)

        self._run_btn = ttk.Button(self, text="Run Sweep", command=self._on_run)
        self._run_btn.pack(anchor=tk.W, pady=(2, 0))

    def _on_run(self) -> None:
        try:
            v0    = float(self._v0_var.get())
            v1    = float(self._v1_var.get())
            pts   = int(self._pts_var.get())
            delay = int(self._delay_var.get())
        except ValueError as e:
            self.err(e); return
        if not (-1.0 <= v0 <= 1.0 and -1.0 <= v1 <= 1.0):
            self.err(ValueError("V0 and V1 must be in [-1.0, 1.0]")); return
        if not (2 <= pts <= 16384):
            self.err(ValueError("Points must be in [2, 16384]")); return
        if delay < 0:
            self.err(ValueError("Delay must be >= 0")); return

        dac = api.DacSel(self._dac_sel.get())
        active_cols = [
            col for col, var in zip(
                ["adc_a", "adc_b", "i_feed", "q_feed"],
                [self._plot_adc_a, self._plot_adc_b, self._plot_i, self._plot_q],
            ) if var.get()
        ]

        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._run_btn, "Running…")

        def on_ok(r: api.SweepRampResult):
            self._unbusy(self._run_btn, "Run Sweep")
            self.ok(f"Sweep done  {r.num_points_cb} pts")
            if active_cols and r.data.size > 0:
                self._plot_sweep(r, active_cols)

        def on_err(e):
            self._unbusy(self._run_btn, "Run Sweep")
            self.err(e)

        self.app.run_in_bg(
            lambda: api.api_sweep_ramp(v0, v1, pts, dac, write_delay_us=delay),
            on_ok, on_err,
        )

    def _plot_sweep(self, r: api.SweepRampResult, active_cols: list[str]) -> None:
        x = r.data[:, 0]  # dac_v
        n = len(active_cols)
        fig, axes = plt.subplots(n, 1, figsize=(10, 2.5 * n), squeeze=False, sharex=True)
        for i, col in enumerate(active_cols):
            idx = api.SWEEP_COLUMNS.index(col)
            axes[i, 0].plot(x, r.data[:, idx], linewidth=PLOT_LINEWIDTH)
            axes[i, 0].set_ylabel(col, fontsize=PLOT_FONTSIZE)
            axes[i, 0].tick_params(labelsize=PLOT_FONTSIZE * 0.85)
            axes[i, 0].grid(True, alpha=0.3)
        axes[-1, 0].set_xlabel("DAC voltage (V)", fontsize=PLOT_FONTSIZE)
        axes[-1, 0].tick_params(labelsize=PLOT_FONTSIZE * 0.85)
        fig.tight_layout()
        _add_figure_menu(fig, r.data, list(api.SWEEP_COLUMNS))
        plt.show(block=False)


# ── Autolock panel ────────────────────────────────────────────────────────────

_CHAN_TO_PID_SEL = {
    "adc_a":  api.PidDatSel.ADC_A,
    "adc_b":  api.PidDatSel.ADC_B,
    "i_feed": api.PidDatSel.I_FEED,
    "q_feed": api.PidDatSel.Q_FEED,
}


class _AutolockPanel(_Panel):
    def __init__(self, parent, app):
        super().__init__(parent, app, "Autolock")
        self._persist_running  = False
        self._persist_busy     = False
        self._persist_after_id = None
        self._build()

    def _build(self) -> None:
        # Sweep parameters
        rng_frm = ttk.LabelFrame(self, text="Sweep", padding=6)
        rng_frm.pack(fill=tk.X, pady=(0, 6))
        ttk.Label(rng_frm, text="V0:").grid(row=0, column=0, sticky=tk.W)
        self._v0_var = tk.StringVar(value="-1.0")
        ttk.Entry(rng_frm, textvariable=self._v0_var, width=8).grid(row=0, column=1, padx=4)
        ttk.Label(rng_frm, text="V1:").grid(row=0, column=2, sticky=tk.W, padx=(8, 0))
        self._v1_var = tk.StringVar(value="1.0")
        ttk.Entry(rng_frm, textvariable=self._v1_var, width=8).grid(row=0, column=3, padx=4)
        ttk.Label(rng_frm, text="Points:").grid(row=1, column=0, sticky=tk.W, pady=(4, 0))
        self._pts_var = tk.StringVar(value="1000")
        ttk.Entry(rng_frm, textvariable=self._pts_var, width=8).grid(row=1, column=1, padx=4, pady=(4, 0))
        ttk.Label(rng_frm, text="Delay µs:").grid(row=1, column=2, sticky=tk.W, padx=(8, 0), pady=(4, 0))
        self._delay_var = tk.StringVar(value="0")
        ttk.Entry(rng_frm, textvariable=self._delay_var, width=8).grid(row=1, column=3, padx=4, pady=(4, 0))

        # Channel selection
        chan_frm = ttk.LabelFrame(self, text="Channel", padding=6)
        chan_frm.pack(fill=tk.X, pady=(0, 6))
        self._chan_var = tk.StringVar(value="i_feed")
        for label, val in [("ADC A", "adc_a"), ("ADC B", "adc_b"),
                           ("I Feed", "i_feed"), ("Q Feed", "q_feed")]:
            ttk.Radiobutton(chan_frm, text=label, variable=self._chan_var,
                            value=val).pack(side=tk.LEFT, padx=2)

        # Peak window
        pk_frm = ttk.LabelFrame(self, text="Peak Window", padding=6)
        pk_frm.pack(fill=tk.X, pady=(0, 6))
        ttk.Label(pk_frm, text="Peak sweep pts:").pack(side=tk.LEFT)
        self._peak_pts_var = tk.StringVar(value="200")
        ttk.Entry(pk_frm, textvariable=self._peak_pts_var, width=8).pack(side=tk.LEFT, padx=4)

        # Results
        res_frm = ttk.LabelFrame(self, text="Result", padding=6)
        res_frm.pack(fill=tk.X, pady=(0, 6))
        self._al_lp_var = tk.StringVar(value="Lock point: —")
        ttk.Label(res_frm, textvariable=self._al_lp_var).pack(anchor=tk.W)
        self._al_slope_var = tk.StringVar(value="Slope: —")
        ttk.Label(res_frm, textvariable=self._al_slope_var).pack(anchor=tk.W)

        # Persistent autolock
        persist_frm = ttk.LabelFrame(self, text="Persistent Autolock", padding=6)
        persist_frm.pack(fill=tk.X, pady=(0, 6))
        ttk.Label(persist_frm, text="Period (ms):").pack(side=tk.LEFT)
        self._persist_ms_var = tk.StringVar(value="1000")
        ttk.Entry(persist_frm, textvariable=self._persist_ms_var, width=8).pack(side=tk.LEFT, padx=4)
        self._persist_status_var = tk.StringVar(value="Stopped")
        ttk.Label(persist_frm, textvariable=self._persist_status_var,
                  foreground="gray").pack(side=tk.LEFT, padx=(8, 0))

        btn_frm = ttk.Frame(self)
        btn_frm.pack(anchor=tk.W, pady=(2, 0))
        self._al_btn = ttk.Button(btn_frm, text="Run Autolock", command=self._on_autolock)
        self._al_btn.pack(side=tk.LEFT, padx=(0, 6))
        self._persist_btn = ttk.Button(btn_frm, text="Start Persistent",
                                       command=self._on_persist_toggle)
        self._persist_btn.pack(side=tk.LEFT)

    def _on_autolock(self) -> None:
        try:
            v0       = float(self._v0_var.get())
            v1       = float(self._v1_var.get())
            pts      = int(self._pts_var.get())
            delay    = int(self._delay_var.get())
            peak_pts = int(self._peak_pts_var.get())
        except ValueError as e:
            self.err(e); return
        if not (-1.0 <= v0 <= 1.0 and -1.0 <= v1 <= 1.0):
            self.err(ValueError("V0 and V1 must be in [-1.0, 1.0]")); return
        if not (2 <= pts <= 16384):
            self.err(ValueError("Points must be in [2, 16384]")); return
        if delay < 0:
            self.err(ValueError("Delay must be >= 0")); return
        if peak_pts < 1:
            self.err(ValueError("Peak sweep pts must be >= 1")); return

        chan = self._chan_var.get()
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._al_btn, "Locking…")

        def on_ok(result):
            r, chan, lock_point, slope, egain, pid_sel = result
            self._al_lp_var.set(f"Lock point: {lock_point:.6f} V")
            self._al_slope_var.set(f"Slope: {slope:.6f} V⁻¹")
            # Sync IO routing panel UI
            self.app._io_panel._dac1_sel.set(int(api.DacDatSel.PID))
            self.app._io_panel._pid_sel.set(int(pid_sel))
            # Sync PID panel UI
            self.app._pid_panel._vars["sp"].set("0.0")
            self.app._pid_panel._vars["bias"].set(f"{lock_point:.6f}")
            self.app._pid_panel._vars["egain"].set(f"{egain:.6f}")
            self.app._pid_panel._en_var.set(True)
            self._unbusy(self._al_btn, "Run Autolock")
            self.ok(f"Locked  lp={lock_point:.4f} V  slope={slope:.4f}")
            self._plot_autolock(r, chan, lock_point)

        def on_err(e):
            self._unbusy(self._al_btn, "Run Autolock")
            self.err(e)

        self.app.run_in_bg(
            lambda: self._autolock_bg(v0, v1, pts, delay, peak_pts, chan),
            on_ok, on_err,
        )

    def _autolock_bg(self, v0, v1, pts, delay, peak_pts, chan):
        """Core autolock sequence — runs in a background thread.
        Returns (r, chan, lock_point, slope, egain, pid_sel)."""
        # 1. Sweep
        r = api.api_sweep_ramp(v0, v1, pts, api.DacSel.DAC_1,
                               write_delay_us=delay)
        # 2. Extract columns
        col_idx = api.SWEEP_COLUMNS.index(chan)
        sig     = r.data[:, col_idx]
        dac_v   = r.data[:, 0]

        # 3. Find global max; window ±peak_pts to find min
        max_idx = int(np.argmax(sig))
        lo      = max(0, max_idx - peak_pts)
        hi      = min(len(sig), max_idx + peak_pts + 1)
        min_idx = lo + int(np.argmin(sig[lo:hi]))

        if abs(dac_v[max_idx] - dac_v[min_idx]) < 1e-9:
            raise ValueError("Max and min are at the same DAC voltage")

        # 4. Find lock point as the point of steepest slope between max and min
        seg_lo   = min(max_idx, min_idx)
        seg_hi   = max(max_idx, min_idx) + 1
        seg_sig  = sig[seg_lo:seg_hi]
        seg_dac  = dac_v[seg_lo:seg_hi]
        if len(seg_sig) < 2:
            raise ValueError("Feature segment too short to compute slope")
        deriv      = np.gradient(seg_sig, seg_dac)
        lp_seg_idx = int(np.argmax(np.abs(deriv)))
        lock_point = float(seg_dac[lp_seg_idx])
        slope      = float(deriv[lp_seg_idx])
        if abs(slope) < 1e-9:
            raise ValueError("Slope is zero — no dispersive feature found")
        egain = 100.0 / slope

        # 5. Validate egain range
        if abs(egain) >= 32.0:
            raise ValueError(
                f"100/slope = {egain:.4f} is outside egain range [-32, 32). "
                "Increase sweep range or signal amplitude."
            )

        # 6. Configure IO routing: DAC1→PID, PID_in→selected channel
        pid_sel = _CHAN_TO_PID_SEL[chan]
        dac2    = api.DacDatSel(self.app._io_panel._dac2_sel.get())
        api.api_config_io(api.DacDatSel.PID, dac2, pid_sel)

        # 7. Configure PID: read current params, override sp/bias/egain/en
        pv = self.app._pid_panel._vars
        kp    = float(pv["kp"].get())
        ki    = float(pv["ki"].get())
        kd    = float(pv["kd"].get())
        dec   = int(pv["dec"].get())
        alpha = int(pv["alpha"].get())
        sat   = int(pv["sat"].get())
        gain  = float(pv["gain"].get())
        api.api_set_pid(kp, kd, ki, 0.0, dec, alpha, sat, 1,
                        gain=gain, bias=lock_point, egain=egain)

        return (r, chan, lock_point, slope, egain, pid_sel)

    def _on_persist_toggle(self) -> None:
        if not self._persist_running:
            try:
                ms = int(self._persist_ms_var.get())
            except ValueError:
                self.err(ValueError("Period must be an integer (ms)")); return
            if ms < 100:
                self.err(ValueError("Period must be >= 100 ms")); return
            self._persist_running = True
            self._persist_btn.configure(text="Stop Persistent")
            self._persist_status_var.set("Running")
            self._persist_tick()
        else:
            self._persist_running = False
            if self._persist_after_id is not None:
                self.after_cancel(self._persist_after_id)
                self._persist_after_id = None
            self._persist_btn.configure(text="Start Persistent")
            self._persist_status_var.set("Stopped")

    def _persist_tick(self) -> None:
        if not self._persist_running:
            return
        try:
            ms = int(self._persist_ms_var.get())
        except ValueError:
            ms = 1000
        if self._persist_busy:
            self._persist_after_id = self.after(ms, self._persist_tick)
            return
        try:
            v0       = float(self._v0_var.get())
            v1       = float(self._v1_var.get())
            pts      = int(self._pts_var.get())
            delay    = int(self._delay_var.get())
            peak_pts = int(self._peak_pts_var.get())
        except ValueError as e:
            self.err(e)
            self._persist_running = False
            self._persist_btn.configure(text="Start Persistent")
            self._persist_status_var.set("Stopped (invalid params)")
            return

        chan = self._chan_var.get()
        self._prep_call(*self._conn())
        self._persist_busy = True

        def on_ok(result):
            _r, _chan, lock_point, slope, egain, pid_sel = result
            self._al_lp_var.set(f"Lock point: {lock_point:.6f} V")
            self._al_slope_var.set(f"Slope: {slope:.6f} V⁻¹")
            self.app._io_panel._dac1_sel.set(int(api.DacDatSel.PID))
            self.app._io_panel._pid_sel.set(int(pid_sel))
            self.app._pid_panel._vars["sp"].set("0.0")
            self.app._pid_panel._vars["bias"].set(f"{lock_point:.6f}")
            self.app._pid_panel._vars["egain"].set(f"{egain:.6f}")
            self.app._pid_panel._en_var.set(True)
            self.ok(f"[Persist] lp={lock_point:.4f} V  slope={slope:.4f}")
            self._persist_busy = False
            if self._persist_running:
                self._persist_after_id = self.after(ms, self._persist_tick)

        def on_err(e):
            self.err(e)
            self._persist_busy = False
            if self._persist_running:
                self._persist_after_id = self.after(ms, self._persist_tick)

        self.app.run_in_bg(
            lambda: self._autolock_bg(v0, v1, pts, delay, peak_pts, chan),
            on_ok, on_err,
        )

    def _plot_autolock(self, r: api.SweepRampResult, locked_chan: str,
                       lock_point: float) -> None:
        all_cols = ["adc_a", "adc_b", "i_feed", "q_feed"]
        x = r.data[:, 0]  # dac_v
        n = len(all_cols)
        fig, axes = plt.subplots(n, 1, figsize=(10, 2.5 * n), squeeze=False, sharex=True)
        for i, col in enumerate(all_cols):
            idx = api.SWEEP_COLUMNS.index(col)
            axes[i, 0].plot(x, r.data[:, idx], linewidth=PLOT_LINEWIDTH)
            axes[i, 0].set_ylabel(col, fontsize=PLOT_FONTSIZE)
            axes[i, 0].tick_params(labelsize=PLOT_FONTSIZE * 0.85)
            axes[i, 0].grid(True, alpha=0.3)
            if col == locked_chan:
                axes[i, 0].axvline(lock_point, color="red", linestyle="--",
                                   linewidth=PLOT_LINEWIDTH * 1.25,
                                   label=f"lock={lock_point:.4f} V")
                axes[i, 0].legend(fontsize=PLOT_FONTSIZE * 0.85)
        axes[-1, 0].set_xlabel("DAC voltage (V)", fontsize=PLOT_FONTSIZE)
        axes[-1, 0].tick_params(labelsize=PLOT_FONTSIZE * 0.85)
        fig.tight_layout()
        _add_figure_menu(fig, r.data, list(api.SWEEP_COLUMNS))
        plt.show(block=False)


# ── PSD panel ─────────────────────────────────────────────────────────────────

class _PSDPanel(_Panel):
    def __init__(self, parent, app):
        super().__init__(parent, app, "PSD")
        self._build()

    def _build(self) -> None:
        plot_frm = ttk.LabelFrame(self, text="Channels", padding=6)
        plot_frm.pack(fill=tk.X, pady=(0, 6))

        # Mode radio buttons
        self._psd_mode = tk.StringVar(value="adc")
        ttk.Radiobutton(plot_frm, text="ADC", variable=self._psd_mode,
                        value="adc", command=self._on_mode_change).pack(side=tk.LEFT, padx=(0, 2))

        # ADC channel checkboxes
        self._plot_adc_a = tk.BooleanVar(value=True)
        self._plot_adc_b = tk.BooleanVar(value=True)
        self._plot_i     = tk.BooleanVar(value=True)
        self._plot_q     = tk.BooleanVar(value=True)
        self._cb_adc_a = ttk.Checkbutton(plot_frm, text="ADC A", variable=self._plot_adc_a)
        self._cb_adc_a.pack(side=tk.LEFT, padx=2)
        self._cb_adc_b = ttk.Checkbutton(plot_frm, text="ADC B", variable=self._plot_adc_b)
        self._cb_adc_b.pack(side=tk.LEFT, padx=2)
        self._cb_i = ttk.Checkbutton(plot_frm, text="I Feed", variable=self._plot_i)
        self._cb_i.pack(side=tk.LEFT, padx=2)
        self._cb_q = ttk.Checkbutton(plot_frm, text="Q Feed", variable=self._plot_q)
        self._cb_q.pack(side=tk.LEFT, padx=2)

        ttk.Separator(plot_frm, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=6)

        ttk.Radiobutton(plot_frm, text="PID", variable=self._psd_mode,
                        value="pid", command=self._on_mode_change).pack(side=tk.LEFT, padx=(0, 2))

        # PID channel checkboxes
        self._plot_pid_in  = tk.BooleanVar(value=True)
        self._plot_err     = tk.BooleanVar(value=True)
        self._plot_pid_out = tk.BooleanVar(value=True)
        self._cb_pid_in  = ttk.Checkbutton(plot_frm, text="PID In",  variable=self._plot_pid_in)
        self._cb_pid_in.pack(side=tk.LEFT, padx=2)
        self._cb_err     = ttk.Checkbutton(plot_frm, text="Error",   variable=self._plot_err)
        self._cb_err.pack(side=tk.LEFT, padx=2)
        self._cb_pid_out = ttk.Checkbutton(plot_frm, text="PID Out", variable=self._plot_pid_out)
        self._cb_pid_out.pack(side=tk.LEFT, padx=2)

        ttk.Separator(plot_frm, orient=tk.VERTICAL).pack(side=tk.LEFT, fill=tk.Y, padx=6)
        self._show_mass = tk.BooleanVar(value=False)
        ttk.Checkbutton(plot_frm, text="Show mass", variable=self._show_mass).pack(side=tk.LEFT, padx=4)

        self._on_mode_change()

        params_frm = ttk.Frame(self)
        params_frm.pack(fill=tk.X, pady=(0, 4))

        ttk.Label(params_frm, text="Decimation:").grid(row=0, column=0, sticky=tk.W)
        self._dec_var = tk.StringVar(value="1")
        ttk.Entry(params_frm, textvariable=self._dec_var, width=8).grid(row=0, column=1, padx=4)

        ttk.Label(params_frm, text="f start (Hz):").grid(row=1, column=0, sticky=tk.W, pady=(4, 0))
        self._fstart_var = tk.StringVar(value="0")
        ttk.Entry(params_frm, textvariable=self._fstart_var, width=10).grid(row=1, column=1, padx=4, pady=(4, 0))

        ttk.Label(params_frm, text="f stop (Hz):").grid(row=2, column=0, sticky=tk.W, pady=(4, 0))
        self._fstop_var = tk.StringVar(value="5000000")
        ttk.Entry(params_frm, textvariable=self._fstop_var, width=10).grid(row=2, column=1, padx=4, pady=(4, 0))

        self._run_btn = ttk.Button(self, text="Compute PSD", command=self._on_run)
        self._run_btn.pack(anchor=tk.W, pady=(2, 0))

    def _on_mode_change(self) -> None:
        if self._psd_mode.get() == "adc":
            adc_state = tk.NORMAL
            pid_state = tk.DISABLED
        else:
            adc_state = tk.DISABLED
            pid_state = tk.NORMAL
        for cb in (self._cb_adc_a, self._cb_adc_b, self._cb_i, self._cb_q):
            cb.configure(state=adc_state)
        for cb in (self._cb_pid_in, self._cb_err, self._cb_pid_out):
            cb.configure(state=pid_state)

    def _on_run(self) -> None:
        try:
            dec    = int(self._dec_var.get())
            fstart = float(self._fstart_var.get())
            fstop  = float(self._fstop_var.get())
        except ValueError as e:
            self.err(e); return
        if dec < 1:
            self.err(ValueError("Decimation must be >= 1")); return
        if not (0 <= fstart < fstop):
            self.err(ValueError("Require 0 <= f start < f stop")); return

        mode = self._psd_mode.get()
        if mode == "adc":
            frame_code = api.FrameCode.ADC_DATA_IN
            active_cols = [
                col for col, var in zip(
                    ["adc_a", "adc_b", "i_feed", "q_feed"],
                    [self._plot_adc_a, self._plot_adc_b, self._plot_i, self._plot_q],
                ) if var.get()
            ]
        else:
            frame_code = api.FrameCode.PID_IO
            active_cols = [
                col for col, var in zip(
                    ["pid_in", "err", "pid_out"],
                    [self._plot_pid_in, self._plot_err, self._plot_pid_out],
                ) if var.get()
            ]
        if not active_cols:
            self.err(ValueError("Select at least one channel")); return

        nyquist = api.FPGA_FS / (2 * dec)
        if fstop > nyquist:
            self.ok(f"Warning: f stop {fstop/1e6:.3f} MHz exceeds Nyquist {nyquist/1e6:.3f} MHz")

        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._run_btn, "Capturing…")
        show_mass = self._show_mass.get()

        def on_ok(r: api.PSDResult):
            self._unbusy(self._run_btn, "Compute PSD")
            if r.freqs.size == 0:
                self.err(RuntimeError("PSD capture returned no data")); return
            self.ok(f"PSD done  fs={r.fs/1e6:.3f} MHz  {r.freqs.size} bins")
            self._plot_psd(r, active_cols, fstart, fstop, dec, show_mass)

        def on_err(e):
            self._unbusy(self._run_btn, "Compute PSD")
            self.err(e)

        self.app.run_in_bg(lambda: api.api_psd(dec, frame_code), on_ok, on_err)

    def _plot_psd(self, r: api.PSDResult, active_cols: list[str],
                  fstart: float, fstop: float, dec: int,
                  show_mass: bool = False) -> None:
        mask = (r.freqs >= fstart) & (r.freqs <= fstop)
        freqs_khz = r.freqs[mask] / 1e3
        n   = len(active_cols)
        ts  = datetime.datetime.now().strftime("%H:%M:%S")
        fig, axes = plt.subplots(n, 1, figsize=(10, 2.5 * n), squeeze=False, sharex=True)
        fig.suptitle(f"PSD  dec={dec}  fs={r.fs/1e6:.3f} MHz  {ts}", fontsize=PLOT_FONTSIZE)
        for i, col in enumerate(active_cols):
            idx = r.columns.index(col)
            psd_col = r.psd[mask, idx]
            axes[i, 0].semilogy(freqs_khz, psd_col, linewidth=PLOT_LINEWIDTH)
            axes[i, 0].set_ylabel(f"{col}\n(cts²/Hz)", fontsize=PLOT_FONTSIZE)
            axes[i, 0].tick_params(labelsize=PLOT_FONTSIZE * 0.85)
            axes[i, 0].grid(True, alpha=0.3, which='both')
            com_khz = np.sum(freqs_khz * psd_col) / np.sum(psd_col)
            ymin = axes[i, 0].get_ylim()[0]
            axes[i, 0].plot(com_khz, ymin, 'o', color='orange', markersize=8,
                            clip_on=False, zorder=5)
            if show_mass:
                mass = np.trapezoid(psd_col, r.freqs[mask])
                axes[i, 0].text(
                    0.98, 0.97, f"mass = {mass:.3g} cts²",
                    transform=axes[i, 0].transAxes,
                    ha="right", va="top",
                    fontsize=PLOT_FONTSIZE * 0.7,
                    bbox=dict(boxstyle="round", fc="white", alpha=0.7),
                )
        axes[-1, 0].set_xlabel("Frequency (kHz)", fontsize=PLOT_FONTSIZE)
        axes[-1, 0].tick_params(labelsize=PLOT_FONTSIZE * 0.85)
        fig.tight_layout()
        col_indices = [r.columns.index(col) for col in active_cols]
        csv_data = np.column_stack([r.freqs[mask]] + [r.psd[mask, i] for i in col_indices])
        _add_figure_menu(fig, csv_data, ["freq_hz"] + active_cols,
                         extra_csv_data=r.raw_data,
                         extra_csv_columns=r.columns)
        plt.show(block=False)


# ── Entry point ───────────────────────────────────────────────────────────────

def main() -> None:
    app = _App()
    app.mainloop()


if __name__ == "__main__":
    main()
