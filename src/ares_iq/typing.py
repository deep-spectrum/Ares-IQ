"""Protocols and types for the ares_iq package."""

from typing import Protocol
from .iq_data import IQData

QuantizedData = object
"""Type alias for the quantized data."""


class SoftwareDefinedRadio(Protocol):
    """Protocol for SoftwareDefinedRadio classes."""

    def capture_iq(self, center: float, bw: float, capture_size: int, verbose: bool = False, extra_verbose: bool = False) -> tuple[list[IQData], list[QuantizedData]]:
        """Capture IQ data from the SDR.

        Args:
            center: The center frequency in Hz.
            bw: The bandwidth in Hz.
            capture_size: The maximum amount of IQ data to collect in bytes.
            verbose: Show progress bar.
            extra_verbose: Extra verbose output. Shows the progress bar and logging messages.

        Returns:
            A list of IQ data and a list of QuantizedData
        """
