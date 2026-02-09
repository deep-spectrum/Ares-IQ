import typer
from typing_extensions import Annotated
from ares_iq.iq_data import IQData
from ares_iq.signal_hound import SM200A, SM200B, SM200C, SM435B, SM435C, SMConfigs, SmGpsInfo
from ares_iq.signal_hound.sm import logger as sm_logger
from ares_iq.print_utils import print_error
from ares_iq.util import CONFIG_FILE
from ares_iq.typing import QuantizedData
import logging
from ares_iq.app.utils import config_set, print_config_errors


class SMDevice:
    app = typer.Typer()

    def __init__(self):
        self._dev: SM200A | SM200B | SM200C | SM435B | SM435C | None = None
        sm_logger.setLevel(logging.WARNING)

    def capture_iq(self, center: float, bw: float, capture_size: int, silent: bool = True, verbose: bool = False) -> \
            tuple[list[IQData], list[QuantizedData]]:
        if self._dev is None:
            raise typer.Abort()
        try:
            iq, quantized, _ = self._dev.capture_iq(center, bw, capture_size, silent, verbose)
        except Exception as e:
            print_error(str(e))
            raise typer.Exit()
        return [], []

    def platform(self, platform: str):
        configs = SMConfigs.from_yaml(CONFIG_FILE)
        match platform:
            case "sm200a": self._dev = SM200A(configs)
            case "sm200b": self._dev = SM200B(configs)
            case "sm200c": self._dev = SM200C(configs)
            case "sm435b": self._dev = SM435B(configs)
            case "sm435c": self._dev = SM435C(configs)

    @staticmethod
    @app.command(name="sm-config", help="Set default configurations for an sm-series device")
    def config(gps_timestamping: Annotated[bool | None, typer.Option(help='Use GPS timestamping.')] = None,
               gps_lock_timeout: Annotated[int | None, typer.Option(help='The amount of seconds allowed to acquire a GPS disciplined timebase. `0` will disable the timeout.')] = None,
               gps_model: Annotated[str | None, typer.Option(help='Select the GPS model to use.')] = None,
               decimation: Annotated[int | None, typer.Option(help='Downsample factor.')] = None,
               software_filter: Annotated[bool | None, typer.Option(help='Use software filter.')] = None,
               samples_per_capture: Annotated[int | None, typer.Option(help='The amount of samples to collect per a capture.')] = None,
               host: Annotated[str | None, typer.Option(help='The host address for a networked SM device.')] = None,
               device_addr: Annotated[str | None, typer.Option(help='The device address fo the SM device.')] = None,
               port: Annotated[str | None, typer.Option(help='The port the networked SM device is on.')] = None):
        configs = SMConfigs.from_yaml(CONFIG_FILE)

        errors = [
            config_set(lambda v: setattr(configs, "gps_timestamping", v), gps_timestamping),
            config_set(lambda v: setattr(configs, "gps_lock_timeout", v), gps_lock_timeout),
            config_set(lambda v: setattr(configs, "gps_model", v), gps_model),
            config_set(lambda v: setattr(configs, "decimation", v), decimation),
            config_set(lambda v: setattr(configs, "software_filter", v), software_filter),
            config_set(lambda v: setattr(configs, "samples_per_capture", v), samples_per_capture),
            config_set(lambda v: setattr(configs, "host", v), host),
            config_set(lambda v: setattr(configs, "device_addr", v), device_addr),
            config_set(lambda v: setattr(configs, "port", v), port),
        ]

        print_config_errors(errors)

        configs.to_yaml(CONFIG_FILE)
