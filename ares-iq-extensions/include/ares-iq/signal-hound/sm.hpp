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
#include <ares/queue.hpp>
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

/**
 * @struct SmSFPDiagnostics
 * Struct for SM200C/SM435C SFP+ port diagnostics
 */
struct SmSFPDiagnostics {
    /**
     * .
     */
    SmSFPDiagnostics() = default;

    /**
     * Reported SFP+ temperature in C.
     */
    float temp;

    /**
     * Reported SFP+ voltage in V.
     */
    float voltage;

    /**
     * Reported transmit power in mW.
     */
    float txPower;

    /**
     * Reported receive power in mW.
     */
    float rxPower;

    /**
     * .
     * @return SFP+ temperature in C.
     */
    [[nodiscard]] float get_temp() const;

    /**
     * .
     * @return SFP+ voltage in V.
     */
    [[nodiscard]] float get_voltage() const;

    /**
     * .
     * @return Transmit power in mW.
     */
    [[nodiscard]] float get_tx_power() const;

    /**
     * .
     * @return Receive power in mW.
     */
    [[nodiscard]] float get_rx_power() const;
};

/**
 * @struct SmGpsInfo
 * GPS information from the SM device.
 */
struct SmGpsInfo {
    /**
     * .
     */
    SmGpsInfo() = default;

    /**
     * Number of seconds since epoch as reported by the GPS NMEA sentences. Last
     * reported value by the GPS. If the GPS is not locked, this value will be
     * set to zero.
     */
    int64_t sec_since_epoch = 0;

    /**
     * Latitude in decimal degrees. If the GPS is not locked, this value will be
     * set to zero.
     */
    double latitude = 0.0;

    /**
     * Longitude in decimal degrees. If the GPS is not locked, this value will
     * be set to zero.
     */
    double longitude = 0.0;

    /**
     * Altitude in meters. If the GPS is not locked, this value will be set to
     * zero.
     */
    double altitude = 0.0;
};

/**
 * @struct SmNetworkConfig
 * Network configurations for SM devices.
 */
struct SmNetworkConfig {
    /**
     * .
     */
    SmNetworkConfig() = default;

    /**
     * .
     * @param[in] kwargs Key-word parameters from Python. Maps to the internal
     * attribute names.
     */
    explicit SmNetworkConfig(const py::kwargs &kwargs);

    /**
     * The MAC address of the device.
     */
    std::string mac;

    /**
     * The IP address of the networked device.
     */
    std::string ip;

    /**
     * The port of the networked device.
     */
    int port = 0;
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

    /**
     * Acquire a GPS lock before collecting any data.
     *
     * @param target_state The target state for acquiring a GPS lock.
     * @param timeout_s The timeout for acquiring a GPS lock in seconds.
     * @return `true` if a GPS lock was acquired. `false` otherwise.
     *
     * @note If a connection to the device is not already open, then
     * this will open the device.
     */
    bool gps_sync(const SmGPSState &target_state, int64_t timeout_s);

    /**
     * Run a network speed test.
     *
     * @param duration The amount of time in seconds to run the test for.
     * @return The bytes per a second.
     *
     * @note If a connection to the device is not already open, then
     * this will open the device.
     */
    double network_speed_test(double duration);

    /**
     * Retrieve the diagnostic information for the SFP+ port.
     * @return SFP+ port diagnostics.
     * @note This only works for networked devices.
     */
    SmSFPDiagnostics network_diagnostic_info() const;

    /**
     * Open a connection to an SM device.
     */
    void open();

    /**
     * Close a connection to  an SM device.
     */
    void close();

    void stream_iq_data(double center, double bw, uint64_t chunk_size,
                        const std::chrono::milliseconds &duration,
                        const std::string &filename, bool silent, bool verbose);

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
    bool _gps_configured = false;

    py::tuple _capture_iq(double center, double bw, uint64_t capture_size,
                          bool silent);

    SmStatus _open_networked_device();
    SmStatus _open_serial_device();
    void _open_device();
    void _log_mode() const;

    void _close_device();

    void _configure(double center, double bw);

    void _configure_gps();
    void _acquire_gps_lock();
    bool _acquire_gps_lock(SmGPSState target_state) const;

    bool _is_networked() const;

    struct RawCapture {
        std::vector<SH_COMPLEX_TEMPLATE_TYPE> buf;
        int64_t timestamp = 0;
        SmGpsInfo gps_info = {};
    };

    struct stream_fd {
        int iq_fd;
        int ts_fd;
        int meta_fd;
    };

    void _capture_iq_data(uint64_t captures,
                          ares::queue<RawCapture *> &queue) const;
    void _stream_iq_data(double center, double bw, uint64_t chunk_size,
                         const std::chrono::milliseconds &duration,
                         const std::string &filename, bool silent);
    void _stream_iq_data(const stream_fd &out_fd,
                         ares::queue<RawCapture *> &queue) const;
    void _write_stream_metadata(const stream_fd &out_fd, uint64_t entries,
                                double duration) const;
};

/**
 * Retrieve a list of SM device serial numbers.
 * @return SM device serial numbers.
 */
py::tuple get_device_list(int max_network_devs, bool usb, bool network);

/**
 * Retrieves a list of SM device serial numbers and types.
 * @return SM device serial numbers and device types.
 */
py::tuple get_device_list2(int max_network_devs, bool usb, bool network,
                           const std::string &host = SM_ADDR_ANY);

/**
 * Retrieve the network configurations for an sm device.
 * @param serial The serial number of the sm device.
 * @return The current network configurations of the SM device.
 */
SmNetworkConfig retrieve_networked_configurations(int serial);

/**
 * Configure the network-capable sm device.
 * @param serial The serial number of the sm device
 * @param config The network configuration of the sm device
 * @param non_volatile Make network configuration persist over power cycles
 */
void configure_networked_device(int serial, const SmNetworkConfig &config,
                                bool non_volatile = false);

/**
 * Broadcast a network configuration to each SM device on the network.
 * @param config The configuration to broadcast
 * @param host The host address to broadcast the configuration on
 * @param non_volatile Make the configuration persist over power cycles
 */
void broadcast_network_config(const SmNetworkConfig &config,
                              const std::string &host = SM_ADDR_ANY,
                              bool non_volatile = false);

#endif // VERSION_SM_HPP