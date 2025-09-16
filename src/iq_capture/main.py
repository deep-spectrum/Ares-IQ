import typer
from .configurations import load_config_section, save_config_section, CONFIG_DIR
from pathlib import Path
from typing_extensions import Annotated
try:
    pass  # TODO: import library that uses uhd here
except ImportError:
    from .uhd_installation import install_uhd
    import os
    import sys
    install_uhd()
    os.execv(sys.executable, [sys.executable] + sys.argv)


PLATFORMS = {
    "usrp": None,  # TODO: Place iq data gathering funcs here for each platform
    "sm200": None,
    "bb60": None,
}


app = typer.Typer()
configs_path = Path().home() / ".iq_capture"
configs_file = configs_path / "config.ini"


@app.command()
def capture():
    pass


def valid_platforms(platform: str):
    if platform != 'usrp' and platform != 'signal-hound':
        raise typer.BadParameter("Platform must be one of the following:\n\n" + "\n".join(f' - {key}' for key in PLATFORMS.keys()))
    return platform

@app.command(name='set-platform')
def set_platform(platform: Annotated[str, typer.Argument(help="The signal analyzer platform being used. Must be one of the following: " + ", ".join(f"'{key}'" for key in PLATFORMS.keys()), callback=valid_platforms)]):
    configs = load_config_section("platform")
    configs["hw"] = platform
    save_config_section("platform", configs)


def main():
    CONFIG_DIR.mkdir(exist_ok=True)
    app()


if __name__ == "__main__":
    main()
