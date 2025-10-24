import typer
from typing_extensions import Annotated
from ares_iq.signal_hound import BB60, BB60Configs, BB60Exception, BBDeviceError
from ares_iq.print_utils import print_error
from ares_iq.util import CONFIG_FILE


class BB60Device(BB60):
    app = typer.Typer()

    def __init__(self):
        configs = BB60Configs.from_yaml(CONFIG_FILE)
        super().__init__(configs)

    def capture_iq(self, center: float, bw: float, file_size_gb: float, verbose: bool, extra: bool) -> None:
        try:
            super().capture_iq(center, bw, file_size_gb, verbose, extra)
        except (BB60Exception, BBDeviceError) as e:
            print_error(str(e))

    @staticmethod
    @app.command(name='bb60-config', help='Set default configurations for the BB60')
    def config(ref_level: Annotated[float | None, typer.Option(help='Reference level of the BB60')] = None,
               decimation: Annotated[int | None, typer.Option(help='Downsample factor')] = None):
        configs = BB60Configs.from_yaml(CONFIG_FILE)

        try:
            if ref_level is not None:
                configs.ref_level = ref_level
            if decimation is not None:
                configs.decimation = decimation
        except ValueError as e:
            print_error(str(e))

        configs.to_yaml(CONFIG_FILE)
