import typer
from typing_extensions import Annotated
from ares_iq.iq_data import IQData
from ares_iq.signal_hound import SM200A, SM200B, SM200C, SMConfigs, SmGpsInfo
from ares_iq.signal_hound.sm import logger as sm_logger
from ares_iq.print_utils import print_error
from ares_iq.util import CONFIG_FILE
from ares_iq.typing import QuantizedData
import logging
from ares_iq.app.utils import config_set, print_config_errors


class SM200ADevice(SM200A):
    app = typer.Typer()

    def __init__(self):
        configs = SMConfigs.from_yaml(CONFIG_FILE)
        super().__init__(configs)
        sm_logger.setLevel(logging.WARNING)

    def capture_iq(self, center: float, bw: float, capture_size: int, silent: bool = True, verbose: bool = False) -> \
            tuple[list[IQData], list[QuantizedData]]:
        try:
            iq, quantized, _ = super().capture_iq(center, bw, capture_size, silent, verbose)
        except Exception as e:
            print_error(str(e))
            raise typer.Exit()
        return iq, quantized
