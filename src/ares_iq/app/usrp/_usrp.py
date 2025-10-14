from ares_iq_ext.usrp import _USRP
from ares_iq.iq_data import IQData
from decimal import Decimal
from abc import ABCMeta, abstractmethod


__USRPMeta = type(_USRP)


class _USRPMeta(__USRPMeta, ABCMeta):
    pass


class USRP(_USRP, metaclass=_USRPMeta):
    _iq_data: list[IQData]
    _quantized_data: list[None]

    @abstractmethod
    def _stream_args(self):
        pass

    def capture_iq(self, center: float, bw: float, file_size: float):
        self._stream_args()
        iq_data, timestamps = super().capture_iq(center, bw, file_size)

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
    def quantized_data(self) -> list[None]:
        return self._quantized_data
