from ares_iq.print_utils import print_error
from typing import Any, Callable


def config_set(expr: Callable[[Any], None], val: Any | None) -> str | None:
    """Set configuration values and print an error if value is invalid.

    Args:
        expr: The expression to execute.
        val: The value in the expression.

    Returns:
        The error string on error.
        None if no error occurred,
    """
    if val is not None:
        try:
            expr(val)
        except ValueError as e:
            return str(e)
    return None


def print_config_errors(errors: list[str | None]) -> None:
    """Print all the error messages at once.

    Args:
        errors: The error messages to print.
    """
    errors = [error for error in errors if error is not None]
    if errors:
        print_error("\n\n".join(errors))
