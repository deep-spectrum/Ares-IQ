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
#include <complex>
#include <logging/log.hpp>
#include <pybind11/native_enum.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <stdexcept>
#include <vector>
#include <cassert>

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

    py::class_<SmGpsInfo>(m, "_SmGpsInfo", "GPS information from SM device")
        .def(py::init<>())
        .def_property_readonly(
            "sec_since_epoch", &SmGpsInfo::sec_since_epoch_,
            "Number of seconds since epoch as reported by the GPS NMEA "
            "sentences. Last reported value by the GPS. If the GPS is not "
            "locked, this value will be set to zero.")
        .def_property_readonly("latitude", &SmGpsInfo::latitude_,
                               "Latitude in decimal degrees. If the GPS is not "
                               "locked, this value will be set to zero.")
        .def_property_readonly("longitude", &SmGpsInfo::longitude_,
                               "Longitude in decimal degrees. If the GPS is "
                               "not locked, this value will be set to zero.")
        .def_property_readonly("altitude", &SmGpsInfo::altitude_,
                               "Altitude in meters. If the GPS is not locked, "
                               "this value will be set to zero.");

    py::class_<SM>(m, "_SM", "SM series device instance")
        .def(py::init<const SMConfigs &>())
        .def("capture_iq", &SM::capture_iq, "Capture IQ data")
        .def("firmware_version", &SM::firmware_version,
             "Retrieve the device firmware info")
        .def("diagnostic_info", &SM::diagnostic_info,
             "Retrieve device diagnostic information");

    m.def("sm_api_version", smGetAPIVersion, "Retrieve the SM API version");
    m.def("get_device_list", get_device_list,
          "Retrieve a list of connected SM series devices");
    m.def("get_device_list2", get_device_list2,
          "Retrieve a list of connected SM series devices with device types");
    m.def("get_networked_device_list", get_networked_device_list, "Retrieve a list of connected networked SM series devices");

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

static void check_sm_status(SmStatus status) {
    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }
}

int64_t SmGpsInfo::sec_since_epoch_() const { return sec_since_epoch; }

double SmGpsInfo::latitude_() const { return latitude; }

double SmGpsInfo::longitude_() const { return longitude; }

double SmGpsInfo::altitude_() const { return altitude; }

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

    check_sm_status(smGetFirmwareVersion(fd, &major, &minor, &revision));

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

    check_sm_status(smGetFullDeviceDiagnostics(fd, &diagnostics.diagnostics));

    return diagnostics;
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

    for (size_t i = 0; i < captures; i++) {
        data[i].buf = static_cast<complex_t *>(data_buf_info.ptr) +
                      (i * samples_per_capture);
        data[i].timestamp = static_cast<int64_t *>(time_buf_info.ptr) + i;
        data[i].gps_info = static_cast<SmGpsInfo *>(gps_buf_info.ptr) + i;
    }

    _acquire_gps_lock();

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
}

void SM::_close_device() {
    if (_open) {
        smCloseDevice(fd);
        _open = false;
    }
}

void SM::_configure(double center, double bw) const {
    SmBool enable_sw_filter = (_configs.software_filter) ? smTrue : smFalse;

    LOG_INF("Configuring the SM device");

    check_sm_status(smSetIQCenterFreq(fd, center));
    check_sm_status(smSetIQSampleRate(fd, _configs.decimation));
    check_sm_status(smSetIQBandwidth(fd, enable_sw_filter, bw));
    check_sm_status(smSetIQDataType(fd, smDataType32fc));
    _configure_gps();

    check_sm_status(smConfigure(fd, smModeIQ));
}

void SM::_configure_gps() const {
    if (_configs.gps_timestamping) {
        check_sm_status(smSetGPSTimebaseUpdate(fd, smTrue));
        check_sm_status(smSetGPSPlatformModel(fd, _configs.gps_model));
    } else {
        check_sm_status(smSetGPSTimebaseUpdate(fd, smFalse));
    }
}

bool SM::_acquire_gps_lock(SmGPSState target_state) const {
    SmGPSState state;
    bool locked;
    SmStatus status;

    status = smGetGPSState(fd, &state);

    if (PyErr_CheckSignals() != 0) {
        LOG_INF("Python exception raised");
        throw py::error_already_set();
    }

    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }

    locked = state == target_state;

    if (!locked) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return locked;
}

