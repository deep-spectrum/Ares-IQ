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

constexpr suseconds_t usec_per_second = 1000000;

py::tuple time_now() {
    struct timeval tv;
    int err = gettimeofday(&tv, nullptr);
    if (err != 0) {
        throw std::runtime_error(strerror(errno));
    }
    return py::make_tuple(tv.tv_sec, tv.tv_usec);
}

py::tuple add_time(int64_t src_sec, int64_t src_usec, int64_t add_sec, int64_t add_usec) {
    struct timeval tv = {src_sec, src_usec}, add = {add_sec, add_usec}, result;
    add_timeval(&tv, &add, &result);
    return py::make_tuple(result.tv_sec, result.tv_usec);
}

void spin_until(int64_t tv_sec, int64_t tv_usec) {
    py::gil_scoped_release release;
    struct timeval target = {tv_sec, tv_usec}, now;
    int err = gettimeofday(&now, nullptr);
    if (err != 0) {
        throw std::runtime_error(strerror(errno));
    }
    while (cmp_timeval(&now, &target) < 0) {
        std::this_thread::sleep_for(1us);
        err = gettimeofday(&now, nullptr);
        if (err != 0) {
            throw std::runtime_error(strerror(errno));
        }
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
    m.def("add_time", &add_time, py::arg("src_sec"), py::arg("src_usec"), py::arg("add_sec"), py::arg("add_usec"));
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

void add_timeval(const struct timeval *timeval, const struct timeval *add_time, struct timeval *result) {
    result->tv_sec = timeval->tv_sec + add_time->tv_sec;
    result->tv_usec = timeval->tv_usec + add_time->tv_usec;

    if (result->tv_usec >= usec_per_second) {
        result->tv_sec++;
        result->tv_usec -= usec_per_second;
    } else if (result->tv_usec <= -usec_per_second) {
        result->tv_usec += usec_per_second;
        result->tv_sec--;
    }
}

int cmp_timeval(const struct timeval *lhs, const struct timeval *rhs) {
    if (lhs->tv_sec < rhs->tv_sec) {
        return -1;
    }

    if (lhs->tv_sec > rhs->tv_sec) {
        return 1;
    }

    if (lhs->tv_usec < rhs->tv_usec) {
        return -1;
    }

    if (lhs->tv_usec > rhs->tv_usec) {
        return 1;
    }

    return 0;
}
