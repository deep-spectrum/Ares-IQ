from rich.console import Console
from rich.panel import Panel
from typer import Exit


def print_error(msg: str, early_exit: bool = True):
    """Print an error to the console.

    Print the given error message to the console. This will also exit the
    program unless specified not to.

    Args:
        msg: The error message to print.
        early_exit: Flag indicating if the program should exit.
    """
    console = Console()
    console.print(Panel(msg, title='Error', title_align='left', border_style='red', expand=True))
    if early_exit:
        raise Exit(code=1)


def print_warning(msg: str, early_exit: bool = False):
    """Print a warning to the console.

    Print the given warning message to the console. This can exit the program
    early if specified to do so.

    Args:
        msg: The warning message to print.
        early_exit: Flag indicating if the program should exit.
    """
    console = Console()
    console.print(Panel(msg, title='Warning', title_align='left', border_style='yellow', expand=True))
    if early_exit:
        raise Exit(code=1)
