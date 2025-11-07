from ares_iq.print_utils import print_error
from typing import Any, Callable


def config_set(expr: Callable[[Any], None], val: Any | None):
    if val is not None:
        try:
            expr(val)
        except ValueError as e:
            print_error(str(e))
