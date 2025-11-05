//
// Created by tschmitz on 11/5/25.
//

#ifndef VERSION_SM_HPP
#define VERSION_SM_HPP

#include <pybind11/pybind11.h>

namespace py = pybind11;

struct SMConfigs {

};

class SM {
public:
    explicit SM(const SMConfigs &configs);
    ~SM();

    py::tuple capture_iq(double center, double bw, double file_size_gb,
                         bool verbose, bool extra);

private:
    int fd = -1;
    SMConfigs _configs;
    bool _open = false;
};

#endif //VERSION_SM_HPP