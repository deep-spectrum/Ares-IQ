import importlib
import typer
from iq_capture.configurations import load_config_section, save_config_section, CONFIG_DIR
from pathlib import Path
from typing_extensions import Annotated
import os
import pkgutil
from iq_capture.typing import SoftwareDefinedRadio

try:
    from .usrp import collect_usrp_iq_data
except ImportError:
    from iq_capture.uhd_installation import install_uhd
    import sys

    install_uhd()
    os.execv(sys.executable, [sys.executable] + sys.argv)


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
        module = importlib.import_module(f".{module_name}", package="iq_capture.app")
        if hasattr(module, 'PLATFORMS'):
            PLATFORMS = PLATFORMS | module.PLATFORMS

import_platforms()

app = typer.Typer()
configs_path = Path().home() / ".iq_capture"
configs_file = configs_path / "config.ini"


@app.command()
def capture(center: Annotated[float, typer.Option("--center", "-c", help='Center frequency of the capture in MHz')] = 2450,
            bw: Annotated[float, typer.Option("--bw", "-w", help='Bandwidth of the capture in MHz')] = 160,
            file_size: Annotated[float, typer.Option("--size", "-s", help='The amount of IQ data to capture in GB')] = 4):
    configs = load_config_section("platform")
    if "hw" not in configs:
        raise typer.Abort("Please run set-platform first")

    if PLATFORMS[configs["hw"]] is None:
        raise typer.Abort(f"{configs['hw']} is not supported yet.")
    PLATFORMS[configs["hw"]].capture_iq(center * 1e6, bw * 1e6, file_size)


def valid_platforms(platform: str):
    for _platform in PLATFORMS.keys():
        if platform == _platform:
            return platform
    raise typer.BadParameter(
        "Platform must be one of the following:\n\n" + "\n".join(f' - {key}' for key in PLATFORMS.keys()))


@app.command(name='set-platform')
def set_platform(platform: Annotated[str, typer.Argument(
    help="The signal analyzer platform being used. Must be one of the following: " + ", ".join(
        f"'{key}'" for key in PLATFORMS.keys()), callback=valid_platforms)]):
    configs = load_config_section("platform")
    configs["hw"] = platform
    save_config_section("platform", configs)


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
