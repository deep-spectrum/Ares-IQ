from ares_iq.configurations import load_config_section, save_config_section
import typer
from typing_extensions import Annotated
from ares_iq.signal_hound import BB60, BB60Configs, BB60Exception, BBDeviceError
from ares_iq.print_utils import print_error


class BB60Device(BB60):
    app = typer.Typer()

    @staticmethod
    def _load_configs():
        # TODO: Loading invalid values will cause the application to fail.
        #  Validate in config setting. This will be fixed with configs refactor.
        configs = load_config_section("bb60-configs")
        configs_ = BB60Configs()

        if "ref-level" in configs:
            configs_.ref_level = float(configs["ref-level"])

        if "decimation" in configs:
            configs_.decimation = int(configs["decimation"])

        return configs_

    def __init__(self):
        configs = self._load_configs()
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
        configs = load_config_section("bb60-configs")
        if ref_level is not None:
            configs['ref-level'] = str(ref_level)
        if decimation is not None:
            configs['decimation'] = str(decimation)
        save_config_section("bb60-configs", configs)
