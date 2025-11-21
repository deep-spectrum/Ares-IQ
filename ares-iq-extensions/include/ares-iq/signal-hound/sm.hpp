/**
 * @file sm.hpp
 *
 * @brief Class declaration of the SM platform and its metadata.
 *
 * @date 11/5/2025
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */
#ifndef VERSION_SM_HPP
#define VERSION_SM_HPP

#include <ares-iq/signal-hound/sm/sm_api.hpp>
#include <complex>
#include <pybind11/pybind11.h>

namespace py = pybind11;

/**
 * The base type for the complex data.
 */
#define SH_COMPLEX_TEMPLATE_TYPE float

/**
 * @struct SMConfigs
 *
 * @brief Configuration parameters for SM series devices.
 */
struct SMConfigs {
    SMConfigs() = default;

    /**
     * .
     * @param[in] kwargs Key-word parameters from Python. Maps to the internal
     * attribute names.
     */
    explicit SMConfigs(const py::kwargs &kwargs);

    /**
     * The SM device type in the SM series.
     */
    SmDeviceType type = smDeviceTypeNotSet;
    /**
     * The serial number.
     */
    int serial = -1;

    /**
     * The host address for connecting to networked devices.
     */
    std::string host = SM_ADDR_ANY;
    /**
     * Device IP address for the networked device.
     */
    std::string device_addr = SM_DEFAULT_ADDR;
    /**
     * Port number for the networked device.
     */
    uint16_t port = SM_DEFAULT_PORT;

    /**
     * Use GPS timestamping.
     */
    bool gps_timestamping = false;
    /**
     * The maximum number of seconds to wait for a GPS lock in seconds. 0
     * seconds represents no timeout.
     */
    int32_t gps_lock_timeout = 0;
    /**
     * The GPS platform model to use.
     */
    SmGPSPlatformModel gps_model = SmGPSPlatformModelStationary;

    /**
     * The downsampling factor. Must be a power of 2.
     */
    uint16_t decimation = 1;
    /**
     * Enable the software filter.
     */
    bool software_filter = false;
    /**
     * The number of samples per a capture.
     */
    uint32_t samples_per_capture = 500000;
};

/**
 * @struct SMDevice
 *
 * @brief SM Device metadata.
 */
struct SMDevice {
    SMDevice() = default;

    /**
     * The serial number.
     */
    int serial = -1;

    /**
     * The device type.
     */
    SmDeviceType type = smDeviceTypeNotSet;

    /**
     * .
     * @return The serial number.
     */
    [[nodiscard]] int getSerial() const;

    /**
     * .
     * @return The device type.
     */
    [[nodiscard]] SmDeviceType getType() const;
};

/**
 * @class SmDiagnostics
 * Wrapper struct for SmDeviceDiagnostics struct.
 *
 * @note The reason why this exists is because pybind11 `def_property_readonly`
 * requires getters. If using C++ API, the values can be fetched directly from
 * the diagnostics member, thus not needing the getters.
 */
struct SmDiagnostics {
    /**
     * .
     */
    SmDiagnostics() = default;

    /**
     * Sm device diagnostic information.
     */
    SmDeviceDiagnostics diagnostics = {};

    /**
     * .
     * @return Device voltage
     */
    [[nodiscard]] float voltage() const;

    /**
     * .
     * @return Input current
     */
    [[nodiscard]] float current_input() const;

    /**
     * .
     * @return OCXO current
     */
    [[nodiscard]] float current_ocxo() const;

    /**
     * .
     * @return FPGA core/internal temp
     */
    [[nodiscard]] float temp_fpga_internal() const;

    /**
     * .
     * @return Temp near FPGA
     */
    [[nodiscard]] float temp_fpga_near() const;

    /**
     * .
     * @return OCXO temperature
     */
    [[nodiscard]] float temp_ocxo() const;

    /**
     * .
     * @return VCO temperature
     */
    [[nodiscard]] float temp_vco() const;

    /**
     * .
     * @return Temperature on RF board LO
     */
    [[nodiscard]] float temp_rf_board_lo() const;

    /**
     * .
     * @return Power supply temperature
     */
    [[nodiscard]] float temp_power_supply() const;
};

struct SmGpsInfo {
    SmGpsInfo() = default;

    int64_t sec_since_epoch = 0;
    double latitude = 0.0;
    double longitude = 0.0;
    double altitude = 0.0;

    [[nodiscard]] int64_t sec_since_epoch_() const;
    [[nodiscard]] double latitude_() const;
    [[nodiscard]] double longitude_() const;
    [[nodiscard]] double altitude_() const;
};

/**
 * @class SM
 * The base class for SM devices. This should be wrapped with Python.
 */
class SM {
  public:
    /**
     * .
     * @param configs The configurations for the SM device.
     */
    explicit SM(const SMConfigs &configs);

    /**
     * .
     */
    ~SM();

    /**
     * Capture IQ data.
     * @param center The center frequency in Hz.
     * @param bw The bandwidth in Hz.
     * @param capture_size The amount of data to capture in bytes.
     * @param silent Hide the progress bar.
     * @param verbose Show the logging messages.
     * @return The captured complex data in a numpy array and the capture
     * timestamps.
     */
    py::tuple capture_iq(double center, double bw, uint64_t capture_size,
                         bool silent, bool verbose);

    /**
     * Retrieve the firmware version.
     * @return A tuple representing the major, minor, and revision number of the
     * firmware version.
     */
    py::tuple firmware_version();

    /**
     * Retrieve diagnostic information from the Sm device. This requires the
     * device to be open first (@ref capture_iq() opens a device and leaves it
     * open).
     * @return Device diagnostic information.
     */
    SmDiagnostics diagnostic_info() const;

  private:
    typedef std::complex<SH_COMPLEX_TEMPLATE_TYPE> complex_t;

    struct Capture {
        complex_t *buf;
        int64_t *timestamp;
        SmGpsInfo *gps_info;
    };

    int fd = -1;
    SMConfigs _configs;
    bool _open = false;

    py::tuple _capture_iq(double center, double bw, uint64_t capture_size,
                          bool silent);

    SmStatus _open_networked_device();
    SmStatus _open_serial_device();
    void _open_device();

    void _close_device();

    void _configure(double center, double bw) const;

    void _configure_gps() const;
    void _acquire_gps_lock() const;
    bool _acquire_gps_lock(SmGPSState target_state) const;
};

/**
 * Retrieve a list of SM device serial numbers.
 * @return SM device serial numbers.
 */
py::tuple get_device_list();

/**
 * Retrieves a list of SM device serial numbers and types.
 * @return SM device serial numbers and device types.
 */
py::tuple get_device_list2();

#endif // VERSION_SM_HPP