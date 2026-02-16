/**
 * @file sm_series.cpp
 *
 * @brief Implementation of the SM class, SMConfigs struct, and SMDevice struct
 * and their Python bindings.
 *
 * @date 11/5/2025
 *
 * @auther Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#include <ares-iq/signal-hound/sm.hpp>
#include <ares-iq/signal-hound/sm/sm_api.hpp>
#include <ares-iq/util.hpp>
#include <capture-progress/progress.hpp>
#include <cassert>
#include <complex>
#include <logging/log.hpp>
#include <pybind11/native_enum.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <stdexcept>
#include <vector>

namespace py = pybind11;

LOG_MODULE_REGISTER(sm_logger);

PYBIND11_MODULE(_sh_sm_series, m, py::mod_gil_not_used()) {
    py::native_enum<SmDeviceType>(m, "SmDeviceType", "enum.IntEnum")
        .value("SM200A", smDeviceTypeSM200A)
        .value("SM200B", smDeviceTypeSM200B)
        .value("SM200C", smDeviceTypeSM200C)
        .value("SM435B", smDeviceTypeSM435B)
        .value("SM435C", smDeviceTypeSM435C)
        .value("NOTSET", smDeviceTypeNotSet)
        .export_values()
        .finalize();

    py::native_enum<SmGPSPlatformModel>(m, "SmGpsPlatformModel", "enum.IntEnum")
        .value("PORTABLE", SmGPSPlatformModelPortable)
        .value("STATIONARY", SmGPSPlatformModelStationary)
        .value("PEDESTRIAN", SmGPSPlatformModelPedestrian)
        .value("AUTOMOTIVE", SmGPSPlatformModelAutomotive)
        .value("AT_SEA", SmGPSPlatformModelAtSea)
        .value("AIRBORNE_1G", SmGPSPlatformModelAirborne_1g)
        .value("AIRBORNE_2G", SmGPSPlatformModelAirborne_2g)
        .export_values()
        .finalize();

    py::native_enum<SmGPSState>(m, "SmGPSState", "enum.IntEnum")
        .value("NOT_PRESENT", smGPSStateNotPresent)
        .value("LOCKED", smGPSStateLocked)
        .value("DISCIPLINED", smGPSStateDisciplined)
        .export_values()
        .finalize();

    py::class_<SMConfigs>(m, "_SmConfigs", "SM device configs")
        .def(py::init<const py::kwargs &>())
        .def_readwrite("type", &SMConfigs::type, "The device type")
        .def_readwrite("serial", &SMConfigs::serial, "The device serial number")
        .def_readwrite("host", &SMConfigs::host,
                       "Host interface IP on which the networked device is "
                       "connected, provided as a string. Can be “0.0.0.0”. An "
                       "example parameter is “192.168.2.2”.")
        .def_readwrite("device_addr", &SMConfigs::device_addr,
                       "Target device IP provided as a string. If more than "
                       "one device with this IP is connected to the host "
                       "interface, the behavior is undefined.")
        .def_readwrite("port", &SMConfigs::port, "Target device port")
        .def_readwrite("gps_timestamping", &SMConfigs::gps_timestamping,
                       "Use GPS timestamping")
        .def_readwrite("gps_lock_timeout", &SMConfigs::gps_lock_timeout,
                       "Amount of time in seconds to wait for a GPS lock")
        .def_readwrite("gps_model", &SMConfigs::gps_model,
                       "The GPS model to use")
        .def_readwrite("decimation", &SMConfigs::decimation,
                       "The downsampling factor")
        .def_readwrite("software_filter", &SMConfigs::software_filter,
                       "Use software filtering")
        .def_readwrite("samples_per_capture", &SMConfigs::samples_per_capture,
                       "The number of samples to collect per a capture");

    py::class_<SMDevice>(m, "_SmDevice",
                         "SM device metadata from device discovery")
        .def(py::init<>())
        .def_property_readonly("serial", &SMDevice::getSerial, "Serial number")
        .def_property_readonly("type", &SMDevice::getType, "Device type");

    py::class_<SmDiagnostics>(m, "SmDiagnostics",
                              "Diagnostic information from the SM device")
        .def(py::init<>())
        .def_property_readonly("voltage", &SmDiagnostics::voltage,
                               "Device voltage")
        .def_property_readonly("current_input", &SmDiagnostics::current_input,
                               "Input current")
        .def_property_readonly("current_ocxo", &SmDiagnostics::current_ocxo,
                               "OCXO current")
        .def_property_readonly("temp_fpga_internal",
                               &SmDiagnostics::temp_fpga_internal,
                               "FPGA core/internal temp")
        .def_property_readonly("temp_fpga_near", &SmDiagnostics::temp_fpga_near,
                               "Temp near FPGA")
        .def_property_readonly("temp_ocxo", &SmDiagnostics::temp_ocxo,
                               "OCXO temperature")
        .def_property_readonly("temp_vco", &SmDiagnostics::temp_vco,
                               "VCO temperature")
        .def_property_readonly("temp_rf_board_lo",
                               &SmDiagnostics::temp_rf_board_lo,
                               "Temperature on RF board LO")
        .def_property_readonly("temp_power_supply",
                               &SmDiagnostics::temp_power_supply,
                               "Power supply temperature");

    py::class_<SmSFPDiagnostics>(
        m, "_SmSFPDiagnostics",
        "Diagnostic information of the SFP+ port from the SM device")
        .def(py::init<>())
        .def_property_readonly("temp", &SmSFPDiagnostics::get_temp,
                               "SFP+ temperature in C")
        .def_property_readonly("voltage", &SmSFPDiagnostics::get_voltage,
                               "SFP+ voltage in V")
        .def_property_readonly("tx_power", &SmSFPDiagnostics::get_tx_power,
                               "Transmit power in mW")
        .def_property_readonly("rx_power", &SmSFPDiagnostics::get_rx_power,
                               "Receive power in mW");

    py::class_<SmNetworkConfig>(m, "_SmNetworkConfig",
                                "Network configuration for/from the SM device")
        .def(py::init<const py::kwargs &>())
        .def_readonly("mac", &SmNetworkConfig::mac,
                      "The MAC address of the device")
        .def_readwrite("ipaddr", &SmNetworkConfig::ip,
                       "The IP address of the SM device")
        .def_readwrite("port", &SmNetworkConfig::port,
                       "The port of the SM device");

    py::class_<SmGpsInfo>(m, "_SmGpsInfo", "GPS information from SM device")
        .def(py::init<>())
        .def_readonly(
            "sec_since_epoch", &SmGpsInfo::sec_since_epoch,
            "Number of seconds since epoch as reported by the GPS NMEA "
            "sentences. Last reported value by the GPS. If the GPS is not "
            "locked, this value will be set to zero.")
        .def_readonly("latitude", &SmGpsInfo::latitude,
                      "Latitude in decimal degrees. If the GPS is not "
                      "locked, this value will be set to zero.")
        .def_readonly("longitude", &SmGpsInfo::longitude,
                      "Longitude in decimal degrees. If the GPS is "
                      "not locked, this value will be set to zero.")
        .def_readonly("altitude", &SmGpsInfo::altitude,
                      "Altitude in meters. If the GPS is not locked, "
                      "this value will be set to zero.");

    PYBIND11_NUMPY_DTYPE(SmGpsInfo, sec_since_epoch, latitude, longitude,
                         altitude);

    py::class_<SM>(m, "_SM", "SM series device instance")
        .def(py::init<const SMConfigs &>())
        .def("capture_iq", &SM::capture_iq, "Capture IQ data")
        .def("firmware_version", &SM::firmware_version,
             "Retrieve the device firmware info")
        .def("diagnostic_info", &SM::diagnostic_info,
             "Retrieve device diagnostic information")
        .def("gps_sync", &SM::gps_sync, "Open and acquire a GPS lock")
        .def("network_speed_test", &SM::network_speed_test,
             "Test the speed of the network")
        .def("network_diagnostic_info", &SM::network_diagnostic_info,
             "Retrieve the diagnostic information for the SFP+ port")
        .def("open", &SM::open, "Open SM device")
        .def("close", &SM::close, "Close SM device");

    m.def("sm_api_version", smGetAPIVersion, "Retrieve the SM API version");
    m.def("get_device_list", get_device_list,
          "Retrieve a list of connected SM series devices");
    m.def("get_device_list2", get_device_list2, py::arg("max_network_devices"),
          py::arg("usb"), py::arg("network"), py::arg("host") = SM200_ADDR_ANY,
          "Retrieve a list of connected SM series devices with device types");
    m.def("retrieve_networked_configurations",
          &retrieve_networked_configurations,
          "Retrieve network configurations from a certain SM device");
    m.def("configure_networked_device", &configure_networked_device,
          py::arg("serial"), py::arg("config"), py::arg("non_volatile") = false,
          "Configure the network settings for a certain SM device");
    m.def("broadcast_network_config", &broadcast_network_config,
          py::arg("config"), py::arg("hostaddr") = SM_ADDR_ANY,
          py::arg("non_volatile") = false,
          "Broadcast the network settings for SM devices on a certain host "
          "address");

    m.attr("HOST_ADDR_ANY") = SM_ADDR_ANY;
    m.attr("DEFAULT_DEV_ADDR") = SM_DEFAULT_ADDR;
    m.attr("DEFAULT_PORT") = SM_DEFAULT_PORT;
    m.attr("LOGGER_NAME") = LOG_MODULE_NAME;
    m.attr("SM_MAX_IQ_DECIMATION") = SM_MAX_IQ_DECIMATION;
}

template <typename T>
static py::tuple array_to_tuple(const T *data, size_t count) {
    py::tuple t(static_cast<py::ssize_t>(count));
    for (size_t i = 0; i < count; i++) {
        t[i] = data[i];
    }
    return t;
}

SMConfigs::SMConfigs(const py::kwargs &kwargs) {
    KWARG_TO_STRUCT_PARAM(kwargs, type);
    KWARG_TO_STRUCT_PARAM(kwargs, serial);
    KWARG_TO_STRUCT_PARAM(kwargs, host);
    KWARG_TO_STRUCT_PARAM(kwargs, device_addr);
    KWARG_TO_STRUCT_PARAM(kwargs, port);
    KWARG_TO_STRUCT_PARAM(kwargs, gps_timestamping);
    KWARG_TO_STRUCT_PARAM(kwargs, gps_lock_timeout);
    KWARG_TO_STRUCT_PARAM(kwargs, gps_model);
    KWARG_TO_STRUCT_PARAM(kwargs, decimation);
    KWARG_TO_STRUCT_PARAM(kwargs, software_filter);
    KWARG_TO_STRUCT_PARAM(kwargs, samples_per_capture);
}

int SMDevice::getSerial() const { return serial; }

SmDeviceType SMDevice::getType() const { return type; }

float SmDiagnostics::voltage() const { return diagnostics.voltage; }

float SmDiagnostics::current_input() const { return diagnostics.currentInput; }

float SmDiagnostics::current_ocxo() const { return diagnostics.currentOCXO; }

float SmDiagnostics::temp_fpga_internal() const {
    return diagnostics.tempFPGAInternal;
}

float SmDiagnostics::temp_fpga_near() const { return diagnostics.tempFPGANear; }

float SmDiagnostics::temp_ocxo() const { return diagnostics.tempOCXO; }

float SmDiagnostics::temp_vco() const { return diagnostics.tempVCO; }

float SmDiagnostics::temp_rf_board_lo() const {
    return diagnostics.tempRFBoardLO;
}

float SmDiagnostics::temp_power_supply() const {
    return diagnostics.tempPowerSupply;
}

float SmSFPDiagnostics::get_temp() const { return temp; }

float SmSFPDiagnostics::get_voltage() const { return voltage; }

float SmSFPDiagnostics::get_tx_power() const { return txPower; }

float SmSFPDiagnostics::get_rx_power() const { return rxPower; }

#define _SM_API_CALL_TRACE(api_call_) api_call_, #api_call_

static void check_sm_status(SmStatus status, const std::string &caller) {
    if (status != smNoError) {
        LOG_ERR("%s failed", caller.c_str());
        throw std::runtime_error(smGetErrorString(status));
    }
}

SmNetworkConfig::SmNetworkConfig(const py::kwargs &kwargs) {
    KWARG_TO_STRUCT_PARAM(kwargs, ip);
    KWARG_TO_STRUCT_PARAM(kwargs, port);
}

SM::SM(const SMConfigs &configs) { _configs = configs; }

SM::~SM() { _close_device(); }

py::tuple SM::capture_iq(double center, double bw, uint64_t capture_size,
                         bool silent, bool verbose) {
    py::tuple ret;
    if (verbose) {
        SAVE_LOG_LEVEL_AND_OVERRIDE(LOG_LEVEL_INFO);
    }

    try {
        ret = _capture_iq(center, bw, capture_size, silent);
    } catch (...) {
        if (verbose) {
            RESTORE_LOG_LEVEL();
        }
        throw;
    }

    if (verbose) {
        RESTORE_LOG_LEVEL();
    }

    return ret;
}

py::tuple SM::firmware_version() {
    int major, minor, revision;
    bool not_open = false;

    if (!_open) {
        _open_device();
        not_open = true;
    }

    check_sm_status(_SM_API_CALL_TRACE(
        smGetFirmwareVersion(fd, &major, &minor, &revision)));

    if (not_open) {
        _close_device();
    }

    return py::make_tuple(major, minor, revision);
}

SmDiagnostics SM::diagnostic_info() const {
    SmDiagnostics diagnostics;

    if (!_open) {
        throw std::runtime_error("Device not open");
    }

    check_sm_status(_SM_API_CALL_TRACE(
        smGetFullDeviceDiagnostics(fd, &diagnostics.diagnostics)));

    return diagnostics;
}

bool SM::gps_sync(const SmGPSState &target_state, int64_t timeout_s) {
    bool locked, timeout = timeout_s != INT64_C(0);
    long time_elapsed;

    if (!_configs.gps_timestamping) {
        throw py::attribute_error(
            "GPS timestamping disabled in the configurations");
    }

    if (target_state == smGPSStateNotPresent) {
        throw std::invalid_argument(
            "NOT_PRESENT is an invalid state to sync to.");
    }

    if (timeout_s < INT64_C(0)) {
        throw std::invalid_argument("Timeout must be positive or `0`");
    }

    if (!_open) {
        _open_device();
    }

    auto start_time = std::chrono::steady_clock::now();
    do {
        locked = _acquire_gps_lock(target_state);
        time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::steady_clock::now() - start_time)
                           .count();
    } while (!locked && (!timeout || time_elapsed < timeout_s));

    return locked;
}

double SM::network_speed_test(double duration) {
    double bytes_per_second;

    if (!_is_networked()) {
        throw py::attribute_error("This is not a networked device");
    }

    if (!_open) {
        _open_device();
    }

    LOG_INF("Conducting speed test for a duration of %lf seconds", duration);

    check_sm_status(_SM_API_CALL_TRACE(
        smNetworkedSpeedTest(fd, duration, &bytes_per_second)));

    LOG_INF("Speed test result: %lf bytes per second", bytes_per_second);

    return bytes_per_second;
}

SmSFPDiagnostics SM::network_diagnostic_info() const {
    SmSFPDiagnostics info{};

    if (!_open) {
        throw std::runtime_error("Device not open");
    }

    if (!_is_networked()) {
        throw std::runtime_error("Device must be a networked device");
    }

    check_sm_status(_SM_API_CALL_TRACE(smGetSFPDiagnostics(
        fd, &info.temp, &info.voltage, &info.txPower, &info.rxPower)));

    return info;
}

void SM::open() {
    if (!_open) {
        _open_device();
    }
}

void SM::close() { _close_device(); }

void SM::_log_mode() const {
    SmMode mode;
    check_sm_status(_SM_API_CALL_TRACE(smGetCurrentMode(fd, &mode)));

    switch (mode) {
    case smModeIdle:
        LOG_INF("Current Mode: Idle");
        break;
    case smModeSweeping:
        LOG_INF("Current Mode: Sweeping");
        break;
    case smModeRealTime:
        LOG_INF("Current Mode: Realtime");
        break;
    case smModeIQStreaming:
        LOG_INF("Current Mode: IQ Streaming");
        break;
    case smModeIQSegmentedCapture:
        LOG_INF("Current Mode: IQ Segment Capture");
        break;
    case smModeIQSweepList:
        LOG_INF("Current Mode: IQ sweep list");
        break;
    case smModeAudio:
        LOG_INF("Current Mode: Audio");
        break;
    default:
        LOG_ERR("Unknown mode");
        break;
    }
}

py::tuple SM::_capture_iq(double center, double bw, uint64_t capture_size,
                          bool silent) {
    if (!_open) {
        _open_device();
    }

    _configure(center, bw);

    uint64_t samples_per_capture = _configs.samples_per_capture;
    uint64_t bytes_per_capture =
        (samples_per_capture * 2 * sizeof(SH_COMPLEX_TEMPLATE_TYPE)) +
        sizeof(Capture::timestamp);
    uint64_t captures = capture_size / bytes_per_capture;

    std::vector<Capture> data(captures);

    py::array_t<complex_t> data_array({captures, samples_per_capture});
    py::buffer_info data_buf_info = data_array.request(true);

    py::array_t<int64_t> capture_times(static_cast<ssize_t>(captures));
    py::buffer_info time_buf_info = capture_times.request(true);

    py::array_t<SmGpsInfo> gps_array(static_cast<ssize_t>(captures));
    py::buffer_info gps_buf_info = gps_array.request(true);

    LOG_DBG("Collecting %lu captures", captures);
    LOG_DBG("Data size: %ld bytes", data_array.size() * data_array.itemsize());
    LOG_DBG("Timestamp data size: %ld bytes",
            capture_times.size() * capture_times.itemsize());
    LOG_DBG("Total size: %ld bytes",
            (data_array.size() * data_array.itemsize()) +
                (capture_times.size() * capture_times.itemsize()));

    for (size_t i = 0; i < captures; i++) {
        data[i].buf = static_cast<complex_t *>(data_buf_info.ptr) +
                      (i * samples_per_capture);
        data[i].timestamp = static_cast<int64_t *>(time_buf_info.ptr) + i;
        data[i].gps_info = static_cast<SmGpsInfo *>(gps_buf_info.ptr) + i;
    }

    CaptureProgress::Progress progress(captures, samples_per_capture, silent);

    LOG_INF("Starting data capture");
    progress.start();
    for (auto &capture : data) {
        smGetIQ(fd, capture.buf, static_cast<int>(samples_per_capture), nullptr,
                0, capture.timestamp, smFalse, nullptr, nullptr);
        smGetGPSInfo(fd, smFalse, nullptr, &capture.gps_info->sec_since_epoch,
                     &capture.gps_info->latitude, &capture.gps_info->longitude,
                     &capture.gps_info->altitude, nullptr, nullptr);
        progress.update();
    }
    progress.update();
    LOG_DBG("Data collection duration: %ld ms", progress.duration_ms());

    return py::make_tuple(data_array, capture_times, gps_array);
}

SmStatus SM::_open_networked_device() {
    LOG_INF("Attempting to open networked device");
    SmStatus status =
        smOpenNetworkedDevice(&fd, _configs.host.c_str(),
                              _configs.device_addr.c_str(), _configs.port);
    return status;
}

SmStatus SM::_open_serial_device() {
    SmStatus status;

    if (_configs.serial >= 0) {
        LOG_INF("Attempting to open serial device with the given serial "
                "number: 0x%X",
                _configs.serial);
        status = smOpenDeviceBySerial(&fd, _configs.serial);
    } else {
        status = smOpenDevice(&fd);
    }

    return status;
}

void SM::_open_device() {
    SmStatus status;

    LOG_DBG("Attempting to open device");

    switch (_configs.type) {
    case smDeviceTypeSM200A:
    case smDeviceTypeSM200B:
    case smDeviceTypeSM435B: {
        status = _open_serial_device();
        break;
    }
    case smDeviceTypeSM200C:
    case smDeviceTypeSM435C: {
        status = _open_networked_device();
        break;
    }
    default: {
        LOG_ERR("Invalid SM device type");
        throw std::invalid_argument("Invalid SM device");
    }
    }

    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }
    _open = true;

    _log_mode();
}

void SM::_close_device() {
    if (_open) {
        smCloseDevice(fd);
        _open = false;
    }
}

void SM::_configure(double center, double bw) {
    SmBool enable_sw_filter = (_configs.software_filter) ? smTrue : smFalse;

    LOG_INF("Configuring the SM device");

    check_sm_status(_SM_API_CALL_TRACE(smSetIQCenterFreq(fd, center)));
    check_sm_status(
        _SM_API_CALL_TRACE(smSetIQSampleRate(fd, _configs.decimation)));
    check_sm_status(
        _SM_API_CALL_TRACE(smSetIQBandwidth(fd, enable_sw_filter, bw)));
    check_sm_status(_SM_API_CALL_TRACE(smSetIQDataType(fd, smDataType32fc)));
    _configure_gps();

    check_sm_status(_SM_API_CALL_TRACE(smConfigure(fd, smModeIQStreaming)));
    _acquire_gps_lock();
}

void SM::_configure_gps() {
    if (_gps_configured) {
        return;
    }

    _acquire_gps_lock();

    if (_configs.gps_timestamping) {
        check_sm_status(_SM_API_CALL_TRACE(smSetGPSTimebaseUpdate(fd, smTrue)));
    } else {
        check_sm_status(
            _SM_API_CALL_TRACE(smSetGPSTimebaseUpdate(fd, smFalse)));
    }

    _gps_configured = true;
}

static void log_gps_state(SmGPSState state) {
    static SmGPSState prev_state = smGPSStateNotPresent;
    static int count = 0;

    if (state == prev_state && count != 0) {
        count = (count + 1) % 10;
        return;
    }

    switch (state) {
    case smGPSStateNotPresent:
        LOG_DBG("Current GPS state: Not Present");
        break;
    case smGPSStateLocked:
        LOG_DBG("Current GPS state: Locked");
        break;
    case smGPSStateDisciplined:
        LOG_DBG("Current GPS state: Disciplined");
        break;
    default:
        LOG_ERR("Invalid GPS state");
        break;
    }

    prev_state = state;
    count++;
}

bool SM::_acquire_gps_lock(SmGPSState target_state) const {
    SmGPSState state;
    bool locked;
    SmStatus status;

    status = smGetGPSState(fd, &state);
    log_gps_state(state);

    if (PyErr_CheckSignals() != 0) {
        LOG_INF("Python exception raised");
        throw py::error_already_set();
    }

    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }

    locked = state >= target_state;

    if (!locked) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return locked;
}

void SM::_acquire_gps_lock() {
    bool locked;
    long time_elapsed;
    int64_t timeout_s = _configs.gps_lock_timeout;

    if (!_configs.gps_timestamping) {
        return;
    }

    LOG_INF("Acquiring a GPS lock with a %ld second timeout", timeout_s);

    auto start_time = std::chrono::steady_clock::now();
    locked = gps_sync(smGPSStateLocked, _configs.gps_lock_timeout);
    auto end_time = std::chrono::steady_clock::now();

    time_elapsed =
        std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time)
            .count();

    if (!locked) {
        LOG_ERR("GPS lock timed out");
        throw std::runtime_error("Unable to acquire a GPS lock");
    }

    LOG_DBG("Time elapsed to acquire a lock: %ld s", time_elapsed);

    _log_mode();

    LOG_INF("GPS lock acquired. Setting platform model.");
    check_sm_status(
        _SM_API_CALL_TRACE(smSetGPSPlatformModel(fd, _configs.gps_model)));

    timeout_s = (_configs.gps_lock_timeout != 0) ? (timeout_s - time_elapsed)
                                                 : INT64_C(0);
    locked = gps_sync(smGPSStateDisciplined, timeout_s);

    LOG_DBG("Time elapsed to discipline the oscillator: %ld s",
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - start_time)
                .count());

    if (!locked) {
        LOG_ERR("GPS lock timed out");
        throw std::runtime_error("Unable to acquire a GPS lock");
    }

    end_time = std::chrono::steady_clock::now();

    LOG_INF("Successfully acquired a GPS lock! Time taken: %d seconds",
            std::chrono::duration_cast<std::chrono::seconds>(end_time -
                                                             start_time));
}

bool SM::_is_networked() const {
    bool ret;

    switch (_configs.type) {
    case smDeviceTypeSM200A:
    case smDeviceTypeSM200B:
    case smDeviceTypeSM435B:
        ret = false;
        break;
    case smDeviceTypeSM200C:
    case smDeviceTypeSM435C:
        ret = true;
        break;
    default:
        throw py::value_error("Invalid device type");
    }

    return ret;
}

py::tuple get_device_list(int max_network_devs, bool usb, bool network) {
    std::vector<int> serials(SM_MAX_DEVICES), net_serials(max_network_devs);
    int count = 0, net_count = 0;

    SmStatus status;

    if (usb) {
        status = smGetDeviceList(serials.data(), &count);
        LOG_DBG("Fetched %d serial numbers from `smGetDeviceList`", count);

        if (status != smNoError) {
            throw std::runtime_error(smGetErrorString(status));
        }
    }

    if (network) {
        net_count = max_network_devs;
        status = smNetworkConfigGetDeviceList(net_serials.data(), &net_count);
        LOG_DBG("Fetched %d serial numbers from `smNetworkConfigGetDeviceList`",
                net_count);

        if (status != smNoError) {
            throw std::runtime_error(smGetErrorString(status));
        }
    }

    serials.resize(count);
    net_serials.resize(net_count);

    serials.insert(std::end(serials), std::begin(net_serials),
                   std::end(net_serials));

    return array_to_tuple(serials.data(), serials.size());
}

static SmStatus
sm_series_internal_fetch_networked_attributes(const char *host, const char *ip,
                                              int port, int serial,
                                              SmDeviceType *type) {
    int handle = -1, serial_;
    SmDeviceType type_;
    SmStatus status = smOpenNetworkedDevice(&handle, host, ip, port);

    if (status != smNoError) {
        return status;
    }

    status = smGetDeviceInfo(handle, &type_, &serial_);

    if (status != smNoError) {
        goto close_device;
    }

    if (serial_ == serial) {
        *type = type_;
    } else {
        *type = smDeviceTypeNotSet;
    }

close_device:
    smCloseDevice(handle);

    return status;
}

static SmStatus
sm_series_internal_get_networked_device_list2(int *serials, SmDeviceType *types,
                                              int *count, const char *host) {
    assert(serials != nullptr);
    assert(types != nullptr);
    assert(count != nullptr);

    SmStatus status = smNetworkConfigGetDeviceList(serials, count);
    LOG_DBG("Fetched %d serial numbers from `smNetworkConfigGetDeviceList`",
            *count);

    if (status != smNoError) {
        return status;
    }

    for (size_t i = 0; i < *count && status == smNoError; i++) {
        int handle = -1;
        char ip[32];
        int port;

        status = smNetworkConfigOpenDevice(&handle, serials[i]);

        if (status != smNoError) {
            return status;
        }

        status = smNetworkConfigGetIP(handle, ip);
        if (status != smNoError) {
            goto close_config_device;
        }

        status = smNetworkConfigGetPort(handle, &port);

    close_config_device:
        (void)smNetworkConfigCloseDevice(handle);
        if (status != smNoError) {
            continue;
        }

        status = sm_series_internal_fetch_networked_attributes(
            host, ip, port, serials[i], &types[i]);
    }

    return status;
}

py::tuple get_device_list2(int max_network_devs, bool usb, bool network,
                           const std::string &host) {
    std::vector<int> serials(SM_MAX_DEVICES), net_serials(max_network_devs);
    std::vector<SmDeviceType> types(SM_MAX_DEVICES),
        net_types(max_network_devs);
    std::vector<SMDevice> devs;
    int count = 0, net_count = 0;
    SmStatus status;

    if (usb) {
        status = smGetDeviceList2(serials.data(), types.data(), &count);
        LOG_DBG("Fetched %d serial numbers from `smGetDeviceList2`", count);

        if (status != smNoError) {
            throw std::runtime_error(smGetErrorString(status));
        }
    }

    if (network) {
        net_count = max_network_devs;
        status = sm_series_internal_get_networked_device_list2(
            net_serials.data(), net_types.data(), &net_count, host.c_str());

        if (status != smNoError) {
            throw std::runtime_error(smGetErrorString(status));
        }
    }

    for (size_t i = 0; i < count; i++) {
        SMDevice dev;
        dev.serial = serials[i];
        dev.type = types[i];
        devs.emplace_back(dev);
    }

    for (size_t i = 0; i < net_count; i++) {
        SMDevice dev;
        dev.serial = net_serials[i];
        dev.type = net_types[i];
        devs.emplace_back(dev);
    }

    return array_to_tuple(devs.data(), devs.size());
}

static SmStatus
sm_series_internal_retrieve_network_configs(int handle,
                                            SmNetworkConfig &config) {
    char mac[32], ip[32];
    int port;

    SmStatus status = smNetworkConfigGetMAC(handle, mac);
    if (status != smNoError) {
        LOG_ERR("smNetworkConfigGetMAC() failed");
        return status;
    }

    status = smNetworkConfigGetIP(handle, ip);
    if (status != smNoError) {
        LOG_ERR("smNetworkConfigGetIP() failed");
        return status;
    }

    status = smNetworkConfigGetPort(handle, &port);
    if (status != smNoError) {
        LOG_ERR("smNetworkConfigGetPort() failed");
        return status;
    }

    config.mac = mac;
    config.ip = ip;
    config.port = port;

    return status;
}

SmNetworkConfig retrieve_networked_configurations(int serial) {
    int handle;
    SmNetworkConfig config;

    SmStatus status = smNetworkConfigOpenDevice(&handle, serial);

    if (status != smNoError) {
        LOG_ERR("Failed to open device");
        throw std::runtime_error(smGetErrorString(status));
    }

    status = sm_series_internal_retrieve_network_configs(handle, config);
    (void)smNetworkConfigCloseDevice(handle);

    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }

    return config;
}

static SmStatus sm_series_internal_network_device_config(
    int handle, const SmNetworkConfig &config, bool nvm) {
    SmBool _nvm = nvm ? smTrue : smFalse;
    SmStatus status = smNetworkConfigSetIP(handle, config.ip.c_str(), _nvm);
    if (status != smNoError) {
        LOG_ERR("smNetworkConfigSetIP() failed");
        return status;
    }

    status = smNetworkConfigSetPort(handle, config.port, _nvm);
    if (status != smNoError) {
        LOG_ERR("smNetworkConfigSetPort failed");
    }

    return status;
}

void configure_networked_device(int serial, const SmNetworkConfig &config,
                                bool non_volatile) {
    int handle;

    SmStatus status = smNetworkConfigOpenDevice(&handle, serial);
    if (status != smNoError) {
        LOG_ERR("smNetworkConfigOpenDevice");
        throw std::runtime_error(smGetErrorString(status));
    }

    status =
        sm_series_internal_network_device_config(handle, config, non_volatile);
    (void)smNetworkConfigCloseDevice(handle);

    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }
}

void broadcast_network_config(const SmNetworkConfig &config,
                              const std::string &host, bool non_volatile) {
    SmBool nvm = (non_volatile) ? smTrue : smFalse;

    LOG_DBG("Broadcasting the following settings on host address %s: <ip "
            "address: %s> <port: %d>",
            host.c_str(), config.ip.c_str(), config.port);
    SmStatus status =
        smBroadcastNetworkConfig(host.c_str(), config.ip.c_str(),
                                 static_cast<uint16_t>(config.port), nvm);
    if (status != smNoError) {
        LOG_ERR("smBroadcastNetworkConfig() failed");
        throw std::runtime_error(smGetErrorString(status));
    }
}
