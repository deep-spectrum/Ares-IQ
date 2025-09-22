from .bbdevice.bb_api import *
import numpy as np
from iq_capture.print_utils import print_warning, print_error
from iq_capture.configurations import load_config_section, save_config_section
import typer
from typing_extensions import Annotated
from iq_capture.iq_data import IQData

SAMPLES_PER_CAPTURE = 262144


class BB60Device:
    _handle: object = None
    _max_bw: float = None
    _center: float = 0
    _bw: float = 0
    _iq_data: list[IQData] = []
    _quantized_data: list[None] = []
    app = typer.Typer()

    @staticmethod
    def _print_bb_error(err: BBDeviceError, config_name: str):
        s = f"{config_name}: {str(err)}"
        if err.warning:
            print_warning(s)
        else:
            print_error(s)

    def _open_device(self):
        devices = bb_get_serial_number_list_2()
        device_count = devices["device_count"].value
        if device_count == 0:
            print_error("No BB60 devices found")
        elif device_count > 1:
            print_error("Multiple BB60 devices found. Please connect 1 device only")

        max_bw = BB60A_MAX_RT_SPAN if devices["device_types"][0] == BB_DEVICE_BB60A else BB60C_MAX_RT_SPAN
        self._handle = bb_open_device()["handle"]
        self._max_bw = max_bw.value

    def _call_config_func(self, func, config_name, *args):
        try:
            func(self._handle, *args)
        except BBDeviceError as e:
            self._print_bb_error(e, config_name)

    def _configure_bb_device(self):
        configs = load_config_section("bb60-configs")

        # Reference level
        ref_level = -20.0
        if 'ref-level' in configs:
            ref_level = float(configs['ref-level'])
        self._call_config_func(bb_configure_ref_level, "Reference level", ref_level)

        # Gain and attenuation
        bb_configure_gain_atten(self._handle, BB_AUTO_GAIN, BB_AUTO_ATTEN)

        # Center frequency
        self._call_config_func(bb_configure_IQ_center, "Center Frequency", self._center)

        # Bandwidth
        decimation = BB_MIN_DECIMATION
        if 'decimation' in configs:
            decimation = int(configs['decimation'])
        self._max_bw = self._max_bw / decimation
        if self._bw > self._max_bw:
            print_warning(f"Unable to set the bandwidth to {self._bw / 1.0e6} MHz. Setting to {self._max_bw / 1.0e6} MHz")
            self._bw = self._max_bw
        self._call_config_func(bb_configure_IQ, "Bandwidth", decimation, self._bw)

    def capture_iq(self, center: float, bw: float, file_size: int):
        self._bw = bw
        self._center = center

        self._open_device()
        self._configure_bb_device()
        bb_initiate(self._handle, BB_STREAMING, BB_STREAM_IQ)

        # TODO: get IQ
        while True:
            iq = bb_get_IQ_unpacked(self._handle, SAMPLES_PER_CAPTURE, BB_FALSE)
            print(len(iq))
            break

        self._quantize()

        bb_close_device(self._handle)

    @app.command(name='bb60-configs')
    def config(self,
               ref_level: Annotated[float, typer.Option(help='Reference level of the BB60')] = None,
               decimation: Annotated[int, typer.Option(help='Downsample factor')] = None):
        configs = load_config_section("bb60-configs")
        if ref_level is not None:
            configs['ref-level'] = str(ref_level)
        if decimation is not None:
            configs['decimation'] = str(decimation)
        save_config_section("bb60-configs", configs)

    @property
    def iq_data(self):
        return self._iq_data

    @property
    def quantized_data(self):
        return self._quantized_data

    def _quantize(self):
        pass
