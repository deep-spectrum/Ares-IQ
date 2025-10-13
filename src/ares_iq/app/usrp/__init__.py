# from .x410 import X410Device
from .x310 import X310Device
from ares_iq.typing import SoftwareDefinedRadio


PLATFORMS: dict[str, SoftwareDefinedRadio] = {
#     "x410": X410Device(),
    "x310": X310Device(),
}
