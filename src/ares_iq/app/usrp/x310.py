from .usrp import USRP
from ares_iq_ext.usrp import _USRPConfigs


class X310Device(USRP):
    def _quantize(self):
        pass

    def __init__(self):
        configs = _USRPConfigs()
        configs.dev_args = 'type=x300'
        super().__init__(configs)