void SM::_acquire_gps_lock() const {
    bool locked;

    if (!_configs.gps_timestamping) {
        return;
    }

    LOG_INF("Acquiring a GPS lock");

    auto start_time = std::chrono::steady_clock::now();
    if (_configs.gps_lock_timeout == 0) {
        do {
            locked = _acquire_gps_lock(smGPSStateDisciplined);
        } while (!locked);
    } else {
        int32_t timeout = _configs.gps_lock_timeout;
        do {
            locked = _acquire_gps_lock(smGPSStateDisciplined);
            timeout--;
        } while (!locked && timeout >= 0);

        if (!locked) {
            LOG_ERR("GPS lock timed out");
            throw std::runtime_error("Unable to acquire a GPS lock");
        }
    }
    auto end_time = std::chrono::steady_clock::now();

    LOG_INF("Successfully acquired a GPS lock! Time taken: %d seconds",
            std::chrono::duration_cast<std::chrono::seconds>(end_time -
                                                             start_time));
}

py::tuple get_device_list() {
    int serial_numbers[SM_MAX_DEVICES], count;
    int net_serials[SM_MAX_DEVICES], net_count;
    int all_serials[SM_MAX_DEVICES * 2] = { 0 }, all_count = 0;

    SmStatus status = smGetDeviceList(serial_numbers, &count);
    LOG_DBG("Fetched %d serial numbers from `smGetDeviceList`", count);

    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }

    status = smNetworkConfigGetDeviceList(net_serials, &net_count);
    LOG_DBG("Fetched %d serial numbers from `smNetworkConfigGetDeviceList`", net_count);

    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }

    for (size_t i = 0; i < count; i++) {
        all_serials[all_count] = serial_numbers[i];
        all_count++;
    }

    for (size_t i = 0; i < net_count; i++) {
        all_serials[all_count] = net_serials[i];
        all_count++;
    }

    return array_to_tuple(all_serials, all_count);
}

static SmStatus get_networked_device_list2(int *serials, SmDeviceType *types, int *count) {
    assert(serials != nullptr);
    assert(types != nullptr);
    assert(count != nullptr);

    SmStatus status = smNetworkConfigGetDeviceList(serials, count);
    LOG_DBG("Fetched %d serial numbers from `smNetworkConfigGetDeviceList`", *count);

    if (status != smNoError) {
        return status;
    }

    for (size_t i = 0; i < *count; i++) {
        int handle = -1;
        status = smNetworkConfigOpenDevice(&handle, serials[i]);

        if (status != smNoError) {
            return status;
        }

        status = smGetDeviceInfo(handle, &types[i], nullptr);
        (void)smNetworkConfigCloseDevice(handle);

        if (status != smNoError) {
            return status;
        }
    }

    return smNoError;
}

py::tuple get_device_list2() {
    int serial_numbers[SM_MAX_DEVICES], count, net_serials[SM_MAX_DEVICES], net_count;
    SmDeviceType types[SM_MAX_DEVICES], net_types[SM_MAX_DEVICES];
    SMDevice devices[SM_MAX_DEVICES * 2];

    SmStatus status = smGetDeviceList2(serial_numbers, types, &count);
    LOG_DBG("Fetched %d serial numbers from `smGetDeviceList2`", count);

    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }

    status = get_networked_device_list2(net_serials, net_types, &net_count);

    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }

    size_t dev_cnt = 0;
    for (; dev_cnt < count; dev_cnt++) {
        devices[dev_cnt].serial = serial_numbers[dev_cnt];
        devices[dev_cnt].type = types[dev_cnt];
    }

    for (size_t i = 0; i < net_count; i++, dev_cnt++) {
        devices[dev_cnt].serial = net_serials[i];
        devices[dev_cnt].type = net_types[i];
    }

    return array_to_tuple(devices, dev_cnt);
}

py::tuple get_networked_device_list() {
    int serial_numbers[SM_MAX_DEVICES], count;

    SmStatus status = smNetworkConfigGetDeviceList(serial_numbers, &count);

    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }

    return array_to_tuple(serial_numbers, count);
}
