import numpy as np
import numpy.typing as npt
import datetime as dt


class IQData:
    def __init__(self, iq_data: npt.NDArray[np.complex64], ts: dt.datetime):
        self._iq: npt.NDArray[np.complex64] = iq_data
        self._ts: dt.datetime = ts

    @property
    def iq(self) -> npt.NDArray[np.complex64]:
        return self._iq

    @property
    def ts(self) -> dt.datetime:
        return self._ts
