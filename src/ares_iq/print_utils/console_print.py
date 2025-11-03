from rich.console import Console
from rich.panel import Panel


def print_error(msg):
    console = Console()
    console.print(Panel(msg, title='Error', title_align='left', border_style='red', expand=True))


def print_warning(msg):
    console = Console()
    console.print(Panel(msg, title='Warning', title_align='left', border_style='yellow', expand=True))
