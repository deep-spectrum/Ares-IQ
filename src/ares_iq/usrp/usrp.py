from ares_iq_ext.usrp import _USRP, _USRPConfigs, _UsrpStreamArgs
from ares_iq.iq_data import IQData
from decimal import Decimal
from abc import abstractmethod, ABC
from ares_iq.print_utils import print_error
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
    _iq_data: list[IQData]
    _quantized_data: list[None]

    def __init__(self, configs: _USRPConfigs, stream_args: _UsrpStreamArgs):
        self._usrp: _USRP = _USRP(configs, stream_args)

    def capture_iq(self, center: float, bw: float, file_size: float, verbose: bool, extra: bool):
        try:
            iq_data, timestamps = self._usrp.capture_iq(center, bw, file_size, verbose, extra)
        except ValueError as e:
            print_error(str(e))
            raise

        self._iq_data = [IQData() for _ in range(len(timestamps))]
        for data, ts, iq in zip(iq_data, timestamps, self._iq_data):
            iq.iq = data
            iq.ts_sec = int(ts)
            iq.ts_nsec = int((Decimal(ts) - iq.ts_sec) * Decimal('1e9'))

        self._quantize()

    @abstractmethod
    def _quantize(self):
        pass

    @property
    def iq_data(self) -> list[IQData]:
        return self._iq_data

    @property
    def quantized_data(self) -> list[QuantizedData]:
        return self._quantized_data
