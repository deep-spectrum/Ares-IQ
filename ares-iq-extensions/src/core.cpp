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
#include <ares-iq/util.hpp>
#include <pybind11/chrono.h>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#define STRINGIFY(x)       #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

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
}

StreamParameters::StreamParameters(const py::kwargs &kwargs) {
    from_kwargs(kwargs, SP(center_frequency), SP(bandwidth),
                SP(file_chunk_size), SP(duration), SP(save_directory),
                SP(silent), SP(verbose), SP(stop_on_sample_loss), SP(done_cb),
                SP(max_buffer_size), SP(stream_cb));
}

py::dict StreamParameters::as_dict() {
    return to_dict(NV(center_frequency), NV(bandwidth), NV(file_chunk_size),
                   NV(duration), NV(save_directory), NV(stop_on_sample_loss));
}
