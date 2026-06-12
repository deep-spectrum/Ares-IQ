/**
 * @file core.cpp
 *
 * @brief Core utilities of the ares-iq extension package.
 *
 * @date 9/29/25
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#include <ares-iq/common.hpp>
#include <ares/pyutil.hpp>
#include <ares/util.h>
#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

constexpr int64_t ms_per_sec = 1000;
constexpr int64_t us_per_ms = 1000;

static py::tuple chrono_to_timeval(const std::chrono::milliseconds &ms) {
    return py::make_tuple(ms.count() / ms_per_sec, (ms.count() % ms_per_sec) * us_per_ms);
}

static std::chrono::milliseconds timeval_to_chrono_ms(int64_t sec, int64_t usec) {
    return std::chrono::milliseconds{(sec * ms_per_sec) + (usec / us_per_ms)};
}

static std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> timeval_to_timepoint(int64_t sec, int64_t usec) {
    using dest_timepoint_type=std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>;
    return dest_timepoint_type{
        timeval_to_chrono_ms(sec, usec)
    };
}

py::tuple time_now() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return chrono_to_timeval(ms);
}

py::tuple add_time(int64_t src_sec, int64_t src_usec, int64_t add_sec,
                   int64_t add_usec) {
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> src = timeval_to_timepoint(src_sec, src_usec);
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> result = src + timeval_to_chrono_ms(add_sec, add_usec);
    return chrono_to_timeval(std::chrono::duration_cast<std::chrono::milliseconds>(result.time_since_epoch()));
}

void spin_until(int64_t tv_sec, int64_t tv_usec) {
    py::gil_scoped_release release;
    auto now = std::chrono::system_clock::now;
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> target = timeval_to_timepoint(tv_sec, tv_usec);
    while (now() < target) {
        std::this_thread::sleep_for(1us);
    }
}

PYBIND11_MODULE(_core, m, py::mod_gil_not_used()) {
#if defined(VERSION_INFO)
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif // defined(VERSION_INFO)

    py::class_<StreamParameters>(m, "_StreamParameters",
                                 "I/Q data streaming parameters")
        .def(py::init<const py::kwargs &>())
        .def("as_dict", &StreamParameters::as_dict,
             "Convert streaming parameters to a dictionary");

    m.def("time_now", &time_now);
    m.def("add_time", &add_time, py::arg("src_sec"), py::arg("src_usec"),
          py::arg("add_sec"), py::arg("add_usec"));
    m.def("spin_until", &spin_until, py::arg("sec"), py::arg("usec"));
}

StreamParameters::StreamParameters(const py::kwargs &kwargs) {
    ares::from_kwargs(kwargs, SP(center_frequency), SP(bandwidth),
                      SP(file_chunk_size), SP(duration), SP(save_directory),
                      SP(silent), SP(verbose), SP(stop_on_sample_loss),
                      SP(done_cb), SP(max_buffer_size), SP(stream_cb),
                      SP(start_time_gps_epoch));
}

py::dict StreamParameters::as_dict() {
    return ares::to_dict(NV(center_frequency), NV(bandwidth),
                         NV(file_chunk_size), NV(duration), NV(save_directory),
                         NV(stop_on_sample_loss));
}
