from typing import Protocol
from .iq_data import IQData
import os
import pathlib

QuantizedData = None


class SoftwareDefinedRadio(Protocol):
    """Protocol for SoftwareDefinedRadio classes."""

    def capture_iq(self, center: float, bw: float, file_size: float, verbose: bool, extra_verbose: bool) -> None:
        """Capture IQ data from the SDR.

        Args:
            center: The center frequency in Hz.
            bw: The bandwidth in Hz.
            file_size: The maximum amount of IQ data to collect in GB.
            verbose: Show progress bar.
            extra_verbose: Extra verbose output. Shows the progress bar and logging messages.
        """

    @property
    def iq_data(self) -> list[IQData]:
        """IQ data from the capture"""

    @property
    def quantized_data(self) -> list[QuantizedData]:
        """Quantized data from the capture"""


PathLike = str | pathlib.Path | os.PathLike | bytes
RealNumber = int | float
