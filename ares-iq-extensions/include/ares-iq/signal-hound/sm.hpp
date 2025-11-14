//
// Created by tschmitz on 11/5/25.
//

#ifndef VERSION_SM_HPP
#define VERSION_SM_HPP

#include <ares-iq/signal-hound/sm/sm_api.hpp>
#include <pybind11/pybind11.h>

namespace py = pybind11;

struct SMConfigs {
    SMConfigs() = default;
    explicit SMConfigs(const py::kwargs &kwargs);

    SmDeviceType type = smDeviceTypeNotSet;
    int serial = -1;

    std::string host = SM_ADDR_ANY;
    std::string device_addr = SM_DEFAULT_ADDR;
    uint16_t port = SM_DEFAULT_PORT;

    bool gps_timestamping = false;
    int32_t gps_lock_timeout = 0u;
    SmGPSPlatformModel gps_model = SmGPSPlatformModelStationary;

    uint16_t decimation = 1;
    bool software_filter = false;
    uint32_t samples_per_capture = 500000;
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

    py::tuple capture_iq(double center, double bw, uint64_t capture_size,
                         bool silent, bool verbose);

  private:
    int fd = -1;
    SMConfigs _configs;
    bool _open = false;

    SmStatus _open_networked_device();
    SmStatus _open_serial_device();
    void _open_device();

    void _configure(double center, double bw) const;

    void _configure_gps() const;
    void _acquire_gps_lock() const;
    bool _acquire_gps_lock(SmGPSState target_state) const;
};

py::tuple get_device_list();
py::tuple get_device_list2();

#endif // VERSION_SM_HPP