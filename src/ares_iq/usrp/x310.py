from .usrp import USRP, USRPConfigs
from typing_extensions import override
from ares_iq.iq_data import IQData
from ares_iq.typing import QuantizedData
from attrs import define, field
from enum import Enum


class X310RefOptions(Enum):
    """Reference clock options for the USRP x310."""
    INTERNAL = "internal"
    EXTERNAL = "external"
    GPSDO = "gpsdo"

    def __str__(self):
        return self.value


@define
class X310Configs(USRPConfigs):
    """Configurations for the USRP x310 platform."""
    _ref_options: tuple[str, ...] = field(default=tuple([str(x) for x in X310RefOptions]), init=False, repr=False)


class UsrpX310(USRP):
    """USRP X310 SDR interface"""

    def __init__(self, configs: X310Configs | None = None):
        """Initializes the USRP X310 instance.

        Args:
            configs:
                Configurations for the USRP X310. If none are provided, default
                values will be used.
        """
        super().__init__("type=x300", configs)

    @override
    def _quantize(self, iq_data: list[IQData]):
        # TODO: implement me
        return [QuantizedData() for _ in iq_data]
