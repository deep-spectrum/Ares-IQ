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
#include <ares/logging/log.hpp>
#include <ares/pyutil.hpp>
#include <capture-progress/monitor.hpp>
#include <capture-progress/progress.hpp>
#include <cassert>
#include <cmath>
#include <complex>
#include <fcntl.h>
#include <pybind11/chrono.h>
// ReSharper disable once CppUnusedIncludeDirective
#include <pybind11/functional.h>
#include <pybind11/native_enum.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <stdexcept>
#include <thread>
#include <vector>

namespace py = pybind11;
using namespace std::chrono_literals;

LOG_MODULE_REGISTER(sm_logger);

extern "C" {
#include <sys/stat.h>
#include <unistd.h>

static int open_fd(const char *file, bool direct) {
    if (direct) {
        return open(file, O_WRONLY | O_CREAT | O_TRUNC | O_DIRECT,
                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    }
    return open(file, O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
}

static int close_fd(int fd) { return close(fd); }
}

static const size_t PAGE_SIZE = sysconf(_SC_PAGESIZE);
constexpr double ns_per_sec = 1e9;

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
        .def_readwrite("device", &SMConfigs::device, "The device type")
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
                       "The number of samples to collect per a capture")
        .def("as_dict", &SMConfigs::as_dict, "Convert struct to dictionary");

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
                               "Power supply temperature")
        .def("as_dict", &SmDiagnostics::as_dict,
             "Retrieve diagnostics as a dictionary");

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
                               "Receive power in mW")
        .def("as_dict", &SmSFPDiagnostics::as_dict,
             "Retrieve SFP diagnostics as a dictionary");

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
        .def("close", &SM::close, "Close SM device")
        .def("stream_iq", &SM::stream_iq_data, "Stream IQ data to a file")
        .def("get_configs", &SM::get_configs, "Retrieve SM configurations");

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

    py::register_exception<SmException>(m, "_SmException");
}

SMConfigs::SMConfigs(const py::kwargs &kwargs) {
    ares::from_kwargs(kwargs, SP(device), SP(serial), SP(host), SP(device_addr),
                      SP(port), SP(gps_timestamping), SP(gps_lock_timeout),
                      SP(gps_model), SP(decimation), SP(software_filter),
                      SP(samples_per_capture));
}

