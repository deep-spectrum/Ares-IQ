from ares_iq_ext.usrp import _USRP
from ares_iq.iq_data import IQData
from decimal import Decimal


class USRP(_USRP):
    _iq_data: list[IQData]
    _quantized_data: list[None]

    def capture_iq(self, center: float, bw: float, file_size: float):
        # TODO: Set stream args
        iq_data, timestamps = super().capture_iq(center, bw, file_size)

        self._iq_data = [IQData() for _ in range(len(timestamps))]
        for data, ts, iq in zip(iq_data, timestamps, self._iq_data):
            iq.iq = data
            iq.ts_sec = int(ts)
            iq.ts_nsec = int((Decimal(ts) - iq.ts_sec) * Decimal('1e9'))

        # TODO quantize

    @property
    def iq_data(self) -> list[IQData]:
        return self._iq_data

    @property
    def quantized_data(self) -> list[None]:
        return self._quantized_data
