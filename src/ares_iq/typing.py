"""Protocols and types for the ares_iq package."""

from typing import Protocol
from .iq_data import IQData
import os
import pathlib

QuantizedData = None
"""Type alias for the quantized data."""

PathLike = str | pathlib.Path | os.PathLike | bytes
"""Type alias for paths."""


class SoftwareDefinedRadio(Protocol):
    """Protocol for SoftwareDefinedRadio classes."""

    def capture_iq(self, center: float, bw: float, capture_size: int, verbose: bool, extra_verbose: bool) -> None:
        """Capture IQ data from the SDR.

        Args:
            center: The center frequency in Hz.
            bw: The bandwidth in Hz.
            capture_size: The maximum amount of IQ data to collect in bytes.
            verbose: Show progress bar.
            extra_verbose: Extra verbose output. Shows the progress bar and logging messages.
        """

    @property
    def iq_data(self) -> list[IQData]:
        """IQ data from the capture"""

    @property
    def quantized_data(self) -> list[QuantizedData]:
        """Quantized data from the capture"""
