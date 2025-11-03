import logging


class AresIqLoggingHandle(logging.Handler):
    def __init__(self, level: int | str = 0):
        super().__init__(level=level)

        formatter = logging.Formatter("%(message)s")
        self.setFormatter(formatter)

    def emit(self, record: logging.LogRecord):
        msg = self.format(record)

        match record.levelno:
            case logging.DEBUG:
                pass
            case logging.INFO:
                pass
            case logging.WARNING:
                pass
            case logging.ERROR:
                pass
            case logging.CRITICAL:
                pass
