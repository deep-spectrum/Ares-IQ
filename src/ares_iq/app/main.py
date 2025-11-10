import importlib
import typer
from ares_iq.util import CONFIG_FILE, CONFIG_DIR
from typing_extensions import Annotated
import os
import pkgutil
from ares_iq.typing import SoftwareDefinedRadio
from ares_iq.save_iq_data import save_iq_data
import yaml
from ares_iq.print_utils import print_error
from ares_iq.print_utils.logging import AresIqHandler
import logging

logger = logging.getLogger()
logger.addHandler(AresIqHandler(warning_panel=True, error_panel=True, critical_error_panel=True))

PLATFORMS: dict[str, SoftwareDefinedRadio] = {}


def import_platforms():
    global PLATFORMS
    main_path = os.path.abspath(__file__)
    main_dir = os.path.dirname(main_path)
    for _, module_name, _ in pkgutil.iter_modules([main_dir]):
        module_path = os.path.join(main_dir, module_name)

        # Only process directories (subpackages)
        if not os.path.isdir(module_path):
            continue
        module = importlib.import_module(f".{module_name}", package="ares_iq.app")
        if hasattr(module, 'PLATFORMS'):
            PLATFORMS = PLATFORMS | module.PLATFORMS


import_platforms()

app = typer.Typer()


@app.command()
def capture(
        center: Annotated[float, typer.Option("--center", "-c", help='Center frequency of the capture in MHz')] = 2450,
        bw: Annotated[float, typer.Option("--bw", "-w", help='Bandwidth of the capture in MHz')] = 160,
        file_size: Annotated[float, typer.Option("--size", "-s", help='The amount of IQ data to capture in GB')] = 4,
        silent: Annotated[bool, typer.Option("--silent", help='Do not show the progress bar')] = False,
        verbose: Annotated[bool, typer.Option("--verbose", "-v", help='Like verbose, but show logging messages too')] = False):
    try:
        with open(CONFIG_FILE, "r") as f:
            configs = yaml.safe_load(f)
        platform: str = configs["platform"]
    except (FileNotFoundError, KeyError):
        print_error("Platform not set.")
        raise typer.Exit()

    if PLATFORMS[platform] is None:
        print_error(f"{platform} is not supported yet.")
        raise typer.Exit()
    PLATFORMS[platform].capture_iq(center * 1e6, bw * 1e6, int(file_size * 1e9), silent, verbose)
    # save_iq_data(PLATFORMS[configs["hw"]].iq_data)  # TODO: separate save function into different package


def valid_platforms(platform: str):
    for _platform in PLATFORMS.keys():
        if platform == _platform:
            return platform
    raise typer.BadParameter(
        "Platform must be one of the following:\n\n" + "\n".join(f' - {key}' for key in PLATFORMS.keys()))


platform_help = "The signal analyzer platform being used. Must be one of the following: " + ", ".join(
    f"'{key}'" for key in PLATFORMS.keys())


@app.command(name='set-platform')
def set_platform(platform: Annotated[str, typer.Argument(
    help=platform_help, callback=valid_platforms)]):
    if CONFIG_FILE.exists():
        with open(CONFIG_FILE, "r") as f:
            configs = yaml.safe_load(f)
    else:
        configs = {}

    configs["platform"] = platform

    with open(CONFIG_FILE, "w") as f:
        yaml.safe_dump(configs, f)


def import_extended_commands():
    for _, platform in PLATFORMS.items():
        if hasattr(platform, "app"):
            app.add_typer(platform.app)


def main():
    CONFIG_DIR.mkdir(exist_ok=True)
    import_extended_commands()
    app()


if __name__ == "__main__":
    main()
