//
// Created by tschmitz on 11/5/25.
//

#include <ares-iq/signal-hound/sm.hpp>
#include <ares-iq/signal-hound/sm/sm_api.hpp>
#include <ares-iq/util.hpp>
#include <pybind11/native_enum.h>
#include <pybind11/pybind11.h>
#include <stdexcept>

namespace py = pybind11;

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

    py::class_<SMConfigs>(m, "_SmCOnfigs", "SM device configs")
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
        .def_readwrite("port", &SMConfigs::port, "Target device port");

    py::class_<SMDevice>(m, "_SmDevice",
                         "SM device metadata from device discovery")
        .def(py::init<>())
        .def_property_readonly("serial", &SMDevice::getSerial, "Serial number")
        .def_property_readonly("type", &SMDevice::getType, "Device type");

    m.def("sm_api_version", smGetAPIVersion, "Retrieve the SM API version");
    m.def("get_device_list", get_device_list,
          "Retrieve a list of connected SM series devices");
    m.def("get_device_list2", get_device_list2,
          "Retrieve a list of connected SM series devices with device types");

    m.attr("HOST_ADDR_ANY") = SM_ADDR_ANY;
    m.attr("DEFAULT_DEV_ADDR") = SM_DEFAULT_ADDR;
    m.attr("DEFAULT_PORT") = SM_DEFAULT_PORT;
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
}

int SMDevice::getSerial() const { return serial; }

SmDeviceType SMDevice::getType() const { return type; }

SM::SM(const SMConfigs &configs) { _configs = configs; }

SM::~SM() {
    if (_open) {
        smCloseDevice(fd);
    }
}

py::tuple SM::capture_iq(double center, double bw, double file_size_gb,
                         bool verbose, bool extra) {
    if (!_open) {
        _open_device();
    }

    return py::make_tuple();
}

SmStatus SM::_open_networked_device() {
    SmStatus status =
        smOpenNetworkedDevice(&fd, _configs.host.c_str(),
                              _configs.device_addr.c_str(), _configs.port);
    return status;
}

SmStatus SM::_open_serial_device() {
    SmStatus status;

    if (_configs.serial >= 0) {
        status = smOpenDeviceBySerial(&fd, _configs.serial);
    } else {
        status = smOpenDevice(&fd);
    }

    return status;
}

void SM::_open_device() {
    SmStatus status;

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
        throw std::invalid_argument("Invalid SM device");
    }
    }

    if (status != smNoError) {
        throw std::runtime_error(smGetErrorString(status));
    }
    _open = true;
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
