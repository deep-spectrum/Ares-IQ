from ares_iq.usrp import UsrpX310, X310Configs
import typer
from typing_extensions import Annotated
from ares_iq.util import CONFIG_FILE
from ares_iq.app.utils import config_set


class X310Device(UsrpX310):
    app = typer.Typer()

    def __init__(self):
        configs = X310Configs.from_yaml(CONFIG_FILE)
        super().__init__(configs)

    @staticmethod
    @app.command('x310-configs', help='Set x310 device configs')
    def dev_configs(spc: Annotated[int | None, typer.Option(help='Samples per capture')] = None,
                    subdev: Annotated[str | None, typer.Option(help='RX frontend specification')] = None,
                    ref: Annotated[str | None, typer.Option(help='Clock source for the USRP device')] = None,
                    rate: Annotated[float | None, typer.Option(help='RX sample rate')] = None,
                    gain: Annotated[float | None, typer.Option(help='Overall RX gain')] = None,
                    spp: Annotated[int | None, typer.Option(help="Samples per packet")] = None):
        configs = X310Configs.from_yaml(CONFIG_FILE)

        config_set(lambda v: setattr(configs, "samples_per_capture", v), spc)
        config_set(lambda v: setattr(configs, "subdev", v), subdev)
        config_set(lambda v: setattr(configs, "ref", v), ref)
        config_set(lambda v: setattr(configs, "rate", v), rate)
        config_set(lambda v: setattr(configs, "gain", v), gain)
        config_set(lambda v: setattr(configs, "samples_per_packet", v), spp)

        configs.to_yaml(CONFIG_FILE)
