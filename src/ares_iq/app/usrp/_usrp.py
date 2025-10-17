from ares_iq_ext.usrp import _USRP
from ares_iq.iq_data import IQData
from decimal import Decimal
from abc import abstractmethod, ABC
from ares_iq.print_utils import print_error


class USRP(_USRP, ABC):
    _iq_data: list[IQData]
    _quantized_data: list[None]

    @abstractmethod
    def _stream_args(self):
        pass

    def capture_iq(self, center: float, bw: float, file_size: float, verbose: bool, extra: bool):
        self._stream_args()
        try:
            iq_data, timestamps = _USRP.capture_iq(self, center, bw, file_size, verbose, extra)
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
    def quantized_data(self) -> list[None]:
        return self._quantized_data