py::dict SMConfigs::as_dict() {
    return ares::to_dict(NV(device), NV(serial), NV(host), NV(device_addr),
                         NV(port), NV(gps_timestamping), NV(gps_lock_timeout),
                         NV(gps_model), NV(decimation), NV(software_filter),
                         NV(samples_per_capture));
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

py::dict SmDiagnostics::as_dict() {
    return ares::to_dict(
        [](auto v) { return static_cast<int64_t>(v) == INT64_C(240); },
        py::none(), NV_NO_CHECK(voltage, diagnostics),
        NV_NO_CHECK(currentInput, diagnostics),
        NV_NO_CHECK(currentOCXO, diagnostics),
        NV_NO_CHECK(tempFPGAInternal, diagnostics),
        NV(tempFPGANear, diagnostics), NV(tempOCXO, diagnostics),
        NV(tempVCO, diagnostics), NV_NO_CHECK(tempRFBoardLO, diagnostics),
        NV(tempPowerSupply, diagnostics));
}

float SmSFPDiagnostics::get_temp() const { return temp; }

float SmSFPDiagnostics::get_voltage() const { return voltage; }

float SmSFPDiagnostics::get_tx_power() const { return txPower; }

float SmSFPDiagnostics::get_rx_power() const { return rxPower; }

py::dict SmSFPDiagnostics::as_dict() {
    return ares::to_dict([](auto v) { return static_cast<int64_t>(v) == 0; },
                         py::none(), NV(temp), NV(voltage), NV(txPower),
                         NV(rxPower));
}

#define SM_API_CALL_TRACE(api_call_) api_call_, #api_call_
#define SM_API_CALL(statement)       check_sm_status(SM_API_CALL_TRACE(statement))

static void check_sm_status(SmStatus status, const std::string &caller) {
    if (status != smNoError) {
        LOG_ERR("%s failed", caller.c_str());
        throw SmException(smGetErrorString(status));
    }
}

SmNetworkConfig::SmNetworkConfig(const py::kwargs &kwargs) {
    ares::from_kwargs(kwargs, SP(ip), SP(port));
}

SM::SM(const SMConfigs &configs) { _configs = configs; }

SM::~SM() {
    // Don't want to release the GIL in destructors
    close_released();
}

py::tuple SM::capture_iq(double center, double bw, uint64_t capture_size,
                         bool silent, bool verbose) {
    // Cannot release the lock here. The internal API needs to construct Python
    // types
    py::tuple ret;
    std::exception_ptr exception = nullptr;

    if (verbose) {
        SAVE_LOG_LEVEL_AND_OVERRIDE(LOG_LEVEL_INFO);
    }

    try {
        ret = capture_iq_internal(center, bw, capture_size, silent);
    } catch (...) {
        exception = std::current_exception();
    }

    if (verbose) {
        RESTORE_LOG_LEVEL();
    }

    if (exception) {
        std::rethrow_exception(exception);
    }

    return ret;
}

std::tuple<int, int, int> SM::firmware_version() const {
    py::gil_scoped_release release;
    return firmware_version_released();
}

SmDiagnostics SM::diagnostic_info() const {
    py::gil_scoped_release release;
    return diagnostic_info_released();
}

bool SM::gps_sync(const SmGPSState &target_state, int64_t timeout_s) const {
    py::gil_scoped_release release;
    return gps_sync_released(target_state, timeout_s);
}

double SM::network_speed_test(double duration) const {
    py::gil_scoped_release release;
    return network_speed_test_released(duration);
}

SmSFPDiagnostics SM::network_diagnostic_info() const {
    py::gil_scoped_release release;
    return network_diagnostic_info_released();
}

void SM::open() {
    py::gil_scoped_release release;
    if (!_open) {
        open_released();
    }
}

void SM::close() {
    py::gil_scoped_release release;
    close_released();
}

py::dict SM::stream_iq_data(const StreamParameters &params) {
    // Cannot release the lock here. The internal API needs to construct Python
    // types
    py::dict ret;
    std::exception_ptr eptr = nullptr;

    if (params.verbose) {
        SAVE_LOG_LEVEL_AND_OVERRIDE(LOG_LEVEL_INFO);
    }

    try {
        ret = stream_iq_data_internal(params);
    } catch (...) {
        eptr = std::current_exception();
    }

    if (params.verbose) {
        RESTORE_LOG_LEVEL();
    }

    if (eptr) {
        std::rethrow_exception(eptr);
    }

    return ret;
}

SMConfigs SM::get_configs() const { return _configs; }

std::tuple<int, int, int> SM::firmware_version_released() const {
    int major, minor, revision;

    if (!_open) {
        throw SmException(SmException::NOT_OPEN);
    }

    SM_API_CALL(smGetFirmwareVersion(fd, &major, &minor, &revision));

    return std::make_tuple(major, minor, revision);
}

SmDiagnostics SM::diagnostic_info_released() const {
    SmDiagnostics diagnostics;

    if (!_open) {
        throw SmException(SmException::NOT_OPEN);
    }

    SM_API_CALL(smGetFullDeviceDiagnostics(fd, &diagnostics.diagnostics));

    return diagnostics;
}

bool SM::gps_sync_released(const SmGPSState &target_state,
                           int64_t timeout_s) const {
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
        throw SmException(SmException::NOT_OPEN);
    }

    auto start_time = std::chrono::steady_clock::now();
    do {
        locked = acquire_gps_lock_target_state_released(target_state);
        std::this_thread::sleep_for(1s);
        time_elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::steady_clock::now() - start_time)
                           .count();
    } while (!locked && (!timeout || time_elapsed < timeout_s));

    return locked;
}

