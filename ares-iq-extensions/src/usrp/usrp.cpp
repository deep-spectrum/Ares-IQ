/**
 * @file usrp.cpp
 *
 * @brief
 *
 * @date 9/29/25
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#include <uhd/utils/thread.hpp>
#include <ares-iq/usrp.hpp>
#include <iostream>
#include <pybind11/pybind11.h>
#include <uhd/usrp/multi_usrp.hpp>
#include <boost/format.hpp>
#include <vector>
#include <capture-progress/progress.hpp>

namespace py = pybind11;

constexpr int32_t timestamp_size = 8;
const std::string ant("RX");

PYBIND11_MODULE(_usrp, m, py::mod_gil_not_used()) {
    py::class_<USRPconfigs>(m, "USRPConfigs")
            .def(py::init<>())
            .def_readwrite("dev_type", &USRPconfigs::type)
            .def_readwrite("samples_per_capture", &USRPconfigs::samples_per_capture)
            .def_readwrite("subdev", &USRPconfigs::subdev)
            .def_readwrite("ref", &USRPconfigs::ref)
            .def_readwrite("rate", &USRPconfigs::rate)
            .def_readwrite("gain", &USRPconfigs::gain);

    py::class_<USRP>(m, "USRP")
            .def(py::init<const USRPconfigs &>())
            .def("capture_iq", &USRP::capture_iq)
            .def("_set_stream_args", &USRP::set_stream_args)
            .def_property_readonly("dev_args", &USRP::dev_args)
            .def_property_readonly("samples_per_capture", &USRP::samples_per_capture)
            .def_property_readonly("subdev", &USRP::subdev)
            .def_property_readonly("ref", &USRP::ref)
            .def_property_readonly("rate", &USRP::rate)
            .def_property_readonly("gain", &USRP::gain);
}

USRP::USRP(const USRPconfigs &configs) {
    setenv("UHD_LOG", "none", 1);
    _configs = configs;
}

void USRP::capture_iq(double center, double bw, double file_size_gb) {
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
    uint64_t bytes_per_capture = (samples_per_capture * 2 * sizeof(COMPLEX_TEMPLATE_TYPE)) + timestamp_size;
    uint64_t captures = file_size / bytes_per_capture;

    std::vector<complex_vec> data(captures);
    std::vector<uhd::time_spec_t> times;
    times.reserve(captures);
    std::vector<size_t> samps;
    samps.reserve(captures);
    for (auto& vec : data) {
        vec.resize(samples_per_capture);
    }

    CaptureProgress::Progress progress(captures, samples_per_capture);

    progress.start();
    _start_stream();
    for (auto& buf_ : data) {
        uhd::rx_streamer::buffs_type buf = { (void*)buf_.data() };
        size_t samples = rx_streamer->recv(buf, samples_per_capture, rx_meta);
        times.emplace_back(rx_meta.time_spec);
        samps.emplace_back(samples);
        progress.update();
    }
    _stop_stream();
    progress.update();
}

void USRP::_open_usrp() {
    this->usrp = uhd::usrp::multi_usrp::make(_configs.type);
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

void USRP::_start_stream() {
    uhd::stream_cmd_t cmd(uhd::stream_cmd_t::stream_mode_t::STREAM_MODE_START_CONTINUOUS);
    cmd.stream_now = true;
    rx_streamer->issue_stream_cmd(cmd);
}

void USRP::_stop_stream() {
    uhd::stream_cmd_t cmd(uhd::stream_cmd_t::stream_mode_t::STREAM_MODE_STOP_CONTINUOUS);
    rx_streamer->issue_stream_cmd(cmd);
}

void USRP::set_stream_args(int spp) {
    this->_spp = spp;
}

const std::string &USRP::dev_args() const {
    return _configs.type;
}

uint64_t USRP::samples_per_capture() const {
    return _configs.samples_per_capture;
}

const std::string &USRP::subdev() const {
    return _configs.subdev;
}

const std::string &USRP::ref() const {
    return _configs.ref;
}

double USRP::rate() const {
    return _configs.rate;
}

double USRP::gain() const {
    return _configs.gain;
}
