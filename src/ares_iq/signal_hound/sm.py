from ares_iq_ext.signal_hound import SmDeviceType, SmGpsPlatformModel, _SmConfigs, _SmDevice, _SM, get_device_list, \
    get_networked_device_list, get_device_list2, get_networked_device_list2, broadcast_network_config, \
    retrieve_networked_configurations, configure_networked_device, _SmNetworkConfig, HOST_ADDR_ANY, DEFAULT_DEV_ADDR, \
    DEFAULT_PORT, SM_LOGGER_NAME, SM_MAX_IQ_DECIMATION, SmGPSState
from ares_iq.iq_data import IQData
from attrs import define, field, validators
from ares_iq.validators import power_of_two, validate_bounds
from ares_iq.typing import QuantizedData
from ares_iq.configs import ConfigBase
from enum import Enum
import logging
from ctypes import c_uint16
from dataclasses import dataclass

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
    PORTABLE = SmGpsPlatformModel.PORTABLE
    STATIONARY = SmGpsPlatformModel.STATIONARY
    PEDESTRIAN = SmGpsPlatformModel.PEDESTRIAN
    AUTOMOTIVE = SmGpsPlatformModel.AUTOMOTIVE
    SEA = SmGpsPlatformModel.AT_SEA
    AIRBORNE_1G = SmGpsPlatformModel.AIRBORNE_1G
    AIRBORNE_2G = SmGpsPlatformModel.AIRBORNE_2G


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
    if isinstance(x, SmGpsPlatformModel):
        return x
    raise TypeError(f"Unable to cast from {type(x)} to SmGpsPlatformModel")


@define
class SMConfigs(ConfigBase):
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


class SM:
    def __init__(self, model: SmDeviceType, configs: SMConfigs | None = None, serial: int = -1):
        if configs is None:
            configs_ = _SmConfigs(type=model)
        else:
            configs_ = _SmConfigs(type=model,
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
            tuple[list[IQData], list[QuantizedData]]:
        iq_data, timestamps, gps_info = self._dev.capture_iq(center, bw, capture_size, silent, verbose)

        iq_data_ = [IQData() for _ in timestamps]
        for data, ts, gps, iq in zip(iq_data, timestamps, gps_info, iq_data_):
            iq.iq = data
            ts_sec, ts_nsec = self._generate_ts(ts, gps['sec_since_epoch'])
            iq.ts_sec = ts_sec
            iq.ts_nsec = ts_nsec

        quant_data = self._quantize(iq_data_)
        return iq_data_, quant_data

    def _quantize(self, iq_data: list[IQData]) -> list[QuantizedData]:
        return [QuantizedData() for _ in iq_data]

    def _generate_ts(self, ts: int, sec_since_epoch: int) -> tuple[int, int]:
        # TODO
        if self._gps_stamping:
            return 0, ts
        return 0, ts

    def acquire_gps_lock(self, target_lock_state: SmGPSState, timeout: int = 0):
        locked = self._dev.gps_sync(target_lock_state, timeout)
        if not locked:
            raise TimeoutError("Unable to acquire GPS lock")


class NetworkedSM(SM):
    def network_speed_test(self, duration: float):
        return self._dev.network_speed_test(duration)


class SM200A(SM):
    def __init__(self, configs: SMConfigs | None = None, serial: int = -1):
        super().__init__(SmDeviceType.SM200A, configs, serial)


class SM200B(SM):
    def __init__(self, configs: SMConfigs | None = None, serial: int = -1):
        super().__init__(SmDeviceType.SM200B, configs, serial)


class SM200C(NetworkedSM):
    def __init__(self, configs: SMConfigs | None = None, serial: int = -1):
        super().__init__(SmDeviceType.SM200C, configs, serial)


class SM435B(SM):
    def __init__(self, configs: SMConfigs | None = None, serial: int = -1):
        super().__init__(SmDeviceType.SM435B, configs, serial)


class SM435C(NetworkedSM):
    def __init__(self, configs: SMConfigs | None = None, serial: int = -1):
        super().__init__(SmDeviceType.SM435C, configs, serial)


@dataclass(frozen=True)
class SmDevice:
    serial: int
    type: SmDeviceType


def get_sm_device_serials(usb: bool = True, networked: bool = True, max_network_devs: int = 9) -> tuple[int, ...]:
    """Retrieve a list of connected SM devices.

    Args:
        usb: Flag to indicate whether USB SM devices should be retrieved or not.
        networked: Flag to indicate whether networked SM devices should be retrieved or not.

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


def get_sm_device_list(usb: bool = True, networked: bool = True, host: str | None = None) -> tuple[SmDevice, ...]:
    """Retrieve a list of connected SM devices.

    Args:
        usb: Flag to indicate whether USB SM devices should be retrieved or not.
        networked: Flag to indicate whether networked SM devices should be retrieved or not.
        host: The host address to search for networked devices on.

    Returns:
        A tuple of the SM device serial numbers and their types.

    Notes:
        The networked device serial numbers are retrieved through the configuration USB. Thus,
        in order to retrieve networked devices, they need to be connected to USB as well.
    """

    if not usb and not networked:
        return tuple()

    usb_devs: tuple[_SmDevice, ...] = tuple()
    network_devs: tuple[_SmDevice, ...] = tuple()

    if networked:
        if host is None:
            network_devs = get_networked_device_list2()
        else:
            network_devs = get_networked_device_list2(host)
    if usb:
        usb_devs = get_device_list2()

    return tuple(
        [SmDevice(dev.serial, dev.type) for dev in network_devs] + [SmDevice(dev.serial, dev.type) for dev in usb_devs])


@dataclass()
class SmNetworkConfig:
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
    """
    config_ = _SmNetworkConfig(ip=config.ip, port=config.port)
    if host is None:
        broadcast_network_config(config_, non_volatile=non_volatile)
    else:
        broadcast_network_config(config_, host, non_volatile)
