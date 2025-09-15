import subprocess
from pathlib import Path
import requests
import tarfile
import shlex
import sys
from rich.progress import Progress, SpinnerColumn, TextColumn
import shutil


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


def _build(cwd: Path):
    build_dir = cwd / BUILD_DIR
    build_dir.mkdir(exist_ok=True)
    subprocess.check_call(shlex.split("cmake -DENABLE_C_API=ON -DENABLE_PYTHON_API=ON ../"), cwd=build_dir, stdout=subprocess.DEVNULL)
    subprocess.check_call(shlex.split("make"), cwd=build_dir, stdout=subprocess.DEVNULL)


def _install(cwd: Path):
    python_dir = cwd / BUILD_DIR / "python"
    subprocess.check_call(shlex.split(f"{sys.executable} -m pip install ."), cwd=python_dir, stdout=subprocess.DEVNULL)


def _cleanup(cwd: Path):
    shutil.rmtree(cwd / DIR)
    tar = cwd / "uhd.tar.gz"
    tar.unlink(missing_ok=True)


def install_uhd():
    print("Missing the UHD driver")
    cwd = Path.cwd()
    with Progress(SpinnerColumn(), TextColumn("[progress.description]{task.description}"), transient=True) as progress:
        progress.add_task("Downloading UHD repository...", total=None)
        _download_repo()
        progress.add_task("Extracting contents...", total=None)
        _extract(cwd)
        progress.add_task("Building driver...", total=None)
        _build(cwd)
        progress.add_task("Installing driver...", total=None)
        _install(cwd)
        progress.add_task("Cleaning up...", total=None)
        _cleanup(cwd)
