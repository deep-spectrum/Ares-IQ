from .bbdevice.bb_api import *
import numpy as np
from iq_capture.print_utils import print_warning, print_error
from iq_capture.configurations import load_config_section


SAMPLES_PER_CAPTURE = 262144


def _print_bb_error(err: BBDeviceError):
    s = str(err)
    if err.warning:
        print_warning(s)
    else:
        print_error(s)


def _get_device_handle() -> tuple[object, float]:
    devices = bb_get_serial_number_list_2()
    device_count = devices["device_count"].value
    if device_count == 0:
        print_error("No BB60 devices found")
    elif device_count > 1:
        print_error("Multiple BB60 devices found. Please connect 1 device only")

    max_bw = BB60A_MAX_RT_SPAN if devices["device_types"][0] == BB_DEVICE_BB60A else BB60C_MAX_RT_SPAN

    return bb_open_device()["handle"], max_bw.value


def _call_config_func(func, handler, *args):
    try:
        func(handler, *args)
    except BBDeviceError as e:
        _print_bb_error(e)


def _configure_bb_device(handle: object, center: float, bw: float):
    configs = load_config_section("bb60-configs")

    # Reference level
    ref_level = -20.0
    if 'ref-level' in configs:
        ref_level = configs['ref-level']
    _call_config_func(bb_configure_ref_level, handle, ref_level)

    # Gain and attenuation
    bb_configure_gain_atten(handle, BB_AUTO_GAIN, BB_AUTO_ATTEN)

    # Center frequency
    _call_config_func(bb_configure_IQ_center, handle, center)

    # Bandwidth
    decimation = BB_MIN_DECIMATION
    if 'decimation' in configs:
        decimation = configs['decimation']
    _call_config_func(bb_configure_IQ, handle, decimation, bw)



def bb60_stream_iq(center_freq: float, bw: float):
    handle, max_bw = _get_device_handle()

    if bw > max_bw:
        print_warning(f"Unable to set the bandwidth to {bw / 1.0e6} MHz. Setting to {max_bw / 1.0e6} MHz")
        bw = max_bw

    _configure_bb_device(handle, center_freq, bw)

    bb_initiate(handle, BB_STREAMING, BB_STREAM_IQ)

    # TODO: get IQ data

    bb_close_device(handle)
