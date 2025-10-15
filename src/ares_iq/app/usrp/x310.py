from ._usrp import USRP
from ares_iq_ext.usrp import _USRPConfigs
import typer
from typing_extensions import Annotated
from ares_iq.configurations import load_config_section, save_config_section
from ares_iq.print_utils import print_error


class X310Device(USRP):
    app = typer.Typer()

    @staticmethod
    def _load_configs():
        configs = load_config_section('x310-configs')
        configs_ = _USRPConfigs()
        configs_.dev_args = "type=x300"

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

    def __init__(self):
        configs = self._load_configs()
        super().__init__(configs)

    def _quantize(self):
        pass

    @staticmethod
    @app.command('x310-stream-args', help='Set USRP platform stream arguments')
    def stream_args(spp: Annotated[int | None, typer.Option(help="Samples per packet")] = None):
        configs = load_config_section("x310-stream-configs")
        if spp is not None:
            configs["spp"] = str(spp)

        save_config_section("x310-stream-configs", configs)

    def _stream_args(self):
        configs = load_config_section("x310-stream-configs")
        if "spp" not in configs:
            spp = 200
        else:
            spp = int(configs["spp"])
        self._set_stream_args(spp)

    @staticmethod
    @app.command('x310-configs', help='Set x310 device configs')
    def dev_configs(spc: Annotated[int | None, typer.Option(help='Samples per capture')] = None,
                    subdev: Annotated[str | None, typer.Option(help='RX frontend specification')] = None,
                    ref: Annotated[str | None, typer.Option(help='Clock source for the USRP device')] = None,
                    rate: Annotated[float | None, typer.Option(help='RX sample rate')] = None,
                    gain: Annotated[float | None, typer.Option(help='Overall RX gain')] = None):
        configs = load_config_section('x310-configs')

        if spc is not None:
            if spc <= 0:
                print_error("spc must be a non-zero positive integer")
            configs["spc"] = str(spc)

        if subdev is not None:
            configs["subdev"] = subdev

        if ref is not None:
            if not (ref == "internal" or ref == "external" or ref == "gpsdo"):
                print_error("ref must be `internal`, `external`, or `gpsdo`")
            configs["ref"] = ref

        if rate is not None:
            configs["rate"] = str(rate)

        if gain is not None:
            configs["gain"] = str(gain)

        save_config_section('x310-configs', configs)
