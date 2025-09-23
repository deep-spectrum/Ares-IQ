from .x410 import X410Device
from ares_iq.typing import SoftwareDefinedRadio


PLATFORMS: dict[str, SoftwareDefinedRadio] = {
    "x410": X410Device()
}
