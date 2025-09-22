from typing import Protocol, Optional
import numpy.typing as npt
import numpy as np
from .iq_data import IQData


class SoftwareDefinedRadio(Protocol):
    def capture_iq(self, center: float, bw: float) -> tuple[npt.NDArray[np.complex64], Optional[npt.NDArray[np.int32]]]:
        """
        Capture IQ data from the SDR.
        :param center: The center frequency in Hz
        :param bw: The bandwidth in Hz
        :return: The captured IQ data and the
        """

    @property
    def iq_data(self) -> list[IQData]:
        """IQ data from the capture"""

    @property
    def quantized_data(self) -> tuple[npt.NDArray[np.int32], npt.NDArray[np.uint64]]:
        """Quantized data from the capture"""
