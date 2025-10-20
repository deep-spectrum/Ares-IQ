import numpy as np
import numpy.typing as npt
import datetime as dt


class IQData:
    _iq: npt.NDArray[np.complex64]
    _ts_s = 0
    _ts_ns = 0

    @property
    def iq(self) -> npt.NDArray[np.complex64]:
        return self._iq

    @iq.setter
    def iq(self, iq_data: npt.NDArray[np.complex64]):
        self._iq = iq_data

    @property
    def ts(self) -> dt.datetime:
        return dt.datetime.fromtimestamp(self._ts_s + (self._ts_ns / 1e9), tz=dt.timezone.utc)

    @property
    def ts_sec(self):
        return self._ts_s

    @ts_sec.setter
    def ts_sec(self, ts_s: int):
        self._ts_s = ts_s

    @property
    def ts_nsec(self):
        return self._ts_ns

    @ts_nsec.setter
    def ts_nsec(self, ts_ns: int):
        self._ts_ns = ts_ns
