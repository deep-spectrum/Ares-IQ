# Reasons for Writing Python Bindings

This document contains the reasons why Python extensions were written
for specific platforms. This is necessary to avoid wasting time in the
future to get a pure Python version working.

To add to this document, add a new section with the platform name and
list the reasons for creating bindings for the platform.

## USRP

1.  Python API not published to PyPI for Linux at time of writing.
2.  Requires a non-standard installation process which involves running
    cmake and running `pip install <python build dir>`. Not ideal for
    modern Python environment tools like
    [uv](https://docs.astral.sh/uv/).
3.  Python implementation performance issues. The Python implementation
    was collecting \<20 kS/sec over 1GbE. The C++ implementation is able
    to collect the full 25 MS/sec over 1GbE.
