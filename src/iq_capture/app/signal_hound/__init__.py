from .bb60 import BB60Device
from iq_capture.typing import SoftwareDefinedRadio

from .sm200 import sm200_stream_iq
from .sm200 import app as sm200_app

PLATFORMS: dict[str, SoftwareDefinedRadio] = {
    "bb60": BB60Device,
}
