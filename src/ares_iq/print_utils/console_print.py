from rich.console import Console
from rich.panel import Panel
from typer import Exit


def print_error(msg, early_exit: bool = True):
    console = Console()
    console.print(Panel(msg, title='Error', title_align='left', border_style='red', expand=True))
    if early_exit:
        raise Exit(code=1)


def print_warning(msg, early_exit: bool = False):
    console = Console()
    console.print(Panel(msg, title='Warning', title_align='left', border_style='yellow', expand=True))
    if early_exit:
        raise Exit(code=1)
