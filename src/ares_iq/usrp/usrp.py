from ares_iq_ext.usrp import _USRP, _USRPConfigs, _UsrpStreamArgs
from ares_iq.iq_data import IQData
from decimal import Decimal
from abc import abstractmethod, ABC
from ares_iq.typing import QuantizedData
from attrs import define, field, validators
from ares_iq.validators import validate_bounds
from ares_iq.configs import ConfigBase


@define
class USRPConfigs(ConfigBase):
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
    _ref_options: tuple[str, ...] = field(default=("internal", "external"), init=False, repr=False)
    samples_per_capture: int = field(default=200000, metadata={"min": 1},
                                     validator=[validators.instance_of(int), validate_bounds])
    subdev: str = "A:0"
    ref: str = field(default="internal", converter=str)
    rate: float = field(default=25e6, metadata={"min": 0, "min_exclusive": True}, validator=validate_bounds)
    gain: float = 0
    samples_per_packet: int = field(default=200, metadata={"min": 1},
                                    validator=[validators.instance_of(int), validate_bounds])

    @ref.validator
    def validate_ref(self, _, value):
        for x in self._ref_options:
            if value == x:
                return
        raise ValueError(f"ref must be on of the following: {', '.join(self._ref_options)}.")


class USRP(ABC):
    """Base class for USRP platforms."""

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

    def capture_iq(self, center: float, bw: float, capture_size: float, verbose: bool = False, extra: bool = False) -> tuple[list[IQData], list[QuantizedData]]:
        """Capture IQ data from the SDR.

        Args:
            center: The center frequency in Hz.
            bw: The bandwidth in Hz.
            capture_size: The maximum amount of IQ data to collect in bytes.
            verbose: Show the progress bar.
            extra: Like verbose, but show the logging messages too.

        Raises:
            ValueError: Bad configuration arguments.
        """
        iq_data, timestamps = self._usrp.capture_iq(center, bw, capture_size, verbose, extra)

        iq_data_ = [IQData() for _ in timestamps]
        for data, ts, iq in zip(iq_data, timestamps, iq_data_):
            iq.iq = data
            iq.ts_sec = int(ts)
            iq.ts_nsec = int((Decimal(ts) - iq.ts_sec) * Decimal('1e9'))

        quant_data = self._quantize(iq_data_)
        return iq_data_, quant_data

    @abstractmethod
    def _quantize(self, iq_data: list[IQData]) -> list[QuantizedData]:
        """Convert the collected IQ data from complex numbers to ADC readings."""
