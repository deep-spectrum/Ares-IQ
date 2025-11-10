"""Protocols and types for the ares_iq package."""

from typing import Protocol
from .iq_data import IQData
import os
import pathlib

QuantizedData = object
"""Type alias for the quantized data."""

PathLike = str | pathlib.Path | os.PathLike | bytes
"""Type alias for paths."""


class SoftwareDefinedRadio(Protocol):
    """Protocol for SoftwareDefinedRadio classes."""

    def capture_iq(self, center: float, bw: float, capture_size: int, silent: bool = True, verbose: bool = False) -> tuple[list[IQData], list[QuantizedData]]:
        """Capture IQ data from the SDR.

        Args:
            center: The center frequency in Hz.
            bw: The bandwidth in Hz.
            capture_size: The maximum amount of IQ data to collect in bytes.
            silent: Do not show the progress bar.
            verbose: Show the logging messages.

        Returns:
            A list of IQ data and a list of QuantizedData
        """
