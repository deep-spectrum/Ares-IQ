//
// Created by tschmitz on 11/5/25.
//

#ifndef VERSION_SM_HPP
#define VERSION_SM_HPP

#include <ares-iq/signal-hound/sm/sm_api.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

struct SMConfigs {
    SMConfigs() = default;
    explicit SMConfigs(const py::kwargs &kwargs);

    SmDeviceType type;
    int serial = -1;
};

struct SMDevice {
    SMDevice() = default;

    int serial = -1;
    SmDeviceType type = smDeviceTypeSM200A;

    [[nodiscard]] int getSerial() const;
    [[nodiscard]] SmDeviceType getType() const;
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

py::tuple get_device_list();
py::tuple get_device_list2();

#endif // VERSION_SM_HPP