from .usrp import USRP, USRPConfigs
from typing_extensions import override
from ares_iq.iq_data import IQData
from ares_iq.typing import QuantizedData


class UsrpX310(USRP):
    """USRP X310 SDR interface"""

    def __init__(self, configs: USRPConfigs | None = None):
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
