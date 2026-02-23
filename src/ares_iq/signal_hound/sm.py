import enum

from ares_iq_ext.signal_hound import SmDeviceType, SmGpsPlatformModel, _SmConfigs, _SmDevice, _SM, get_device_list, \
    get_device_list2, broadcast_network_config, retrieve_networked_configurations, configure_networked_device, \
    _SmNetworkConfig, HOST_ADDR_ANY, DEFAULT_DEV_ADDR, DEFAULT_PORT, SM_LOGGER_NAME, SM_MAX_IQ_DECIMATION, SmGPSState, \
    sm_api_version
from ares_iq.iq_data import IQData
from attrs import define, field, validators
from ares_iq.validators import power_of_two, validate_bounds
from ares_iq.typing import QuantizedData
from ares_iq.configs import ConfigBase
from enum import Enum
import logging
from ctypes import c_uint16
from dataclasses import dataclass
from abc import ABC, abstractmethod
import datetime
import yaml
from pathlib import Path
from typing import Callable

logger = logging.getLogger(SM_LOGGER_NAME)

GPS_MODELS = {
    'portable': SmGpsPlatformModel.PORTABLE,
    'stationary': SmGpsPlatformModel.STATIONARY,
    'pedestrian': SmGpsPlatformModel.PEDESTRIAN,
    'automotive': SmGpsPlatformModel.AUTOMOTIVE,
    'at-sea': SmGpsPlatformModel.AT_SEA,
    'at_sea': SmGpsPlatformModel.AT_SEA,
    'atsea': SmGpsPlatformModel.AT_SEA,
    'sea': SmGpsPlatformModel.AT_SEA,
    'airborne-1g': SmGpsPlatformModel.AIRBORNE_1G,
    'airborne_1g': SmGpsPlatformModel.AIRBORNE_1G,
    'airborne1g': SmGpsPlatformModel.AIRBORNE_1G,
    'airborne-2g': SmGpsPlatformModel.AIRBORNE_2G,
    'airborne_2g': SmGpsPlatformModel.AIRBORNE_2G,
    'airborne2g': SmGpsPlatformModel.AIRBORNE_2G,
}


class GpsModel(Enum):
    """
    Available GPS models for SM series devices.

    Attributes:
        PORTABLE: Applications with low acceleration, e.g. portable devices. Suitable for most applications.

        STATIONARY: Used in timing applications (antenna must be stationary) or other stationary applications. Velocity
        restricted to 0 m/s. Zero dynamics assumed. This is the default setting.

        PEDESTRIAN: Applications with low acceleration and speed, how a pedestrian would move. Low acceleration assumed.

        AUTOMOTIVE: Used for applications with equivalent dynamics to those of a passenger car. Low vertical
        acceleration assumed.

        SEA: Recommended for applications at sea, with zero vertical velocity. Zero vertical velocity assumed, sea
        level assumed.

        AIRBORNE_1G: Used for applications with a higher dynamic range and greater vertical acceleration than a
        passenger car. No 2D position fixes supported.

        AIRBORNE_2G: Recommended for typical airborne environment. No 2D position fixes supported.
    """

    PORTABLE = SmGpsPlatformModel.PORTABLE
    STATIONARY = SmGpsPlatformModel.STATIONARY
    PEDESTRIAN = SmGpsPlatformModel.PEDESTRIAN
    AUTOMOTIVE = SmGpsPlatformModel.AUTOMOTIVE
    SEA = SmGpsPlatformModel.AT_SEA
    AIRBORNE_1G = SmGpsPlatformModel.AIRBORNE_1G
    AIRBORNE_2G = SmGpsPlatformModel.AIRBORNE_2G


class GpsState(Enum):
    """
    GPS state of the SM device.

    Attributes:
        NOT_PRESENT: GPS is not locked.

        LOCKED: GPS is locked, NMEA data is valid, but the timebase is not being disciplined by the GPS.

        DISCIPLINED: GPS is locked, NMEA data is valid, timebase is being disciplined by the GPS.
    """
    NOT_PRESENT = SmGPSState.NOT_PRESENT
    LOCKED = SmGPSState.LOCKED
    DISCIPLINED = SmGPSState.DISCIPLINED


