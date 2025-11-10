from .print_utils import print_warning
from .iq_data import IQData
import numpy as np
import h5py
import datetime as dt
from pathlib import Path
from .warnings import deprecated


SAVE_DIR = Path.cwd() / "ares-iq-data"


def _save_file(iq, ts):
    fname = f"capture-{dt.datetime.now().strftime('%Y%m%d-%H%M%S')}.h5"
    SAVE_DIR.mkdir(exist_ok=True)
    with h5py.File(SAVE_DIR / fname, "w") as f:
        f.create_dataset("iq_data", data=iq)
        f.create_dataset("iq_ts", data=ts)


@deprecated("Being split into separate package.")
def save_iq_data(data: list[IQData]):
    """Save the captured IQ data.

    Args:
        data: The captured IQ data.
    """
    if not data:
        return
    print_warning("TODO: I'm not sure if this is a good way to store data. Will likely factor data saving into a separate repo maintained by Tianshu...")
    ts = np.vstack([np.int64(iq.ts_sec * int(1e9)) + np.int64(iq.ts_nsec) for iq in data])
    iq = np.vstack([iq_.iq for iq_ in data])

    _save_file(iq, ts)
