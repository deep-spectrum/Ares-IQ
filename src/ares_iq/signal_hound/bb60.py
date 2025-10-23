from .bbdevice.bb_api import (bb_get_serial_number_list_2, bb_open_device, bb_configure_ref_level,
                              bb_configure_gain_atten, bb_configure_IQ_center, bb_configure_IQ, bb_initiate,
                              bb_get_IQ_unpacked, bb_close_device, BB_DEVICE_BB60A,
                              BB60A_MAX_RT_SPAN, BB60C_MAX_RT_SPAN, BB_AUTO_GAIN, BB_AUTO_ATTEN, BB_MIN_DECIMATION,
                              BB_MAX_DECIMATION, BB_STREAMING, BB_STREAM_IQ, BB_FALSE)
from ares_iq.print_utils import print_warning, print_error, CaptureProgress
from ares_iq.iq_data import IQData
import math
from attrs import define, field, Converter
from ares_iq.validators import clamp_bounds, power_of_two
from ares_iq.typing import RealNumber, QuantizedData

SAMPLES_PER_CAPTURE = 262144
BYTES_PER_CAPTURE = (16 * SAMPLES_PER_CAPTURE) + 8


@define
class BB60Configs:
    """Configurations for the Signal Hound BB60 spectrum analyzer.

    Attributes:
        ref_level: Reference level for the BB60 in dBm.
        decimation: The downsampling factor. Must be a power of 2 between `BB_MIN_DECIMATION` and `BB_MAX_DECIMATION`.
    """
    ref_level: RealNumber = -20.0
    decimation: int = field(default=BB_MIN_DECIMATION,
                            metadata={"min": BB_MIN_DECIMATION, "max": BB_MAX_DECIMATION},
                            converter=Converter(clamp_bounds, takes_field=True),  # type: ignore[misc]
                            validator=power_of_two)


class BB60:
    """Signal Hound BB60 spectrum analyzer interface.

    Interface for a Signal Hound BB60 spectrum analyzer.
    """
    _handle: object = None
    _max_bw: float = 0
    _center: float = 0
    _bw: float = 0
    _iq_data: list[IQData] = []
    _quantized_data: list[QuantizedData] = []

    def __init__(self, configs: BB60Configs | None = None):
        """Initializes the BB60 instance based on the configurations passed in through BB60Configs.

        Args:
            configs: Defines the configurations for the BB60. If None, then the default configurations are used.
        """
        if configs is None:
            configs = BB60Configs()
        self._configs = configs

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

    def _configure_bb_device(self):
        # Reference level
        bb_configure_ref_level(self._handle, self._configs.ref_level)

        # Gain and attenuation
        bb_configure_gain_atten(self._handle, BB_AUTO_GAIN, BB_AUTO_ATTEN)

        # Center frequency
        bb_configure_IQ_center(self._handle, self._center)

        # Bandwidth
        decimation = self._configs.decimation
        self._max_bw = self._max_bw / decimation
        if self._bw > self._max_bw:
            print_warning(
                f"Unable to set the bandwidth to {self._bw / 1.0e6} MHz. Setting to {self._max_bw / 1.0e6} MHz")
            self._bw = self._max_bw
        bb_configure_IQ(self._handle, decimation, self._bw)

    def capture_iq(self, center: float, bw: float, file_size_gb: float, verbose: bool, extra: bool) -> None:
        """Capture I/Q data from the spectrum analyzer.

        Args:
            center: The center frequency in Hz.
            bw: The capture bandwidth in Hz.
            file_size_gb: The amount of uncompressed data to capture.
            verbose: Show progress bar during the capture.
            extra: Like verbose, but also shows logging messages.
        """
        self._bw = bw
        self._center = center

        self._open_device()
        self._configure_bb_device()
        bb_initiate(self._handle, BB_STREAMING, BB_STREAM_IQ)

        # Pre-allocate to avoid doing it later...
        file_size = file_size_gb * 1e9
        captures = math.ceil(file_size / BYTES_PER_CAPTURE)
        self._iq_data = [IQData() for _ in range(captures)]

        with CaptureProgress(captures, SAMPLES_PER_CAPTURE, not (verbose or extra)) as progress:
            for iq in self._iq_data:
                data = bb_get_IQ_unpacked(self._handle, SAMPLES_PER_CAPTURE, BB_FALSE)
                iq.iq = data["iq"]
                iq.ts_sec = data["sec"]
                iq.ts_nsec = data["nano"]
                progress.update()
            progress.update()

        self._quantize()

        bb_close_device(self._handle)

    @property
    def iq_data(self):
        """The captured IQ data from the last call to BB60.capture_iq()."""
        return self._iq_data

    @property
    def quantized_data(self):
        """The quantized data from the data capture."""
        return self._quantized_data

    def _quantize(self):
        pass
