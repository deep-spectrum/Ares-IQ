import logging
from .console_print import print_debug, print_info, print_warning, print_error, print_critical

OFF = logging.CRITICAL + 10


class AresIqHandler(logging.Handler):
    def __init__(self,
                 level: int | str = 0,
                 dbg_panel: bool = False,
                 info_panel: bool = False,
                 warning_panel: bool = False,
                 error_panel: bool = False,
                 critical_error_panel: bool = False,
                 fmt: str | logging.Formatter = "%(message)s"):
        super().__init__(level=level)

        if isinstance(fmt, logging.Formatter):
            self.setFormatter(fmt)
        else:
            formatter = logging.Formatter(fmt)
            self.setFormatter(formatter)

        self._dbg_panel = dbg_panel
        self._inf_panel = info_panel
        self._wrn_panel = warning_panel
        self._err_panel = error_panel
        self._crit_panel = critical_error_panel

    def emit(self, record: logging.LogRecord):
        msg = self.format(record)

        match record.levelno:
            case logging.DEBUG:
                print_debug(msg, self._dbg_panel)
            case logging.INFO:
                print_info(msg, self._inf_panel)
            case logging.WARNING:
                print_warning(msg, self._wrn_panel)
            case logging.ERROR:
                print_error(msg, self._err_panel)
            case logging.CRITICAL:
                print_critical(msg, self._crit_panel)
