import subprocess
import shlex
import sys


def _install_pip():
    subprocess.check_call(shlex.split(f"{sys.executable} -m ensurepip --upgrade"))


def _install_setuptools():
    subprocess.check_call(shlex.split(f"{sys.executable} -m pip install --upgrade setuptools"))


def install_pip():
    print("Missing pip. Now installing it")
    _install_pip()
    _install_setuptools()
