from ares_iq.typing import SoftwareDefinedRadio
from typing import Type

from .bb60 import BB60Device

PLATFORMS: dict[str, Type[SoftwareDefinedRadio]] = {
    "bb60": BB60Device,
}
