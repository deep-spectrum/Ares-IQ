from __future__ import annotations
import ctypes.util

if not ctypes.util.find_library('usb-1.0'):
    raise OSError('Missing libusb-1.0-0. Please run `sudo apt install libusb-1.0-0`')

from ._sh_sm_series import sm_api_version, SmDeviceType, SmGpsPlatformModel, _SmConfigs, _SmDevice, _SM, \
    get_device_list, get_device_list2, HOST_ADDR_ANY, DEFAULT_DEV_ADDR, DEFAULT_PORT
from ._sh_sm_series import LOGGER_NAME as SM_LOGGER_NAME

__all__ = [
    # Enums
    "SmDeviceType",
    "SmGpsPlatformModel",

    # Classes
    "_SmConfigs",
    "_SmDevice",
    "_SM",

    # Functions
    "sm_api_version",
    "get_device_list",
    "get_device_list2",

    # Attributes
    "HOST_ADDR_ANY",
    "DEFAULT_DEV_ADDR",
    "DEFAULT_PORT",
    "SM_LOGGER_NAME",
]
