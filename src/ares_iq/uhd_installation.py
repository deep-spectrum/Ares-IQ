import subprocess
from pathlib import Path
import requests
import tarfile
import shlex
import sys
from rich.progress import Progress, SpinnerColumn, TextColumn
from rich.console import Console
from rich.live import Live
import os
from .install_pip import install_pip
from .configurations import CONFIG_DIR

try:
    import pip
except ImportError:
    install_pip()
    os.execv(sys.executable, [sys.executable] + sys.argv)


DOWNLOAD_DIR = CONFIG_DIR
URL = "https://github.com/EttusResearch/uhd/archive/refs/tags/v4.9.0.0.tar.gz"
DIR = DOWNLOAD_DIR / Path("uhd-4.9.0.0")
BUILD_DIR = DIR / "host/build"


def _download_repo():
    if (DOWNLOAD_DIR / 'uhd.tar.gz').exists():
        return
    response = requests.get(URL)
    with open(DOWNLOAD_DIR / 'uhd.tar.gz', 'wb') as f:
        f.write(response.content)


def _extract():
    with tarfile.open(DOWNLOAD_DIR / 'uhd.tar.gz', 'r:gz') as tar:
        tar.extractall(DOWNLOAD_DIR)


def _generate_build():
    BUILD_DIR.mkdir(exist_ok=True)
    subprocess.check_call(shlex.split("cmake -DENABLE_C_API=ON -DENABLE_PYTHON_API=ON ../"), cwd=BUILD_DIR,
                          stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


KEYWORD_STYLE = {
    "Generating": "blue",
    "Building": "green",
    "Linking": "bold green"
}


def _style_line(line: str) -> str:
    if line.startswith("["):
        try:
            prefix, rest = line.split("] ", 1)
            bracket = f"{prefix}]"
            key, _ = rest.split(" ", 1)
        except ValueError:
            return line
        if key not in KEYWORD_STYLE:
            return line
        return f"{bracket} [{KEYWORD_STYLE[key]}]{rest}[/{KEYWORD_STYLE[key]}]"
    else:
        return line


def _build(console: Console):
    env = os.environ.copy()
    env["TERM"] = "xterm-256color"
    p = subprocess.Popen(
        shlex.split("make"),
        cwd=BUILD_DIR,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        bufsize=1,
        universal_newlines=True,
        env=env,
    )

    for line in p.stdout:
        console.print(_style_line(line.rstrip()), markup=True, highlight=False)

    if p.wait() != 0:
        console.print("[red]Failed to install the UHD package. Please make sure 'pybind11' is not installed on your machine.")
        exit(p.returncode)


def _install():
    python_dir = BUILD_DIR / "python"
    subprocess.check_call(shlex.split(f"{sys.executable} -m pip install ."), cwd=python_dir, stdout=subprocess.DEVNULL)


def install_uhd():
    print("Missing the UHD driver")
    console = Console()
    progress = Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}"), transient=True)
    with Live(progress, console=console, refresh_per_second=100, transient=True):
        task_id = progress.add_task("Downloading UHD repository...", total=None)
        _download_repo()
        progress.update(task_id, description="Extracting contents...")
        _extract()
        progress.update(task_id, description="Building driver...")
        _generate_build()
        _build(console)
        progress.update(task_id, description="Installing driver...")
        _install()
        progress.stop()
