from ares_iq_ext.usrp import _USRP, _USRPConfigs, _UsrpStreamArgs
from ares_iq.iq_data import IQData
from decimal import Decimal
from abc import abstractmethod, ABC
from ares_iq.print_utils import print_error
from ares_iq.typing import QuantizedData


class USRPConfigs:
    def __init__(self,
                 samples_per_capture: int = 200000,
                 subdev: str = "A:0",
                 ref: str = "internal",
                 rate: int | float = 25e6,
                 gain: int | float = 0,
                 samples_per_packet: int = 200):
        self._configs = _USRPConfigs()
        self._stream_args = _UsrpStreamArgs()
        self._configs.samples_per_capture = samples_per_capture
        self._configs.subdev = subdev
        self._configs.ref = ref
        self._configs.rate = rate
        self._configs.gain = gain
        self._stream_args.spp = samples_per_packet

    @property
    def samples_per_capture(self):
        return self._configs.samples_per_capture

    @samples_per_capture.setter
    def samples_per_capture(self, spc: int):
        self._configs.samples_per_capture = spc

    @property
    def subdev(self):
        return self._configs.subdev

    @subdev.setter
    def subdev(self, dev: str):
        self._configs.subdev = dev

    @property
    def ref(self):
        return self._configs.ref

    @ref.setter
    def ref(self, osc: str):
        self._configs.ref = osc

    @property
    def rate(self):
        return self._configs.rate

    @rate.setter
    def rate(self, speed: int | float):
        self._configs.rate = speed

    @property
    def gain(self):
        return self._configs.gain

    @gain.setter
    def gain(self, k: int | float):
        self._configs.gain = k

    @property
    def samples_per_packet(self):
        return self._stream_args.spp

    @samples_per_packet.setter
    def samples_per_packet(self, spp: int):
        self._stream_args.spp = spp

    @property
    def configs_(self):
        return self._configs

    @property
    def stream_args_(self):
        return self._stream_args


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
