from .bbdevice.bb_api import *
from iq_capture.print_utils import print_warning, print_error, CaptureProgress
from iq_capture.configurations import load_config_section, save_config_section
import typer
from typing_extensions import Annotated
from iq_capture.iq_data import IQData
import math

SAMPLES_PER_CAPTURE = 262144
BYTES_PER_CAPTURE = (16 * SAMPLES_PER_CAPTURE) + 8


class BB60Device:
    _handle: object = None
    _max_bw: float = 0
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

    def capture_iq(self, center: float, bw: float, file_size_gb: float) -> None:
        self._bw = bw
        self._center = center

        self._open_device()
        self._configure_bb_device()
        bb_initiate(self._handle, BB_STREAMING, BB_STREAM_IQ)

        file_size = file_size_gb * 1e9
        captures = math.ceil(file_size / BYTES_PER_CAPTURE)
        self._iq_data = [IQData() for _ in range(captures)]

        with CaptureProgress(captures, SAMPLES_PER_CAPTURE) as progress:
            for iq in self._iq_data:
                data = bb_get_IQ_unpacked(self._handle, SAMPLES_PER_CAPTURE, BB_FALSE)
                iq.iq = data["iq"]
                iq.ts_sec = data["sec"]
                iq.ts_nsec = data["nano"]
                progress.update()
            progress.update()

        # print(f"\nCaptured {SAMPLES_PER_CAPTURE * captures} samples @ {(SAMPLES_PER_CAPTURE * captures) / time_diff / 1e6} megasamples/second")

        self._quantize()

        bb_close_device(self._handle)

    @staticmethod
    @app.command(name='bb60-config', help='Set default configurations for the BB60')
    def config(ref_level: Annotated[float | None, typer.Option(help='Reference level of the BB60')] = None,
               decimation: Annotated[int | None, typer.Option(help='Downsample factor')] = None):
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
