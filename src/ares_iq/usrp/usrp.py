from ares_iq_ext.usrp import _USRP, _USRPConfigs, _UsrpStreamArgs
from ares_iq.iq_data import IQData
from decimal import Decimal
from abc import abstractmethod, ABC
from ares_iq.typing import QuantizedData, RealNumber
from attrs import define, field
from ares_iq.validators import is_positive


@define
class USRPConfigs:
    """USRP device configurations and stream configurations.

    Device and stream configurations for USRP platforms.

    Attributes:
        samples_per_capture: Sample chunk size.
        subdev: RX Frontend specification.
        ref: Reference clock source.
        rate: RX sample rate.
        gain: Overall RX gain.
        samples_per_packet: Number of samples per a UDP packet.
    """
    samples_per_capture: int = field(default=200000, validator=is_positive)
    subdev: str = "A:0"
    ref: str = "internal"
    rate: RealNumber = field(default=25e6, validator=is_positive)
    gain: RealNumber = 0
    samples_per_packet: int = field(default=200, validator=is_positive)


class USRP(ABC):
    """Base class for USRP platforms."""
    _iq_data: list[IQData]
    _quantized_data: list[None]

    def __init__(self, dev_args: str, configs: USRPConfigs | None):
        """Initializes the base USRP instance.

        Args:
            dev_args: The device arguments required for finding and opening a USRP device.
            configs: The configurations for the USRP device.
        """

        if configs is None:
            configs_ = _USRPConfigs(dev_args=dev_args)
            stream_ = _UsrpStreamArgs()
        else:
            configs_ = _USRPConfigs(dev_args=dev_args,
                                    spc=configs.samples_per_capture,
                                    subdev=configs.subdev,
                                    ref=configs.ref,
                                    rate=configs.rate,
                                    gain=configs.gain)
            stream_ = _UsrpStreamArgs(spp=configs.samples_per_packet)

        self._usrp: _USRP = _USRP(configs_, stream_)

    def capture_iq(self, center: float, bw: float, file_size: float, verbose: bool, extra: bool):
        """Capture IQ data from the USRP.

        Args:
            center: The center frequency in Hz.
            bw: The bandwidth in Hz.
            file_size: The amount of data to capture in GB.
            verbose: Show the progress bar.
            extra: Like verbose, but show the logging messages too.

        Raises:
            ValueError: Bad configuration arguments.
        """
        iq_data, timestamps = self._usrp.capture_iq(center, bw, file_size, verbose, extra)

        self._iq_data = [IQData() for _ in range(len(timestamps))]
        for data, ts, iq in zip(iq_data, timestamps, self._iq_data):
            iq.iq = data
            iq.ts_sec = int(ts)
            iq.ts_nsec = int((Decimal(ts) - iq.ts_sec) * Decimal('1e9'))

        self._quantize()

    @abstractmethod
    def _quantize(self):
        """Convert the collected IQ data from complex numbers to ADC readings."""

    @property
    def iq_data(self) -> list[IQData]:
        """The captured IQ data from the last call to capture_iq()"""
        return self._iq_data

    @property
    def quantized_data(self) -> list[QuantizedData]:
        """The quantized IQ data from the last call to capture_iq()"""
        return self._quantized_data
