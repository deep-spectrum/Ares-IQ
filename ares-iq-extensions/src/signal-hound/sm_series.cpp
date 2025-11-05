//
// Created by tschmitz on 11/5/25.
//

#include <ares-iq/signal-hound/sm/sm_api.h>
#include <ares-iq/signal-hound/sm.hpp>
#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(_sh_sm_series, m, py::mod_gil_not_used()) {
    m.def("sm_api_version", smGetAPIVersion, "Retrieve the SM API version");
}
