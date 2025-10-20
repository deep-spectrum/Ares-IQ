from .usrp import USRP, USRPConfigs
from ares_iq_ext.usrp import _USRPConfigs, _UsrpStreamArgs
from ares_iq.configurations import load_config_section


class UsrpX310(USRP):
    _from_cli = False

    def _load_configs(self):
        configs = load_config_section('x310-configs')
        configs_ = _USRPConfigs()
        configs_.dev_args = "type=x300"

        if not self._from_cli:
            return configs_

        if "spc" in configs:
            configs_.samples_per_capture = int(configs["spc"])

        if "subdev" in configs:
            configs_.subdev = configs["subdev"]

        if "ref" in configs:
            configs_.ref = configs["ref"]

        if "rate" in configs:
            configs_.rate = float(configs["rate"])

        if "gain" in configs:
            configs_.gain = float(configs["gain"])

        return configs_

    def _load_stream_args(self):
        configs = load_config_section("x310-stream-configs")
        configs_ = _UsrpStreamArgs()

        if not self._from_cli:
            return configs_

        if "spp" in configs:
            configs_.spp = int(configs["spp"])

        return configs_

    def __init__(self, configs: USRPConfigs | None = None):
        self._spp = None
        if configs is None:
            configs_ = self._load_configs()
            stream_args_ = self._load_stream_args()
        else:
            configs_ = configs.configs_
            configs_.dev_args = "type=x300"
            stream_args_ = configs.stream_args_
        super().__init__(configs_, stream_args_)

    def _quantize(self):
        pass
