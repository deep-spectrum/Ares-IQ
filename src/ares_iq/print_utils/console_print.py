from rich.console import Console
from rich.panel import Panel


def _print_fmt(msg: str, color: str, panel: bool, title: str):
    console = Console()
    if panel:
        console.print(Panel(msg, title=title, title_align='left', border_style=color, expand=True))
    else:
        console.print(f"[{color}][{title.upper()}][/{color}] {msg}")

def print_debug(msg: str, panel: bool = False):
    """Print a debug message to the console.

    Args:
        msg: The debug message to print.
        panel: Print the message in a panel.
    """
    _print_fmt(msg, "dim cyan", panel, "Debug")


def print_info(msg: str, panel: bool = False):
    """Print an info message to the console.

    Args:
        msg: The info message to print.
        panel: Print the message in a panel.
    """
    _print_fmt(msg, "green", panel, "Info")


def print_warning(msg: str, panel: bool = True):
    """Print a warning message to the console.

    Args:
        msg: The warning message to print.
        panel: Print the message in a panel.
    """
    _print_fmt(msg, "yellow", panel, "Warning")


def print_error(msg: str, panel: bool = True):
    """Print an error message to the console.

    Args:
        msg: The error message to print.
        panel: Print the message in a panel.
    """
    _print_fmt(msg, "red", panel, "Error")


def print_critical(msg: str, panel: bool = True):
    """Print a critical error message to the console.

    Args:
        msg: The warning message to print.
        panel: Print the message in a panel.
    """
    _print_fmt(msg, "magenta", panel, "Critical")
