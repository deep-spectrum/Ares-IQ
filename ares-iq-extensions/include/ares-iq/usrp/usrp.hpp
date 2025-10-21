/**
 * @file usrp.hpp
 *
 * @brief Class declaration of the USRP platform and its metadata.
 *
 * @date 9/29/25
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#ifndef ARES_IQ_USRP_HPP
#define ARES_IQ_USRP_HPP

#include <complex>
#include <pybind11/numpy.h>
#include <string>
#include <uhd/usrp/multi_usrp.hpp>

namespace py = pybind11;

/**
 * The base type for the complex data.
 */
#define COMPLEX_TEMPLATE_TYPE float

/**
 * @struct USRPconfigs
 *
 * @brief Configuration parameters for the USRP.
 * @note Each field has common default values, but specific platforms may
 * require additional or different values. This struct is intended solely for
 * passing configuration data from Python to C++. Platform-specific
 * configurations should be handled in the Python abstraction layer.
 */
struct USRPconfigs {
    USRPconfigs() = default;

    /**
     * .
     * @param spc Samples per second.
     * @throws std::range_error If samples per second is less than 1.
     */
    void set_samples_per_capture(uint64_t spc);

    /**
     * .
     * @return Samples per second.
     */
    uint64_t get_samples_per_capture() const;

    /// Device arguments. These must be defined before trying to run a capture.
    std::string device_args;

    /// Buffer size for each capture.
    uint64_t samples_per_capture = 200000;

    /// Sub device.
    std::string subdev = "A:0";

    /// CLock reference.
    std::string ref = "internal";

    /// Sample rate in MS/s
    double rate = 25e6;

    /// Receiver gain
    double gain = 0;
};

/**
 * @struct USRPStreamArgs
 *
 * @brief Stream arguments for the USRP.
 */
struct USRPStreamArgs {
    USRPStreamArgs() = default;

    /// Samples per packet.
    int spp = 200;
};

/**
 * @class USRP
 * The base class for the USRP platform. This should be wrapped with Python.
 */
class USRP {
  public:
    /**
     * .
     * @param[in] configs The configurations for the USRP device.
     * @param[in] stream_args USRP stream arguments.
     */
    explicit USRP(const USRPconfigs &configs,
                  const USRPStreamArgs &stream_args);

    /**
     * .
     */
    ~USRP() = default;

    /**
     * Capture IQ data.
     * @param[in] center The center frequency to tune to.
     * @param[in] bw The bandwidth of the capture.
     * @param[in] file_size_gb The amount of data to capture in GB.
     * @param[in] verbose Show the progress bar.
     * @param[in] extra Like verbose, but show logging messages from UHD too.
     * @return The captured complex data in a numpy array and the capture
     * timestamps.
     */
    py::tuple capture_iq(double center, double bw, double file_size_gb,
                         bool verbose, bool extra);

    /**
     * .
     * @return The device arguments.
     */
    const std::string &dev_args() const;

    /**
     * .
     * @return samples per capture.
     */
    uint64_t samples_per_capture() const;

    /**
     * .
     * @return The subdevice used.
     */
    const std::string &subdev() const;

    /**
     * .
     * @return The reference clock.
     */
    const std::string &ref() const;

    /**
     * .
     * @return The data rate.
     */
    double rate() const;

    /**
     * .
     * @return The gain.
     */
    double gain() const;

  private:
    typedef std::complex<COMPLEX_TEMPLATE_TYPE> complex_t;

    struct Capture {
        complex_t *buf;
        double *timestamp;
        // Diagnostic info
        size_t samples;
    };

    USRPconfigs _configs;
    uhd::usrp::multi_usrp::sptr usrp;
    std::shared_ptr<uhd::rx_streamer> rx_streamer;
    uhd::rx_metadata_t rx_meta;
    USRPStreamArgs _stream_args;
    bool configured = false;

    bool _extra_verbose = false;
    int _stdout = -1;
    int _stderr = -1;
    int _dev_null = -1;

    void _open_usrp();
    void _configure_usrp(double center, double bw);
    void _start_stream() const;
    void _stop_stream() const;
    void _configure(double center, double bw);

    void _disable_console_output();
    void _enable_console_output() const;
};

#endif // ARES_IQ_USRP_HPP
