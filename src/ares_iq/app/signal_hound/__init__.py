from ares_iq.typing import SoftwareDefinedRadio
from typing import Type

from .bb60 import BB60Device
from .sm_series import SMDevice

PLATFORMS: dict[str, Type[SoftwareDefinedRadio]] = {
    "bb60": BB60Device,
    "sm200a": SMDevice,
    "sm200b": SMDevice,
    "sm200c": SMDevice,
    "sm435b": SMDevice,
    "sm435c": SMDevice,
}
