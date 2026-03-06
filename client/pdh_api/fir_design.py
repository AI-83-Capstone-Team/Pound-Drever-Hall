"""
Windowed-sinc FIR lowpass filter design for the PDH FPGA FIR block.

Canonical implementation — also referenced by core/hw/sim/run_fir_freq.py.
"""
import math

FS = 125e6  # hardware sample rate (Hz)


def design_lowpass(ntaps: int, corner_hz: float, window: str) -> list[float]:
    """Windowed-sinc lowpass FIR.

    Returns a list of ntaps float coefficients, all in (-1, 1) (Q15-safe).

    Args:
        ntaps:     Number of filter taps.
        corner_hz: Half-amplitude (-6 dB) cutoff frequency in Hz.
        window:    One of "hann", "hamming", "blackman", "rectangular".

    Raises:
        ValueError: If any coefficient magnitude >= 1.0 (exceeds Q15 range).
    """
    wc = 2.0 * math.pi * corner_hz / FS
    nc = (ntaps - 1) / 2.0
    coeffs = []
    for n in range(ntaps):
        x = n - nc
        h = wc / math.pi if abs(x) < 1e-10 else math.sin(wc * x) / (math.pi * x)
        if window == "hann":
            w = 0.5 * (1.0 - math.cos(2.0 * math.pi * n / (ntaps - 1)))
        elif window == "hamming":
            w = 0.54 - 0.46 * math.cos(2.0 * math.pi * n / (ntaps - 1))
        elif window == "blackman":
            w = (0.42
                 - 0.50 * math.cos(2.0 * math.pi * n / (ntaps - 1))
                 + 0.08 * math.cos(4.0 * math.pi * n / (ntaps - 1)))
        else:  # rectangular
            w = 1.0
        coeffs.append(h * w)
    if max(abs(c) for c in coeffs) >= 1.0:
        raise ValueError(
            "Coefficient magnitude >= 1 — exceeds Q15 range. "
            "Reduce corner frequency or change window."
        )
    return coeffs
