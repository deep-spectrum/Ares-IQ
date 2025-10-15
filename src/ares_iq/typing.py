from typing import Protocol
from .iq_data import IQData


class SoftwareDefinedRadio(Protocol):
    def capture_iq(self, center: float, bw: float, file_size: float, verbose: bool, extra: bool) -> None:
        """
        Capture IQ data from the SDR.
        :param center: The center frequency in Hz
        :param bw: The bandwidth in Hz
        :param file_size: The maximum amount of IQ data to collect in GB
        :param verbose: Verbose output
        :param extra: Extra verbose output
        :return: The captured IQ data and the
        """

    @property
    def iq_data(self) -> list[IQData]:
        """IQ data from the capture"""

    @property
    def quantized_data(self) -> list[None]:
        """Quantized data from the capture"""
