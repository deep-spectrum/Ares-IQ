from .usrp import USRP, USRPConfigs
from ares_iq_ext.usrp import _USRPConfigs, _UsrpStreamArgs


class UsrpX310(USRP):
    def __init__(self, configs: USRPConfigs | None = None):
        if configs is None:
            configs_ = _USRPConfigs()
            stream_args_ = _UsrpStreamArgs()
        else:
            configs_ = configs.configs_
            stream_args_ = configs.stream_args_
        configs_.dev_args = "type=x300"
        super().__init__(configs_, stream_args_)

    def _quantize(self):
        pass
