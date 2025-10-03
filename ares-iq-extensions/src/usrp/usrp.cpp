/**
 * @file usrp.cpp
 *
 * @brief Implementation of the USRP class and USRPconfigs struct and their
 * Python bindings.
 *
 * @date 9/29/25
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#include <ares-iq/usrp.hpp>
#include <boost/format.hpp>
#include <capture-progress/progress.hpp>
#include <exception>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <uhd/usrp/multi_usrp.hpp>
#include <uhd/utils/thread.hpp>
#include <vector>

namespace py = pybind11;

constexpr int32_t timestamp_size = 8;
const std::string ant("RX");

PYBIND11_MODULE(_usrp, m, py::mod_gil_not_used()) {
    m.doc() = "USRP Platform low level interface";

    py::class_<USRPconfigs>(m, "USRPConfigs",
                            "Configuration parameters for the USRP.")
        .def(py::init<>())
        .def_readwrite("dev_args", &USRPconfigs::device_args,
                       "Device arguments")
        .def_property(
            "samples_per_capture", &USRPconfigs::get_samples_per_capture,
            &USRPconfigs::set_samples_per_capture, "Samples per capture")
        .def_readwrite("subdev", &USRPconfigs::subdev,
                       "RX frontend specification")
        .def_readwrite("ref", &USRPconfigs::ref,
                       "Clock source for the USRP device")
        .def_readwrite("rate", &USRPconfigs::rate, "RX sample rate")
        .def_readwrite("gain", &USRPconfigs::gain, "Overall RX gain");

    py::class_<USRP>(m, "USRP",
                     "The base class for the USRP platform. This should be "
                     "wrapped with Python.")
        .def(py::init<const USRPconfigs &>())
        .def("capture_iq", &USRP::capture_iq, "Capture IQ data")
        .def("_set_stream_args", &USRP::set_stream_args)
        .def_property_readonly("dev_args", &USRP::dev_args, "Device arguments")
        .def_property_readonly("samples_per_capture",
                               &USRP::samples_per_capture,
                               "Samples per capture")
        .def_property_readonly("subdev", &USRP::subdev,
                               "RX frontend specification")
        .def_property_readonly("ref", &USRP::ref,
                               "Clock source for the USRP device")
        .def_property_readonly("rate", &USRP::rate, "RX sample rate")
        .def_property_readonly("gain", &USRP::gain, "Overall RX gain");
}

USRP::USRP(const USRPconfigs &configs) { _configs = configs; }

py::tuple USRP::capture_iq(double center, double bw, double file_size_gb) {
    if (!configured) {
        uhd::set_thread_priority_safe();
        _open_usrp();
        _configure_usrp(center, bw);
        configured = true;
    } else {
        usrp->set_rx_freq(uhd::tune_request_t(center));
        usrp->set_rx_bandwidth(bw);
    }

    uint64_t samples_per_capture = _configs.samples_per_capture;
    uint64_t file_size = static_cast<uint64_t>(file_size_gb * 1e9);
    uint64_t bytes_per_capture =
        (samples_per_capture * 2 * sizeof(COMPLEX_TEMPLATE_TYPE)) +
        timestamp_size;
    uint64_t captures = file_size / bytes_per_capture;

    std::vector<Capture> data(captures);

    py::array_t<std::complex<COMPLEX_TEMPLATE_TYPE>> data_array(
        {captures, samples_per_capture});
    py::buffer_info data_buf_info = data_array.request(true);

    py::array_t<double> capture_times(static_cast<ssize_t>(captures));
    py::buffer_info time_buf_info = capture_times.request(true);

    for (size_t i = 0; i < captures; i++) {
        data[i].buf = static_cast<std::complex<COMPLEX_TEMPLATE_TYPE> *>(
                          data_buf_info.ptr) +
                      (i * samples_per_capture);
        data[i].timestamp = static_cast<double *>(time_buf_info.ptr) + i;
    }

    CaptureProgress::Progress progress(captures, samples_per_capture);

    progress.start();
    _start_stream();
    for (auto &capture : data) {
        uhd::rx_streamer::buffs_type buf = {static_cast<void *>(capture.buf)};
        capture.samples = rx_streamer->recv(buf, samples_per_capture, rx_meta);
        *capture.timestamp = rx_meta.time_spec.get_real_secs();
        progress.update();
    }
    _stop_stream();
    progress.update();

    return py::make_tuple(data_array, capture_times);
}

void USRP::_open_usrp() {
    if (_configs.device_args.empty()) {
        throw std::invalid_argument("usage error. device arguments missing.");
    }
    this->usrp = uhd::usrp::multi_usrp::make(_configs.device_args);
}

void USRP::_configure_usrp(double center, double bw) {
    usrp->set_clock_source(_configs.ref);
    usrp->set_rx_subdev_spec(_configs.subdev);
    usrp->set_rx_rate(_configs.rate);
    usrp->set_rx_freq(uhd::tune_request_t(center));
    usrp->set_rx_gain(_configs.gain);
    usrp->set_rx_bandwidth(bw);
    usrp->set_rx_antenna(ant);

    uhd::stream_args_t stream_args = uhd::stream_args_t("fc32", "sc16");
    stream_args.args = (boost::format("spp=%d") % _spp).str();
    rx_streamer = usrp->get_rx_stream(stream_args);
}

void USRP::_start_stream() const {
    uhd::stream_cmd_t cmd(
        uhd::stream_cmd_t::stream_mode_t::STREAM_MODE_START_CONTINUOUS);
    cmd.stream_now = true;
    rx_streamer->issue_stream_cmd(cmd);
}

void USRP::_stop_stream() const {
    uhd::stream_cmd_t cmd(
        uhd::stream_cmd_t::stream_mode_t::STREAM_MODE_STOP_CONTINUOUS);
    rx_streamer->issue_stream_cmd(cmd);
}

void USRP::set_stream_args(int spp) {
    if (spp < 1) {
        throw py::value_error(
            (boost::format(
                 "Cannot set samples per packet to %d. Must be > 0.") %
             spp)
                .str());
    }
    this->_spp = spp;
}

const std::string &USRP::dev_args() const { return _configs.device_args; }

uint64_t USRP::samples_per_capture() const {
    return _configs.samples_per_capture;
}

const std::string &USRP::subdev() const {
    if (configured) {
        static std::string subdev;
        subdev = usrp->get_rx_subdev_spec().to_string();
        return subdev;
    }
    return _configs.subdev;
}

const std::string &USRP::ref() const {
    if (configured) {
        static std::string configured_ref;
        configured_ref =
            usrp->get_clock_source(uhd::usrp::multi_usrp::ALL_MBOARDS);
        return configured_ref;
    }
    return _configs.ref;
}

double USRP::rate() const {
    if (configured) {
        return usrp->get_rx_rate();
    }
    return _configs.rate;
}

double USRP::gain() const {
    if (configured) {
        return usrp->get_rx_gain();
    }

    return _configs.gain;
}

void USRPconfigs::set_samples_per_capture(uint64_t spc) {
    if (spc == 0u) {
        throw std::range_error("samples_per_capture must be above 0");
    }
    samples_per_capture = spc;
}

uint64_t USRPconfigs::get_samples_per_capture() const {
    return samples_per_capture;
}
