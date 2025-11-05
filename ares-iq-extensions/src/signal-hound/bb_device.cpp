//
// Created by tschmitz on 11/4/25.
//

#include <ares-iq/signal-hound/bb_api.h>
#include <pybind11/pybind11.h>
#include <cstdio>

namespace py = pybind11;

int dev_count() {
    int device_count = 0;
    bbStatus status = bbGetSerialNumberList(nullptr, &device_count);
    (void)printf("Device count: %d\n", device_count);
    (void)printf("Status: %d\n", static_cast<int>(status));
    return device_count;
}

PYBIND11_MODULE(_signal_hound, m, py::mod_gil_not_used()) {
    m.def("dev_cnt", &dev_count);
}
