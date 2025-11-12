from __future__ import annotations
import ctypes.util

if not ctypes.util.find_library('usb-1.0'):
    raise OSError('Missing libusb-1.0-0. Please run `sudo apt install libusb-1.0-0`')

from ._sh_sm_series import sm_api_version, SmDeviceType, _SmDevice, get_device_list, get_device_list2

__all__ = ["sm_api_version",
           "SmDeviceType",
           "_SmDevice",
           "get_device_list",
           "get_device_list2",]
