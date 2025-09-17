from .bbdevice.bb_api import *
import numpy as np
from iq_capture.print_utils import print_warning


SAMPLES_PER_CAPTURE = 262144

MAX_BANDWIDTH = 20.0e6


def bb60_stream_iq(center_freq: float, bw: float):
    if bw > MAX_BANDWIDTH:
        print_warning(f"Unable to set the bandwidth to {bw / 1.0e6} MHz. Setting to {MAX_BANDWIDTH / 1.0e6} MHz")
        bw = MAX_BANDWIDTH

    handle = bb_open_device()['handle']
