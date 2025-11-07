from ares_iq.usrp import UsrpX310, X310Configs
import typer
from typing_extensions import Annotated
from ares_iq.print_utils import print_error
from ares_iq.util import CONFIG_FILE


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

        try:
            if spc is not None:
                configs.samples_per_capture = spc
            if subdev is not None:
                configs.subdev = subdev
            if ref is not None:
                configs.ref = ref
            if rate is not None:
                configs.rate = rate
            if gain is not None:
                configs.gain = gain
            if spp is not None:
                configs.samples_per_packet = spp
        except ValueError as e:
            print_error(str(e))
            raise typer.Exit()

        configs.to_yaml(CONFIG_FILE)
