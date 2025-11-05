from __future__ import annotations
from ctypes import cdll
from pathlib import Path

# Needed bc modifying the .so file is against the Signal Hound API EULA
_site_package = Path(__file__).parent.resolve() / "lib"
cdll.LoadLibrary(f"{_site_package}/libftd2xx.so")

from ._signal_hound import dev_cnt

__all__ = ["dev_cnt"]
