from ares_iq.print_utils import print_error
from typing import Any, Callable


def config_set(expr: Callable[[Any], None], val: Any | None):
    """Set configuration values and print an error if value is invalid.

    Args:
        expr: The expression to execute.
        val: The value in the expression
    """
    if val is not None:
        try:
            expr(val)
        except ValueError as e:
            print_error(str(e))