def _convert_str_sm_gps(x: str) -> SmGpsPlatformModel:
    x = x.lower()
    try:
        return GPS_MODELS[x]
    except KeyError:
        raise ValueError(f"Invalid GPS model: {x}")


def _convert_sm_gps(x: object) -> SmGpsPlatformModel:
    if isinstance(x, GpsModel):
        return x.value
    if isinstance(x, str):
        return _convert_str_sm_gps(x)
    if isinstance(x, int):
        return SmGpsPlatformModel(x)
    if isinstance(x, SmGpsPlatformModel):
        return x
    raise TypeError(f"Unable to cast from {type(x)} to SmGpsPlatformModel")


@define
class SMConfigs(ConfigBase):
    """SM device configurations.

    Device configuration for SM series devices.

    Attributes:
        gps_timestamping: Generate GPS disciplined timestamps.
        gps_lock_timeout: Number number of seconds to wait for a GPS lock.
        gps_model: The GPS model to use.
        decimation: The downsampling factor. Must be a power of 2 between 1 and SM_MAX_IQ_DECIMATION.
        software_filter: Enable software filtering. Ignored on networked SM devices.
        samples_per_capture: Sample chunk size.
        host: The host address for the SM device.
        device_addr: The device address of the SM device.
        port: The port for the SM device.
    """
    gps_timestamping: bool = False
    gps_lock_timeout: int = field(default=0, metadata={"min": 0},
                                  validator=[validators.instance_of(int), validate_bounds])
    gps_model: SmGpsPlatformModel = field(default=SmGpsPlatformModel.STATIONARY, converter=_convert_sm_gps)
    decimation: int = field(default=1,
                            metadata={"min": 1, "max": SM_MAX_IQ_DECIMATION},
                            validator=[validators.instance_of(int), validate_bounds, power_of_two])
    software_filter: bool = False
    samples_per_capture: int = field(default=500000, metadata={"min": 1},
                                     validator=[validators.instance_of(int), validate_bounds])
    host: str = HOST_ADDR_ANY
    device_addr: str = DEFAULT_DEV_ADDR
    port: int = field(default=DEFAULT_PORT, metadata={"min": 0, "max": c_uint16(-1).value},
                      validator=[validators.instance_of(int), validate_bounds])

    @staticmethod
    def serialize(inst, field_, value):
        if field_.type == SmGpsPlatformModel:
            return value.value
        return value


@dataclass(frozen=True)
class SmGpsInfo:
    sec_since_epoch: int
    latitude: float
    longitude: float
    altitude: float


