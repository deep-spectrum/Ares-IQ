from ares_iq.typing import SoftwareDefinedRadio

from .bb60 import BB60Device

PLATFORMS: dict[str, SoftwareDefinedRadio] = {
    "bb60": BB60Device(),
}
