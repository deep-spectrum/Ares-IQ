from .smdevice.sm_api import *
import numpy as np
import typer
from typing_extensions import Annotated


def sm200_stream_iq(center_freq: float, bw: float):
    pass


app = typer.Typer()

@app.command(name="sm200-config")
def sm200_config(decimation: Annotated[int, typer.Argument(help='Downsample factor')]):
    pass