class SM(ABC):
    """Base class for SM platforms"""

    def __init__(self, model: SmDeviceType, configs: SMConfigs | None = None, serial: int = -1):
        """Initializes the base SM device.

        Args:
            model: The SM device model
            configs: The configurations for the SM device.
            serial: The serial number to connect to. Only relevant for USB SM devices.
        """
        if configs is None:
            configs_ = _SmConfigs(device=model)
        else:
            configs_ = _SmConfigs(device=model,
                                  serial=serial,
                                  gps_timestamping=configs.gps_timestamping,
                                  gps_lock_timeout=configs.gps_lock_timeout,
                                  gps_model=configs.gps_model,
                                  decimation=configs.decimation,
                                  software_filter=configs.software_filter,
                                  samples_per_capture=configs.samples_per_capture,
                                  host=configs.host,
                                  device_addr=configs.device_addr,
                                  port=configs.port)
        self._dev = _SM(configs_)
        self._gps_stamping = configs_.gps_timestamping

    def capture_iq(self, center: float, bw: float, capture_size: int, silent: bool = True, verbose: bool = False) -> \
            tuple[list[IQData], list[QuantizedData], list[SmGpsInfo]]:
        """Capture IQ data from the SDR.

        Args:
            center: The center frequency in Hz.
            bw: The bandwidth in Hz.
            capture_size: The maximum amount of IQ data to collect in bytes.
            silent: Do not show the progress bar.
            verbose: Show the logging messages.

        Raises:
            ValueError: Bad configuration arguments.
            RuntimeError: Any other error.

        Notes:
            This will automatically open a connection if there is no open connection.
        """
        iq_data, timestamps, gps_info = self._dev.capture_iq(center, bw, capture_size, silent, verbose)

        iq_data_ = [IQData() for _ in timestamps]
        for data, ts, gps, iq in zip(iq_data, timestamps, gps_info, iq_data_):
            iq.iq = data
            ts_sec, ts_nsec = self._generate_ts(ts, gps['sec_since_epoch'])
            iq.ts_sec = ts_sec
            iq.ts_nsec = ts_nsec

        gps_data = [SmGpsInfo(gps['sec_since_epoch'], gps['latitude'], gps['longitude'], gps['altitude']) for gps in
                    gps_info]

        quant_data = self._quantize(iq_data_)
        return iq_data_, quant_data, gps_data

    @abstractmethod
    def _quantize(self, iq_data: list[IQData]) -> list[QuantizedData]:
        """Convert the collected IQ data from complex numbers to ADC readings."""

    def _generate_ts(self, ts: int, sec_since_epoch: int) -> tuple[int, int]:
        # TODO
        if self._gps_stamping:
            return sec_since_epoch, ts
        return 0, ts

    def acquire_gps_lock(self, target_lock_state: GpsState, timeout: int = 0):
        """Acquire a GPS lock with the given GPS state.

        Acquire a GPS lock. Behavior of the given states:

        GpsState.LOCKED: Acquire the GPS locked or the GPS disciplined states.
        GpsState.DISCIPLINED: Acquire GPS disciplined time base state.

        Args:
            target_lock_state: The GPS lock state to acquire.
            timeout: The number of seconds to wait for a GPS lock to be acquired. If <= `0`, wait indefinitely.

        Raises:
            ValueError: Bad parameter for target_lock_state.
            TimeoutError: If timeout expired.

        Notes:
            This will automatically open a connection if there is no open connection.
        """
        locked = self._dev.gps_sync(target_lock_state.value, timeout)
        if not locked:
            raise TimeoutError("Unable to acquire GPS lock")

    def open(self):
        """
        Open a connection to the SM device, if one is not already open.
        """
        self._dev.open()

    def close(self):
        """
        Close a connection to the SM device, if one is open.
        """
        self._dev.close()

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def _save_stream_iq_meta(self, meta: dict[str, object | dict[str, object | datetime.timedelta]], save_directory: Path):
        configs: dict[str, object] = self._dev.get_configs().as_dict()
        meta["diagnostics"]["save_duration"] = meta["diagnostics"]["save_duration"].total_seconds()
        meta["diagnostics"]["device_diagnostics"] = self._dev.diagnostic_info().as_dict()
        meta["diagnostics"]["api_version"] = sm_api_version()
        try:
            meta["diagnostics"]["network_diagnostics"] = self._dev.network_diagnostic_info().as_dict()
        except RuntimeError:
            pass
        for key, value in configs.items():
            if issubclass(type(value), enum.IntEnum):
                configs[key] = value.name
        # Samples per a capture is already in the metadata
        del configs["samples_per_capture"]
        meta["device_configurations"] = configs
        with open(save_directory / "meta.yaml", "w") as f:
            yaml.safe_dump(meta, f)

    @staticmethod
    def _create_save_directory(save_directory: str | Path) -> Path:
        if isinstance(save_directory, str):
            save_directory = Path(save_directory)
        save_directory.mkdir(exist_ok=True)
        return save_directory

    def stream_iq(self, center: float, bw: float, chunk_size: int, duration: datetime.timedelta,
                  save_directory: str | Path, silent: bool = True, verbose: bool = False,
                  stop_sample_loss: bool = False, stop_cb: Callable[[], None] | None = None):
        """Stream I/Q data to disk.

        Args:
            center: The center frequency in Hz.
            bw: The capture bandwidth in Hz.
            chunk_size: The file chunk size in bytes.
            duration: The amount of time to stream I/Q for.
            save_directory: The directory to save the I/Q data, timestamps, and metadata to. If this directory does
                            not exist, then this method will attempt to create the specified directory.
            silent: Run the streamed capture in silent mode (no status bars). By default, this is `True`.
            verbose: Run the streamed capture in verbose mode (info logging messages). By default, this is `False`.
            stop_sample_loss: Stop the streamed capture if sample loss starts occurring. By default, this is `False`.
            stop_cb: User callback for notifying when the streamed capture is done. By default, this is `None`.
        """
        save_directory = self._create_save_directory(save_directory)

        def done():
            if stop_cb is not None:
                stop_cb()

        meta = self._dev.stream_iq(center, bw, chunk_size, duration, str(save_directory), silent, verbose,
                                   stop_sample_loss, done)
        meta["parameters"] = {
            "center_frequency": center,
            "bandwidth": bw,
            "capture_duration": duration.total_seconds(),
            "stop_if_sample_loss": stop_sample_loss,
        }
        self._save_stream_iq_meta(meta, save_directory)


