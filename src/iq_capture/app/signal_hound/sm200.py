# from .smdevice.sm_api import *
# import numpy as np
# import typer
# from typing_extensions import Annotated
# from iq_capture.print_utils import print_warning, print_error
# from iq_capture.configurations import load_config_section, save_config_section
#
#
# SAMPLES_PER_CAPTURE = 262144
# SM_SERIES_NETWORKED_MAX_RT_SPAN = SM_REAL_TIME_MAX_SPAN
# SM_SERIES_USB_MAX_RT_SPAN = 20.0e6
#
#
# def _print_sm_error(err: SMDeviceError, config_name: str):
#     s = f"{config_name}: {str(err)}"
#     if err.warning:
#         print_warning(s)
#     else:
#         print_error(s)
#
# 
# def _get_device_handle() -> tuple[object, float]:
#     configs = load_config_section("platform")
#     networked = configs["hw"] == "sm200c"
#
#     if networked:
#         devices = sm_network_config_get_device_list()
#     else:
#         devices = sm_get_device_list2()
#     device_count = devices["device_count"]
#     print(devices)
#     print(sm_get_API_version())
#     return None, None
#
#
# def sm200_stream_iq(center_freq: float, bw: float):
#     handle, max_bw = _get_device_handle()
#
#
# app = typer.Typer()
#
# @app.command(name="sm200-config")
# def sm200_config(decimation: Annotated[int, typer.Argument(help='Downsample factor')]):
#     configs = load_config_section("sm200-configs")
#     configs['decimation'] = str(decimation)
#     save_config_section("sm200-configs", configs)
