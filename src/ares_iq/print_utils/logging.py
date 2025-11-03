import logging
from .console_print import print_error, print_warning
from rich import print as rprint


def _dbg_print(msg: str):
    rprint(f"[DBG] {msg}")


def _info_print(msg: str):
    rprint(f"[green][INFO][/green] {msg}")


def _crit_print(msg: str):
    rprint(f"[][CRITICAL][/] {msg}")


class AresIqLoggingHandle(logging.Handler):
    def __init__(self, level: int | str = 0):
        super().__init__(level=level)

        formatter = logging.Formatter("%(message)s")
        self.setFormatter(formatter)

    def emit(self, record: logging.LogRecord):
        msg = self.format(record)

        match record.levelno:
            case logging.DEBUG:
                _dbg_print(msg)
            case logging.INFO:
                _info_print(msg)
            case logging.WARNING:
                print_warning(msg)
            case logging.ERROR:
                print_error(msg)
            case logging.CRITICAL:
                _crit_print(msg)
