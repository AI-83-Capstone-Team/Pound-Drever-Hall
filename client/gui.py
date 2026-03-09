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
                     extra_csv_label: str = "Save DMA CSV…") -> None:
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
        _IORoutingPanel(col0, self).pack(fill=tk.X, pady=(0, 4))
        _SweepRampPanel(col0, self).pack(fill=tk.X)

        # Col 1: NCO, Rotation, FIR
        _NCOPanel(col1, self).pack(fill=tk.X, pady=(0, 4))
        _RotationPanel(col1, self).pack(fill=tk.X, pady=(0, 4))
        _FIRPanel(col1, self).pack(fill=tk.X)

        # Col 2: PID, Frame Capture, PSD
        _PIDPanel(col2, self).pack(fill=tk.X, pady=(0, 4))
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
            ("Gain [-32, 32):",  "gain",  "1.0"),
            ("Bias [-1V, 1V]:",  "bias",  "0.0"),
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
            gain  = float(self._vars["gain"].get())
            bias  = float(self._vars["bias"].get())
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
        en = int(self._en_var.get())
        ip, port = self._conn()
        self._prep_call(ip, port)
        self._busy(self._apply_btn)
        def on_ok(r):
            self._fb_var.set(
                f"kp={r.kp_cb:.4f}  ki={r.ki_cb:.4f}  kd={r.kd_cb:.4f}  "
                f"sp={r.sp_cb:.4f}  dec={r.dec_cb}  alpha={r.alpha_cb}  "
                f"sat={r.sat_cb}  en={r.en_cb}  gain={r.gain_cb:.4f}  bias={r.bias_cb:.4f}"
            )
            self._unbusy(self._apply_btn, "Apply")
            self.ok(f"PID set  kp={r.kp_cb:.4f}  en={r.en_cb}")
        def on_err(e):
            self._unbusy(self._apply_btn, "Apply")
            self.err(e)
        self.app.run_in_bg(
            lambda: api.api_set_pid(kp, kd, ki, sp, dec, alpha, sat, en, gain, bias=bias), on_ok, on_err
        )


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
        self._pts_var = tk.StringVar(value="100")
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

        lp_frm = ttk.LabelFrame(self, text="Lock Point", padding=6)
        lp_frm.pack(fill=tk.X, pady=(0, 6))

        self._compute_lp_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(lp_frm, text="Compute lock point",
                        variable=self._compute_lp_var,
                        command=self._on_lp_toggle).pack(anchor=tk.W)

        opts_frm = ttk.Frame(lp_frm)
        opts_frm.pack(anchor=tk.W, pady=(2, 0))
        ttk.Label(opts_frm, text="Sign:").pack(side=tk.LEFT)
        self._sign_sel_var = tk.StringVar(value="I")
        self._sign_i_rb = ttk.Radiobutton(opts_frm, text="I", variable=self._sign_sel_var, value="I")
        self._sign_q_rb = ttk.Radiobutton(opts_frm, text="Q", variable=self._sign_sel_var, value="Q")
        self._sign_i_rb.pack(side=tk.LEFT, padx=(4, 2))
        self._sign_q_rb.pack(side=tk.LEFT, padx=2)
        self._invert_delta_var = tk.BooleanVar(value=False)
        self._invert_cb = ttk.Checkbutton(opts_frm, text="Invert delta",
                                          variable=self._invert_delta_var)
        self._invert_cb.pack(side=tk.LEFT, padx=(8, 0))

        res_frm = ttk.Frame(lp_frm)
        res_frm.pack(anchor=tk.W, pady=(4, 0))
        ttk.Label(res_frm, text="Angle:").pack(side=tk.LEFT)
        self._lp_angle_var = tk.StringVar(value="—")
        ttk.Label(res_frm, textvariable=self._lp_angle_var, width=12).pack(side=tk.LEFT, padx=(2, 8))
        ttk.Label(res_frm, text="Lock:").pack(side=tk.LEFT)
        self._lp_lock_var = tk.StringVar(value="—")
        ttk.Label(res_frm, textvariable=self._lp_lock_var, width=12).pack(side=tk.LEFT, padx=2)

        # Initialise widget states
        self._on_lp_toggle()

        self._run_btn = ttk.Button(self, text="Run Sweep", command=self._on_run)
        self._run_btn.pack(anchor=tk.W, pady=(2, 0))

    def _on_lp_toggle(self) -> None:
        state = tk.NORMAL if self._compute_lp_var.get() else tk.DISABLED
        self._sign_i_rb.configure(state=state)
        self._sign_q_rb.configure(state=state)
        self._invert_cb.configure(state=state)

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
            lp = None
            if self._compute_lp_var.get() and r.data.size > 0:
                try:
                    lp = api.compute_lockpoint(
                        r.data,
                        sign_sel=self._sign_sel_var.get(),
                        invert_delta=self._invert_delta_var.get(),
                    )
                    self._lp_angle_var.set(f"{lp.optimal_angle_deg:.2f}°")
                    self._lp_lock_var.set(f"{lp.lock_point:.4f} V")
                except Exception as exc:
                    self.err(exc)
            if active_cols and r.data.size > 0:
                self._plot_sweep(r, active_cols, lp)

        def on_err(e):
            self._unbusy(self._run_btn, "Run Sweep")
            self.err(e)

        self.app.run_in_bg(
            lambda: api.api_sweep_ramp(v0, v1, pts, dac, write_delay_us=delay),
            on_ok, on_err,
        )

    def _plot_sweep(self, r: api.SweepRampResult, active_cols: list[str],
                    lp: api.LockPointResult | None = None) -> None:
        x = r.data[:, 0]  # dac_v
        n = len(active_cols) + (1 if lp is not None else 0)
        fig, axes = plt.subplots(n, 1, figsize=(10, 2.5 * n), squeeze=False, sharex=True)
        for i, col in enumerate(active_cols):
            idx = api.SWEEP_COLUMNS.index(col)
            axes[i, 0].plot(x, r.data[:, idx], linewidth=PLOT_LINEWIDTH)
            axes[i, 0].set_ylabel(col, fontsize=PLOT_FONTSIZE)
            axes[i, 0].tick_params(labelsize=PLOT_FONTSIZE * 0.85)
            axes[i, 0].grid(True, alpha=0.3)
        if lp is not None:
            ax_g = axes[len(active_cols), 0]
            ax_g.plot(x, lp.G, linewidth=PLOT_LINEWIDTH)
            ax_g.set_ylabel("G (V)", fontsize=PLOT_FONTSIZE)
            ax_g.tick_params(labelsize=PLOT_FONTSIZE * 0.85)
            ax_g.grid(True, alpha=0.3)
            ax_g.axvline(lp.lock_point, color="red", linestyle="--",
                         linewidth=PLOT_LINEWIDTH * 1.25,
                         label=f"lock={lp.lock_point:.4f} V")
            ax_g.legend(fontsize=PLOT_FONTSIZE * 0.85)
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
        self._plot_adc_a = tk.BooleanVar(value=True)
        self._plot_adc_b = tk.BooleanVar(value=True)
        self._plot_i     = tk.BooleanVar(value=True)
        self._plot_q     = tk.BooleanVar(value=True)
        ttk.Checkbutton(plot_frm, text="ADC A", variable=self._plot_adc_a).pack(side=tk.LEFT, padx=4)
        ttk.Checkbutton(plot_frm, text="ADC B", variable=self._plot_adc_b).pack(side=tk.LEFT, padx=4)
        ttk.Checkbutton(plot_frm, text="I Feed", variable=self._plot_i).pack(side=tk.LEFT, padx=4)
        ttk.Checkbutton(plot_frm, text="Q Feed", variable=self._plot_q).pack(side=tk.LEFT, padx=4)

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

        active_cols = [
            col for col, var in zip(
                ["adc_a", "adc_b", "i_feed", "q_feed"],
                [self._plot_adc_a, self._plot_adc_b, self._plot_i, self._plot_q],
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

        def on_ok(r: api.PSDResult):
            self._unbusy(self._run_btn, "Compute PSD")
            if r.freqs.size == 0:
                self.err(RuntimeError("PSD capture returned no data")); return
            self.ok(f"PSD done  fs={r.fs/1e6:.3f} MHz  {r.freqs.size} bins")
            self._plot_psd(r, active_cols, fstart, fstop, dec)

        def on_err(e):
            self._unbusy(self._run_btn, "Compute PSD")
            self.err(e)

        self.app.run_in_bg(lambda: api.api_psd(dec), on_ok, on_err)

    def _plot_psd(self, r: api.PSDResult, active_cols: list[str],
                  fstart: float, fstop: float, dec: int) -> None:
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
