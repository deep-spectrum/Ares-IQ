from .bbdevice.bb_api import (bb_get_serial_number_list_2, bb_open_device, bb_configure_ref_level,
                              bb_configure_gain_atten, bb_configure_IQ_center, bb_configure_IQ, bb_initiate,
                              bb_get_IQ_unpacked, bb_close_device, BB_DEVICE_BB60A,
                              BB60A_MAX_RT_SPAN, BB60C_MAX_RT_SPAN, BB_AUTO_GAIN, BB_AUTO_ATTEN, BB_MIN_DECIMATION,
                              BB_MAX_DECIMATION, BB_STREAMING, BB_STREAM_IQ, BB_FALSE)
from ares_iq.print_utils import CaptureProgress
from ares_iq.iq_data import IQData
from ares_iq.print_utils import logging as aiq_logging
import math
from attrs import define, field, validators
from ares_iq.validators import power_of_two, validate_bounds
from ares_iq.typing import QuantizedData
import logging

SAMPLES_PER_CAPTURE = 262144
BYTES_PER_CAPTURE = (16 * SAMPLES_PER_CAPTURE) + 8
logger = logging.getLogger(__name__)


class BB60Exception(Exception):
    """Exception for BB60 related errors"""


@define
class BB60Configs:
    """Configurations for the Signal Hound BB60 spectrum analyzer.

    Attributes:
        ref_level: Reference level for the BB60 in dBm.
        decimation: The downsampling factor. Must be a power of 2 between `BB_MIN_DECIMATION` and `BB_MAX_DECIMATION`.
    """
    ref_level: float = -20.0
    decimation: int = field(default=BB_MIN_DECIMATION,
                            metadata={"min": BB_MIN_DECIMATION, "max": BB_MAX_DECIMATION},
                            validator=[validators.instance_of(int), validate_bounds, power_of_two])


class BB60:
    """Signal Hound BB60 spectrum analyzer interface.

    Interface for a Signal Hound BB60 spectrum analyzer.
    """
    _handle: object = None
    _max_bw: float = 0
    _center: float = 0
    _bw: float = 0

    def __init__(self, configs: BB60Configs | None = None):
        """Initializes the BB60 instance based on the configurations passed in through BB60Configs.

        Args:
            configs: Defines the configurations for the BB60. If None, then the default configurations are used.
        """
        if configs is None:
            configs = BB60Configs()
        self._configs = configs
        logger.setLevel(aiq_logging.OFF)

    def _open_device(self):
        logger.debug("Discovering BB60 devices")
        devices = bb_get_serial_number_list_2()
        device_count = devices["device_count"].value
        # TODO: allow serial number in the presence of multiple devices?
        if device_count == 0:
            raise BB60Exception("No BB60 devices found")
        elif device_count > 1:
            raise BB60Exception("Multiple BB60 devices found. Please connect 1 device only")
        logger.debug("Found a BB60 device. Attempting to open it.")
        max_bw = BB60A_MAX_RT_SPAN if devices["device_types"][0] == BB_DEVICE_BB60A else BB60C_MAX_RT_SPAN
        self._handle = bb_open_device()["handle"]
        self._max_bw = max_bw.value

    def _configure_bb_device(self):
        logger.debug(f"Setting reference level to {self._configs.ref_level} dBm")
        bb_configure_ref_level(self._handle, self._configs.ref_level)

        logger.debug("Configuring the gain and attenuation")
        bb_configure_gain_atten(self._handle, BB_AUTO_GAIN, BB_AUTO_ATTEN)

        logger.debug(f"Setting the center frequency to {self._center}")
        bb_configure_IQ_center(self._handle, self._center)

        logger.debug("Setting the bandwidth and down sampling factor")
        decimation = self._configs.decimation
        self._max_bw = self._max_bw / decimation
        if self._bw > self._max_bw:
            logger.warning(
                f"Unable to set the bandwidth to {self._bw / 1.0e6} MHz. Setting to {self._max_bw / 1.0e6} MHz")
            self._bw = self._max_bw
        bb_configure_IQ(self._handle, decimation, self._bw)

    def capture_iq(self, center: float, bw: float, capture_size: int, verbose: bool = False, extra: bool = False) -> tuple[list[IQData], list[QuantizedData]]:
        """Capture I/Q data from the spectrum analyzer.

        Args:
            center: The center frequency in Hz.
            bw: The capture bandwidth in Hz.
            capture_size: The maximum amount of IQ data to collect in bytes.
            verbose: Show progress bar during the capture.
            extra: Like verbose, but also shows logging messages.
        """

        if extra:
            logger.setLevel(logging.INFO)

        self._bw = bw
        self._center = center

        logger.info("Opening and configuring BB60")
        self._open_device()
        self._configure_bb_device()

        # Pre-allocate to avoid doing it later...
        captures = math.floor(capture_size / BYTES_PER_CAPTURE)
        iq_data = [IQData() for _ in range(captures)]

        logger.info("BB60 configured. Starting stream")
        bb_initiate(self._handle, BB_STREAMING, BB_STREAM_IQ)

        with CaptureProgress(captures, SAMPLES_PER_CAPTURE, not (verbose or extra)) as progress:
            for iq in iq_data:
                data = bb_get_IQ_unpacked(self._handle, SAMPLES_PER_CAPTURE, BB_FALSE)
                iq.iq = data["iq"]
                iq.ts_sec = data["sec"]
                iq.ts_nsec = data["nano"]
                progress.update()
            progress.update()

        logger.info("Finished collecting IQ data. Quantizing the data.")
        quant_data = self._quantize(iq_data)

        bb_close_device(self._handle)

        logger.setLevel(aiq_logging.OFF)
        return iq_data, quant_data

    def _quantize(self, iq_data):
        # TODO: Implement me
        return [QuantizedData() for _ in iq_data]