void SM::gps_configure_released() {
    if (_gps_configured) {
        return;
    }

    acquire_gps_lock_released();

    SmBool enabled = (_configs.gps_timestamping) ? smTrue : smFalse;
    LOG_DBG("GPS Timestamping: %s", (enabled == smTrue) ? "On" : "Off");
    SM_API_CALL(smSetGPSTimebaseUpdate(fd, enabled));

    _gps_configured = true;
}

void SM::log_gps_state(SmGPSState state) {
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

bool SM::acquire_gps_lock_target_state_released(SmGPSState target_state) const {
    SmGPSState state;

    SM_API_CALL(smGetGPSState(fd, &state));
    log_gps_state(state);

    if (check_python_signals()) {
        LOG_INF("Python exception raised");
        throw py::error_already_set();
    }

    return state >= target_state;
}

void SM::acquire_gps_lock_released() const {
    bool locked;
    long time_elapsed;
    int64_t timeout_s = _configs.gps_lock_timeout;

    if (!_configs.gps_timestamping) {
        return;
    }

    LOG_INF("Acquiring a GPS lock with a %ld second timeout", timeout_s);

    auto now = std::chrono::steady_clock::now;
    auto start_init = now();
    locked = gps_sync_released(smGPSStateLocked, timeout_s);
    auto stop = now();

    if (!locked) {
        LOG_ERR("GPS lock timed out");
        throw std::runtime_error("Unable to acquire a GPS lock");
    }

    time_elapsed =
        std::chrono::duration_cast<std::chrono::seconds>(stop - start_init)
            .count();
    LOG_DBG("Time elapsed to acquire a lock: %ld s", time_elapsed);

    log_mode();

    LOG_INF("GPS lock acquired. Setting platform model.");
    SM_API_CALL(smSetGPSPlatformModel(fd, _configs.gps_model));

    auto start = now();
    locked = gps_sync_released(smGPSStateLocked, timeout_s);
    stop = now();

    if (!locked) {
        LOG_ERR("GPS lock timed out");
        throw std::runtime_error("Unable to GPS discipline the oscillator");
    }

    time_elapsed =
        std::chrono::duration_cast<std::chrono::seconds>(stop - start).count();
    LOG_DBG("Time elapsed to discipline the oscillator: %ld s", time_elapsed);

    LOG_INF(
        "Successfully acquired a GPS lock! Time taken: %d seconds",
        std::chrono::duration_cast<std::chrono::seconds>(stop - start_init));
}

void SM::capture_iq_configure_released(double center, double bw) {
    if (!_open) {
        throw SmException(SmException::NOT_OPEN);
    }

    SmBool enable_sw_filter = (_configs.software_filter) ? smTrue : smFalse;
    LOG_INF("Configuring the SM device");

    SM_API_CALL(smSetIQCenterFreq(fd, center));
    SM_API_CALL(smSetIQSampleRate(fd, _configs.decimation));
    SM_API_CALL(smSetIQBandwidth(fd, enable_sw_filter, bw));
    SM_API_CALL(smSetIQDataType(fd, smDataType32fc));

    gps_configure_released();

    SM_API_CALL(smConfigure(fd, smModeIQStreaming));

    // todo: acquire lock?
}

void SM::capture_iq_configure(double center, double bw) {
    py::gil_scoped_release release;
    capture_iq_configure_released(center, bw);
}

void SM::capture_iq_internal_released(std::vector<Capture> &data,
                                      uint64_t captures,
                                      uint64_t samples_per_capture,
                                      bool silent) const {
    CaptureProgress::Progress progress(captures, samples_per_capture, silent);

    LOG_INF("Starting data capture");
    progress.start();
    for (auto &[buf, timestamp, gps_info] : data) {
        smGetIQ(fd, buf, static_cast<int>(samples_per_capture), nullptr, 0,
                timestamp, smFalse, nullptr, nullptr);
        smGetGPSInfo(fd, smFalse, nullptr, &gps_info->sec_since_epoch,
                     &gps_info->latitude, &gps_info->longitude,
                     &gps_info->altitude, nullptr, nullptr);
        progress.update();
    }
    progress.update();
    LOG_DBG("Data collection duration: %ld ms", progress.duration_ms());
}

void SM::capture_iq_internal(std::vector<Capture> &data, uint64_t captures,
                             uint64_t samples_per_capture, bool silent) const {
    py::gil_scoped_release release;
    capture_iq_internal_released(data, captures, samples_per_capture, silent);
}

py::tuple SM::capture_iq_internal(double center, double bw,
                                  uint64_t capture_size, bool silent) {
    capture_iq_configure(center, bw);

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

    capture_iq_internal(data, captures, samples_per_capture, silent);

    return py::make_tuple(data_array, capture_times, gps_array);
}

double SM::network_speed_test_released(double duration) const {
    double bytes_per_s;

    if (!is_networked()) {
        throw py::attribute_error("This is not a networked device");
    }

    if (!_open) {
        throw SmException(SmException::NOT_OPEN);
    }

    LOG_INF("Conducting speed test for a duration of %lf seconds", duration);
    SM_API_CALL(smNetworkedSpeedTest(fd, duration, &bytes_per_s));
    LOG_INF("Speed test result: %lf bytes per second", bytes_per_s);

    return bytes_per_s;
}

SmSFPDiagnostics SM::network_diagnostic_info_released() const {
    SmSFPDiagnostics info{};

    if (_open) {
        throw SmException(SmException::NOT_OPEN);
    }

    if (!is_networked()) {
        throw std::runtime_error("Device must be a networked device");
    }

    SM_API_CALL(smGetSFPDiagnostics(fd, &info.temp, &info.voltage,
                                    &info.txPower, &info.rxPower));

    return info;
}

void SM::log_mode() const {
    SmMode mode;
    check_sm_status(SM_API_CALL_TRACE(smGetCurrentMode(fd, &mode)));

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

bool SM::check_python_signals() {
    py::gil_scoped_acquire acquire;
    return PyErr_CheckSignals() != 0;
}

bool SM::is_networked() const {
    bool ret;

    switch (_configs.device) {
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

SmStatus SM::open_networked_device_released() {
    LOG_INF("Attempting to open networked device");
    SmStatus status =
        smOpenNetworkedDevice(&fd, _configs.host.c_str(),
                              _configs.device_addr.c_str(), _configs.port);
    return status;
}

SmStatus SM::open_serial_device_released() {
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

void SM::open_released() {
    SmStatus status;

    LOG_DBG("Attempting to open device");

    switch (_configs.device) {
    case smDeviceTypeSM200A:
    case smDeviceTypeSM200B:
    case smDeviceTypeSM435B: {
        status = open_serial_device_released();
        break;
    }
    case smDeviceTypeSM200C:
    case smDeviceTypeSM435C: {
        status = open_networked_device_released();
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

    log_mode();
}

void SM::close_released() {
    if (_open) {
        smCloseDevice(fd);
        _open = false;
    }
}

bool SM::stream_iq_data_capture_released(
    uint64_t captures, ares::queue<std::unique_ptr<RawCapture>> &queue,
    int32_t chunk) const {
    int sample_loss;
    bool sample_loss_ = false;
    uint32_t samples_per_capture = _configs.samples_per_capture;

    for (size_t i = 0; i < captures; i++) {
        auto capture = std::make_unique<RawCapture>();
        capture->buf.resize(samples_per_capture * 2);

        (void)smGetIQ(fd, capture->buf.data(),
                      static_cast<int>(samples_per_capture), nullptr, 0,
                      &capture->timestamp, smFalse, &sample_loss, nullptr);
        (void)smGetGPSInfo(fd, smFalse, &capture->gps_info.updated,
                           &capture->gps_info.sec_since_epoch,
                           &capture->gps_info.latitude,
                           &capture->gps_info.longitude,
                           &capture->gps_info.altitude, nullptr, nullptr);
        capture->chunk_id = chunk;
        queue.put(std::move(capture));
        if (sample_loss == SM_TRUE) {
            LOG_WRN("SM API dropping samples");
            sample_loss_ = true;
        }
    }

    return sample_loss_;
}

void SM::stream_iq_data_capture_released(const StreamParameters &params,
                                         uint64_t &captures_per_chunk,
                                         RecordingMetadata &metadata, bool &oom,
                                         bool &sample_loss) const {
    uint64_t samples_per_capture = _configs.samples_per_capture;
    uint64_t bytes_per_capture =
        (samples_per_capture * 2 * sizeof(SH_COMPLEX_TEMPLATE_TYPE)) +
        sizeof(Capture::timestamp);
    captures_per_chunk = params.file_chunk_size / bytes_per_capture;

    LOG_DBG("Page size: %u", PAGE_SIZE);
    LOG_DBG("Queue size limit: %lu bytes", params.max_buffer_size);

    ares::queue<std::unique_ptr<RawCapture>> capture_q;
    std::thread consumer([this, &params, &metadata, &capture_q]() {
        stream_iq_data_to_disk(params, metadata, capture_q);
    });

    CaptureProgress::MemoryMonitor memory_monitor(
        bytes_per_capture, [&capture_q]() { return capture_q.size(); },
        params.max_buffer_size, params.silent);

    auto now = std::chrono::steady_clock::now;
    memory_monitor.start();
    auto start = now();
    for (int32_t chunk = 0;
         (now() - start) < params.duration && !metadata.save_failed &&
         memory_monitor.out_of_memory();
         chunk++) {
        sample_loss = stream_iq_data_capture_released(captures_per_chunk,
                                                      capture_q, chunk) ||
                      sample_loss;

        if (check_python_signals()) {
            LOG_INF("CTRL+C Received");
            memory_monitor.stop();
            capture_q.clear();
            capture_q.put(static_cast<std::unique_ptr<RawCapture>>(nullptr));
            consumer.join();
            throw py::error_already_set();
        }
        if (params.stop_on_sample_loss && sample_loss) {
            LOG_ERR("Stopping prematurely due to sample loss");
            break;
        }
    }

    memory_monitor.stop(true);

    LOG_INF("Data collected");
    capture_q.put(static_cast<std::unique_ptr<RawCapture>>(nullptr));
    consumer.join();

    if (metadata.save_failed) {
        throw std::runtime_error("Operation failed");
    }

    params.done_cb();

    oom = memory_monitor.out_of_memory();
}

void SM::stream_iq_data_capture(const StreamParameters &params,
                                uint64_t &captures_per_chunk,
                                RecordingMetadata &metadata, bool &oom,
                                bool &sample_loss) const {
    py::gil_scoped_release release;
    stream_iq_data_capture_released(params, captures_per_chunk, metadata, oom,
                                    sample_loss);
}

py::dict SM::stream_iq_data_internal(const StreamParameters &params) {
    if (!_open) {
        throw SmException(SmException::NOT_OPEN);
    }

    capture_iq_configure(params.center_frequency, params.bandwidth);

    bool sample_loss, oom;
    RecordingMetadata metadata;
    uint64_t captures_per_chunk;

    stream_iq_data_capture(params, captures_per_chunk, metadata, oom,
                           sample_loss);

    py::dict ret;
    py::dict diagnostics;

    diagnostics["save_duration"] = metadata.write_duration;
    diagnostics["resource_exhaustion"] = oom;
    ret["captures"] = metadata.total_captures;
    ret["samples_per_capture"] = _configs.samples_per_capture;
    ret["captures_per_chunk"] = captures_per_chunk;
    ret["diagnostics"] = diagnostics;
    ret["sample_loss"] = sample_loss;

    return ret;
}

void SM::stream_iq_data_to_disk(
    const StreamParameters &params, RecordingMetadata &metadata,
    ares::queue<std::unique_ptr<RawCapture>> &queue) const {
    uint64_t entries_written = 0;
    int32_t current_chunk = -1;
    std::vector<uint8_t> buffer;
    int iq_fd = -1, ts_fd = -1;

    ts_fd = stream_iq_open_fd(ts_fd, params.save_directory, false, 0);
    if (ts_fd < 0) {
        metadata.save_failed = true;
        return;
    }

    buffer.reserve(_configs.samples_per_capture * 2 * 10);

    auto start = std::chrono::steady_clock::now();
    while (true) {
        auto write_data = queue.get();

        if (write_data == nullptr) {
            stream_iq_flush_chunk(iq_fd, buffer);
            close_fd(iq_fd);
            break;
        }

        if (current_chunk != write_data->chunk_id) {
            stream_iq_flush_chunk(iq_fd, buffer);
            iq_fd = stream_iq_open_fd(iq_fd, params.save_directory, true,
                                      write_data->chunk_id);
            current_chunk = write_data->chunk_id;
            if (iq_fd < 0) {
                metadata.save_failed = true;
                break;
            }
        }

        const size_t num_bytes = write_data->buf.size() * sizeof(float);
        const uint8_t *data =
            reinterpret_cast<uint8_t *>(write_data->buf.data());
        buffer.insert(buffer.end(), data, data + num_bytes);
        double timestamp =
            static_cast<double>(write_data->timestamp) / ns_per_sec;

        if (buffer.size() >= PAGE_SIZE) {
            size_t size = (buffer.size() / PAGE_SIZE) * PAGE_SIZE;
            ssize_t bytes_written = write(iq_fd, buffer.data(), size);
            if (bytes_written < 0) {
                LOG_ERR("write: %s", strerror(errno));
                continue;
            }
            buffer.erase(buffer.begin(), buffer.begin() + bytes_written);
        }

        ssize_t written = write(ts_fd, &timestamp, sizeof(double));
        if (written < 0) {
            LOG_ERR("write: %s", strerror(errno));
        }

        entries_written += 1;
    }
    auto stop = std::chrono::steady_clock::now();
    close_fd(ts_fd);

    LOG_DBG("%lu bytes dropped", buffer.size());
    LOG_DBG("Entries written: %lu", entries_written);

    metadata.total_captures = entries_written;
    metadata.write_duration = stop - start;
}

void SM::stream_iq_flush_chunk(int iq_fd, std::vector<uint8_t> &buffer) {
    if (!buffer.empty()) {
        assert(iq_fd > 0);
        size_t new_size = (((buffer.size() - 1) / PAGE_SIZE) + 1) * PAGE_SIZE;
        buffer.resize(new_size);
        ssize_t err = write(iq_fd, buffer.data(), buffer.size());
        if (err < 0) {
            LOG_ERR("write: %s", strerror(errno));
        } else {
            buffer.erase(buffer.begin(), buffer.begin() + err);
        }
    }
}

int SM::stream_iq_open_fd(int old_fd, const std::string &save_dir, bool iq,
                          int32_t chunk) {
    std::stringstream oss;
    if (old_fd > 0) {
        close_fd(old_fd);
    }

    if (iq) {
        oss << save_dir << "/"
            << "iq" << chunk << ".c8";
    } else {
        oss << save_dir << "/"
            << "ts.f8";
    }

    int new_fd = open_fd(oss.str().c_str(), iq);
    if (new_fd < 0) {
        LOG_ERR("open: %s", strerror(errno));
    }
    return new_fd;
}

SmException::SmException(SmExceptionType type) : _type(type) {
    switch (type) {
    case NOT_OPEN: {
        _msg = "Not open";
        break;
    }
    default: {
        _msg = "Unknown";
        break;
    }
    }
}

SmException::SmException(const char *msg) : _msg(msg), _type(UNKNOWN) {}

const char *SmException::what() const noexcept { return _msg.c_str(); }

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

    return ares::array_to_tuple(serials.data(), serials.size());
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

    return ares::array_to_tuple(devs.data(), devs.size());
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
