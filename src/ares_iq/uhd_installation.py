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

try:
    import pip
except ImportError:
    install_pip()
    os.execv(sys.executable, [sys.executable] + sys.argv)


URL = "https://github.com/EttusResearch/uhd/archive/refs/tags/v4.9.0.0.tar.gz"
DIR = Path("uhd-4.9.0.0")
BUILD_DIR = DIR / "host/build"


def _download_repo():
    response = requests.get(URL)
    with open('uhd.tar.gz', 'wb') as f:
        f.write(response.content)


def _extract(cwd: Path):
    with tarfile.open(cwd / 'uhd.tar.gz', 'r:gz') as tar:
        tar.extractall()


def _generate_build(cwd: Path):
    build_dir = cwd / BUILD_DIR
    build_dir.mkdir(exist_ok=True)
    subprocess.check_call(shlex.split("cmake -DENABLE_C_API=ON -DENABLE_PYTHON_API=ON ../"), cwd=build_dir,
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
    elif line.startswith("Consolidate"):
        return f"[bold magenta]{line}[/bold magenta]"
    return line


def _build(cwd: Path, console: Console):
    build_dir = cwd / BUILD_DIR
    env = os.environ.copy()
    env["TERM"] = "xterm-256color"
    p = subprocess.Popen(
        shlex.split("make"),
        cwd=build_dir,
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


def _install(cwd: Path):
    python_dir = cwd / BUILD_DIR / "python"
    subprocess.check_call(shlex.split(f"{sys.executable} -m pip install ."), cwd=python_dir, stdout=subprocess.DEVNULL)


def _cleanup(cwd: Path):
    tar = cwd / "uhd.tar.gz"
    tar.unlink(missing_ok=True)


def install_uhd():
    print("Missing the UHD driver")
    cwd = Path.cwd()
    console = Console()
    progress = Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}"), transient=True)
    with Live(progress, console=console, refresh_per_second=100, transient=True):
        task_id = progress.add_task("Downloading UHD repository...", total=None)
        _download_repo()
        progress.update(task_id, description="Extracting contents...")
        _extract(cwd)
        progress.update(task_id, description="Building driver...")
        _generate_build(cwd)
        _build(cwd, console)
        progress.update(task_id, description="Installing driver...")
        _install(cwd)
        progress.update(task_id, description="Cleaning up...")
        _cleanup(cwd)
        progress.stop()
