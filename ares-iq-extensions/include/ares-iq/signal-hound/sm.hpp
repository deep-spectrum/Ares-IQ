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

#include <ares-iq/common.hpp>
#include <ares-iq/signal-hound/sm/sm_api.hpp>
#include <ares-iq/signal-hound/ubx_msg.hpp>
#include <ares/data-structures/queue.hpp>
#include <complex>
#include <functional>
#include <pybind11/pybind11.h>
#include <xxhash.hpp>

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
    SmDeviceType device = smDeviceTypeNotSet;
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
    uint32_t samples_per_capture = 524288;

    /**
     * Seed for xxHash64 algorithm.
     */
    uint64_t hash_seed = 0;

    /**
     * Retrieve the dictionary representation of the configurations.
     * @return The dictionary representation.
     */
    py::dict as_dict();
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

    py::dict as_dict();
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

    py::dict as_dict();
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

    /**
     * Flag indicating that the GPS data has been updated.
     */
    bool updated = false;

    /**
     * Chunk index the GPS entry is associated with.
     * @note This is for internal use with the stream API.
     */
    int64_t chunk = 0;

    /**
     * Capture index the GPS entry is associated with.
     * @note This is for internal use with the stream API.
     */
    size_t capture = 0;
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
 * @struct StartTime
 * Timeval specification.
 */
struct StartTime {
    /**
     * Seconds since epoch.
     */
    int64_t seconds = 0;

    /**
     * Microseconds.
     */
    int64_t microseconds = 0;
};

/**
 * @class SM
 * The base class for SM devices. This should be wrapped with Python.
 */
class SM {
  public:
    /**
     * Constructor.
     * @param configs The configurations for the SM device.
     */
    explicit SM(const SMConfigs &configs);

    /**
     * Destructor.
     */
    ~SM();

    /**
     * Capture IQ data.
     * @param[in] center The center frequency in Hz.
     * @param[in] bw The bandwidth in Hz.
     * @param[in] capture_size The amount of data to capture in bytes.
     * @param[in] silent Hide the progress bar.
     * @param[in] verbose Show the logging messages.
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
    std::tuple<int, int, int> firmware_version() const;

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
     * @param[in] target_state The target state for acquiring a GPS lock.
     * @param[in] timeout_s The timeout for acquiring a GPS lock in seconds.
     * @return `true` if a GPS lock was acquired. `false` otherwise.
     *
     * @note If a connection to the device is not already open, then
     * this will open the device.
     */
    bool gps_sync(const SmGPSState &target_state, int64_t timeout_s);

    /**
     * Run a network speed test.
     *
     * @param[in] duration The amount of time in seconds to run the test for.
     * @return The bytes per a second.
     *
     * @note If a connection to the device is not already open, then
     * this will open the device.
     */
    double network_speed_test(double duration) const;

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

    /**
     * Stream captured I/Q data directly to storage.
     *
     * @param[in] params The stream parameters.
     * @return Stream capture metadata.
     */
    py::dict stream_iq_data(const StreamParameters &params);

    /**
     * Retrieve the configurations for the SM device.
     * @return The SM device configurations passed in upon initialization.
     */
    SMConfigs get_configs() const;

    /**
     * Retrieve the current GPS information from the SM device.
     * @param[in] refresh Force the GPS information to refresh.
     * @return The current GPS information.
     */
    SmGpsInfo get_gps_info(bool refresh) const;

    /**
     * Enable or disable GPS timestamping.
     *
     * @param[in] enable Flag to enable or disable GPS timestamping.
     * @param[in] wait_disciplined Wait for the oscillator to be disciplined by
     * the GPS. This has no effect when the @p enable flag is set to @p false.
     * @param[in] lock_timeout The amount of seconds to wait for a lock and to
     * wait for the oscillator to get disciplined when @p wait_disciplined gets
     * set to @p true. Set to @p 0 to wait indefinitely.
     */
    void enable_gps_timestamping(bool enable, bool wait_disciplined,
                                 int64_t lock_timeout);

    /**
     * Abort the current measurement mode.
     */
    void abort_measurements() const;

    /**
     * Register logging redirects.
     *
     * @param[in] dbg Debug message callback.
     * @param[in] info Info message callback.
     * @param[in] warn Warning message callback.
     * @param[in] error Error message callback.
     * @param[in] crit Critical message callback.
     * @param[in] get_level Get level callback.
     * @param[in] set_level Set level callback.
     */
    void register_logger_callbacks(
        const std::function<void(const std::string &)> &dbg,
        const std::function<void(const std::string &)> &info,
        const std::function<void(const std::string &)> &warn,
        const std::function<void(const std::string &)> &error,
        const std::function<void(const std::string &)> &crit,
        const std::function<long()> &get_level,
        const std::function<void(long)> &set_level);

    /**
     * Set the logger level.
     * @param level New log level.
     */
    void set_logging_level(long level);

    /**
     * Retrieve the current logger level.
     * @return The current log level.
     */
    long get_log_level();

    /**
     * Retrieve the SM GPS module information.
     *
     * @param[in] timeout The maximum amount of time to wait for a response.
     *
     * @return A dictionary with the software version string, hardware version
     * string, and a list of all the version extension strings.
     */
    py::dict get_gps_module_info(const std::chrono::seconds &timeout) const;

    /**
     * Retrieve the configured reference level.
     *
     * @return The configured reference level in dBm.
     *
     * @note The reference level is set in the stream parameters.
     */
    double reference_level() const;

  private:
    typedef std::complex<SH_COMPLEX_TEMPLATE_TYPE> complex_t;

    int fd = -1;
    SMConfigs _configs;
    bool _open = false;
    bool _gps_configured = false;
    bool _gps_timestamps = false;
    std::exception_ptr py_exception = nullptr;

