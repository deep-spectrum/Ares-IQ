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

    py::class_<SM>(m, "_SM", "SM series device instance")
        .def(py::init<const SMConfigs &>())
        .def("capture_iq", &SM::capture_iq, "Capture IQ data");

    m.def("sm_api_version", smGetAPIVersion, "Retrieve the SM API version");
    m.def("get_device_list", get_device_list,
          "Retrieve a list of connected SM series devices");
    m.def("get_device_list2", get_device_list2,
          "Retrieve a list of connected SM series devices with device types");

    m.attr("HOST_ADDR_ANY") = SM_ADDR_ANY;
    m.attr("DEFAULT_DEV_ADDR") = SM_DEFAULT_ADDR;
    m.attr("DEFAULT_PORT") = SM_DEFAULT_PORT;
    m.attr("LOGGER_NAME") = LOG_MODULE_NAME;
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

static void check_sm_status(SmStatus status) {
    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }
}

SM::SM(const SMConfigs &configs) { _configs = configs; }

SM::~SM() {
    if (_open) {
        smCloseDevice(fd);
    }
}

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

    for (size_t i = 0; i < captures; i++) {
        data[i].buf = static_cast<complex_t *>(data_buf_info.ptr) +
                      (i * samples_per_capture);
        data[i].timestamp = static_cast<int64_t *>(time_buf_info.ptr) + i;
    }

    _acquire_gps_lock();

    CaptureProgress::Progress progress(captures, samples_per_capture, silent);

    LOG_INF("Starting data capture");
    progress.start();
    for (auto &capture : data) {
        smGetIQ(fd, capture.buf, static_cast<int>(samples_per_capture), nullptr,
                0, capture.timestamp, smFalse, nullptr, nullptr);
        progress.update();
    }
    progress.update();

    return py::make_tuple(data_array, capture_times);
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

    SmStatus status = smGetDeviceList(serial_numbers, &count);

    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }

    return array_to_tuple(serial_numbers, count);
}

py::tuple get_device_list2() {
    int serial_numbers[SM_MAX_DEVICES], count;
    SmDeviceType types[SM_MAX_DEVICES];
    SMDevice devices[SM_MAX_DEVICES];

    SmStatus status = smGetDeviceList2(serial_numbers, types, &count);

    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }

    for (size_t i = 0; i < count; i++) {
        devices[i].serial = serial_numbers[i];
        devices[i].type = types[i];
    }

    return array_to_tuple(devices, count);
}