@dataclass(frozen=True)
class SmSFPDiagnostics:
    """Diagnostic information for the SFP+ port

    Attributes:
        temperature: Reported SFP+ temperature in C.
        voltage: Reported SFP+ voltage in V.
        tx_power: Reported transmit power in mW.
        rx_power: Reported receive power in mW.
    """
    temperature: float
    voltage: float
    tx_power: float
    rx_power: float


class NetworkedSM(SM, ABC):
    """Base class for networked SM devices"""

    def network_speed_test(self, duration: float) -> float:
        """Perform a network speed test for the connected device.

        Args:
            duration: The amount of seconds to run the speed test for.

        Returns:
            The amount of bytes/second recorded by the speed test.
        """
        return self._dev.network_speed_test(duration)

    def sfp_diagnostics(self) -> SmSFPDiagnostics:
        data = self._dev.network_diagnostic_info()
        return SmSFPDiagnostics(data.temp, data.voltage, data.tx_power, data.rx_power)


class SM200A(SM):
    """SM200A device."""

    def __init__(self, configs: SMConfigs | None = None, serial: int = -1):
        super().__init__(SmDeviceType.SM200A, configs, serial)

    def _quantize(self, iq_data: list[IQData]) -> list[QuantizedData]:
        return [QuantizedData() for _ in iq_data]


class SM200B(SM):
    """SM200B device."""

    def __init__(self, configs: SMConfigs | None = None, serial: int = -1):
        super().__init__(SmDeviceType.SM200B, configs, serial)

    def _quantize(self, iq_data: list[IQData]) -> list[QuantizedData]:
        return [QuantizedData() for _ in iq_data]


class SM200C(NetworkedSM):
    """SM200C device."""

    def __init__(self, configs: SMConfigs | None = None, serial: int = -1):
        super().__init__(SmDeviceType.SM200C, configs, serial)

    def _quantize(self, iq_data: list[IQData]) -> list[QuantizedData]:
        return [QuantizedData() for _ in iq_data]


class SM435B(SM):
    """SM435B device."""

    def __init__(self, configs: SMConfigs | None = None, serial: int = -1):
        super().__init__(SmDeviceType.SM435B, configs, serial)

    def _quantize(self, iq_data: list[IQData]) -> list[QuantizedData]:
        return [QuantizedData() for _ in iq_data]


class SM435C(NetworkedSM):
    """SM435C device."""

    def __init__(self, configs: SMConfigs | None = None, serial: int = -1):
        super().__init__(SmDeviceType.SM435C, configs, serial)

    def _quantize(self, iq_data: list[IQData]) -> list[QuantizedData]:
        return [QuantizedData() for _ in iq_data]


@dataclass(frozen=True)
class SmDevice:
    """SM device information

    Attributes:
        serial: The serial number.
        type: The device type.
    """
    serial: int
    type: SmDeviceType