    std::tuple<int, int, int> firmware_version_released() const;
    SmDiagnostics diagnostic_info_released() const;
    double network_speed_test_released(double duration) const;
    SmSFPDiagnostics network_diagnostic_info_released() const;

    void log_mode() const;
    bool check_python_signals();
    bool is_networked() const;

    SmStatus open_networked_device_released();
    SmStatus open_serial_device_released();
    void open_released();

    void close_released();

    SmGpsInfo get_gps_info_released(bool refresh) const;

    static void log_gps_state(SmGPSState state);
    bool acquire_gps_lock_target_state_released(SmGPSState target_state);
    bool gps_sync_released(const SmGPSState &target_state, int64_t timeout);
    void gps_sync_released_throw_no_lock(const SmGPSState &target_state,
                                         int64_t timeout);
    void enable_gps_timestamping_released(bool enable, bool wait_disciplined,
                                          int64_t lock_timeout);

    void abort_measurements_released() const;

    struct Capture {
        complex_t *buf;
        int64_t *timestamp;
        SmGpsInfo *gps_info;
    };

    void wait_until_gps_epoch_released(SmGpsInfo &info, StartTime &start);
    void capture_iq_configure_released(const StreamParameters &params,
                                       SmGpsInfo &info, StartTime &start);
    void capture_iq_configure(const StreamParameters &params, SmGpsInfo &info,
                              StartTime &start);
    void capture_iq_internal_released(std::vector<Capture> &data,
                                      uint64_t captures,
                                      uint64_t samples_per_capture,
                                      bool silent) const;
    void capture_iq_internal(std::vector<Capture> &data, uint64_t captures,
                             uint64_t samples_per_capture, bool silent) const;
    py::tuple capture_iq_internal(double center, double bw,
                                  uint64_t capture_size, bool silent);

    struct RawCapture {
        std::vector<SH_COMPLEX_TEMPLATE_TYPE> buf;
        SmGpsInfo gps_info = {};
        int64_t timestamp;
        int32_t chunk_id;
    };

    struct RecordingMetadata {
        std::chrono::steady_clock::duration write_duration;
        uint64_t total_captures = 0;
        volatile bool save_failed = false;
        volatile bool signal_received = false;
        std::vector<SmGpsInfo> gps_updates;
        std::vector<xxh::hash64_t> iq_hash;
        xxh::hash64_t ts_hash;
    };

    struct StreamDiagnostics {
        size_t padding_written = 0;
        size_t data_bytes_written = 0;
    };
    StreamDiagnostics _stream_diagnostics;

    bool stream_iq_data_capture_released(
        uint64_t captures, ares::queue<std::unique_ptr<RawCapture>> &queue,
        int32_t chunk) const;
    void stream_iq_data_capture_released(const StreamParameters &params,
                                         uint64_t &captures_per_chunk,
                                         RecordingMetadata &metadata, bool &oom,
                                         bool &sample_loss,
                                         StartTime &start_time);
    void stream_iq_data_capture(const StreamParameters &params,
                                uint64_t &captures_per_chunk,
                                RecordingMetadata &metadata, bool &oom,
                                bool &sample_loss, StartTime &start_time);

    py::dict stream_iq_data_internal(const StreamParameters &params);
    void
    stream_iq_data_to_disk(const StreamParameters &params,
                           RecordingMetadata &metadata,
                           ares::queue<std::unique_ptr<RawCapture>> &queue);
    int stream_iq_write_iq_data(int iq_fd, std::vector<uint8_t> &data,
                                bool direct, xxh::hash_state64_t &hash_stream);
    void stream_iq_flush_chunk(int iq_fd, std::vector<uint8_t> &buffer,
                               bool direct, xxh::hash_state64_t &hash_stream);
    void save_hash_digest(int iq_fd, xxh::hash_state64_t &hash_stream,
                          std::vector<xxh::hash64_t> &save_vector) const;
    static int stream_iq_open_fd(int old_fd, const std::string &save_dir,
                                 bool iq, int32_t chunk, bool direct);

    void
    get_gps_module_info_released(UbxMsg &response,
                                 const std::chrono::seconds &timeout) const;
    bool
    wait_for_ubx_response_released(UbxMsg &response, UbxMsgType type,
                                   const std::chrono::seconds &timeout) const;
    static bool find_ubx_message_released(const std::vector<UbxMsg> &msg_list,
                                          UbxMsg &response, UbxMsgType type);

    static void warn_python(const std::stringstream &ss);
};

/**
 * @class SmException
 * Exception class for SM errors.
 */
class SmException : std::exception {
  public:
    enum SmExceptionType {
        /**
         * Device not open.
         */
        NOT_OPEN,

        /**
         * Device not idle.
         */
        NOT_IDLE,

        /**
         * No GPS lock.
         */
        NO_GPS_LOCK,

        /**
         * Unknown error/error thrown by the SM API.
         */
        UNKNOWN,
    };

    /**
     * Build an SmException from a standard exception.
     * @param type The standard exception type.
     */
    explicit SmException(SmExceptionType type);

    /**
     * Build an unknown SM exception with the given error message.
     * @param msg The error message.
     */
    explicit SmException(const char *msg);

    /**
     * What caused the exception.
     * @return The error message.
     */
    const char *what() const noexcept override;

  private:
    SmExceptionType _type;
    std::string _msg;
};

/**
 * @class TimeoutError
 * Timeout error.
 */
class TimeoutError : std::exception {
  public:
    /**
     * Constructor.
     * @param msg The timeout error message.
     */
    explicit TimeoutError(const char *msg) : _msg(msg) {}

    /**
     * What caused the timeout exception.
     * @return The error message.
     */
    const char *what() const noexcept override;

  private:
    std::string _msg;
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