def sm_get_device_serials(usb: bool = True, networked: bool = True, max_network_devs: int = 9) -> tuple[int, ...]:
    """Retrieve a list of connected SM devices.

    Args:
        usb: Flag to indicate whether USB SM devices should be retrieved or not.
        networked: Flag to indicate whether networked SM devices should be retrieved or not.
        max_network_devs: The maximum number of networked devices to scan for. Must be > 0.

    Returns:
        A tuple of serial numbers

    Notes:
        The networked device serial numbers are retrieved through the configuration USB. Thus,
        in order to retrieve networked devices, they need to be connected to USB as well.
    """

    if not usb and not networked:
        return tuple()

    if max_network_devs <= 0:
        raise ValueError("`max_network_devs` must be a positive integer")

    return get_device_list(max_network_devs, usb, networked)


def sm_get_device_list(usb: bool = True, networked: bool = True, max_network_devices: int = 9,
                       host: str | None = None) -> tuple[SmDevice, ...]:
    """Retrieve a list of connected SM devices.

    Args:
        usb: Flag to indicate whether USB SM devices should be retrieved or not.
        networked: Flag to indicate whether networked SM devices should be retrieved or not.
        max_network_devices: The maximum number of networked devices to scan for. Must be > 0.
        host: The host address to search for networked devices on.

    Returns:
        A tuple of the SM device serial numbers and their types.

    Notes:
        The networked device serial numbers are retrieved through the configuration USB. Thus,
        in order to retrieve networked devices, they need to be connected to USB as well.
    """

    if not usb and not networked:
        return tuple()

    if max_network_devices <= 0:
        raise ValueError("`max_network_devices` must be > 0")

    if host is None:
        devs: tuple[_SmDevice, ...] = get_device_list2(max_network_devices, usb, networked)
    else:
        devs: tuple[_SmDevice, ...] = get_device_list2(max_network_devices, usb, networked, host)
    return tuple([SmDevice(dev.serial, dev.type) for dev in devs])


@dataclass()
class SmNetworkConfig:
    """SM device network configurations.

    Attributes:
        mac: The MAC address of the SM device.
        ip: The device IP address.
        port: The device network port.
        serial: The network device serial number.
    """
    mac: str = ""
    ip: str = DEFAULT_DEV_ADDR
    port: int = DEFAULT_PORT
    serial: int = -1


def sm_get_network_config(serial: int) -> SmNetworkConfig:
    """Get the specified networked SM device configurations over USB 2.0.

    Args:
        serial: The serial number of the networked device.

    Returns:
        The SM device network configuration.

    Notes:
        The networked device must be connected over USB 2.0 for this function to work.
    """
    config_ = retrieve_networked_configurations(serial)
    return SmNetworkConfig(mac=config_.mac, ip=config_.ipaddr, port=config_.port, serial=serial)


def sm_configure_network_device(serial: int, config: SmNetworkConfig, non_volatile: bool = False):
    """Configure the specified networked SM device.

    Args:
        serial: The serial number of the networked device.
        config: The new configuration for the specified networked device.
        non_volatile: Flag to indicate whether the config should persist through power cycles.

    Notes:
        The `ip` and `port` fields in `SmNetworkConfig` are the only fields used for configuration.
        The mac and the serial fields are immutable.
        The networked device must be connected over USB 2.0 for this function to work.
    """
    config_ = _SmNetworkConfig(ip=config.ip, port=config.port)
    configure_networked_device(serial, config_, non_volatile)


def sm_broadcast_network_config(config: SmNetworkConfig, host: str | None = None, non_volatile: bool = False):
    """Broadcast a network configuration to all network devices.

    Args:
        config: The new configuration for the networked devices.
        host: The host address to send the network configuration on.
        non_volatile: Flag to indicate whether the config should persist through power cycles.

    Notes:
        The `ip` and `port` fields in `SmNetworkConfig` are the only fields used for configuration.
        The mac and the serial fields are immutable.
        The networked device must be connected over USB 2.0 for this function to work.
    """
    config_ = _SmNetworkConfig(ip=config.ip, port=config.port)
    if host is None:
        broadcast_network_config(config_, non_volatile=non_volatile)
    else:
        broadcast_network_config(config_, host, non_volatile)
