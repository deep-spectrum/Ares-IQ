/**
 * @file usrp.hpp
 *
 * @brief
 *
 * @date 9/29/25
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#ifndef ARES_IQ_USRP_HPP
#define ARES_IQ_USRP_HPP

#include <complex>
#include <cstdint>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <string>
#include <uhd/usrp/multi_usrp.hpp>
#include <vector>

namespace py = pybind11;

#define COMPLEX_TEMPLATE_TYPE float

struct USRPconfigs {
    USRPconfigs() = default;

    void set_samples_per_capture(uint64_t spc);
    uint64_t get_samples_per_capture() const;

    std::string type = "";
    uint64_t samples_per_capture = 200000;
    std::string subdev = "A:0";
    std::string ref = "internal";
    double rate = 25e6;
    double gain = 0;
};

class USRP {
  public:
    explicit USRP(const struct USRPconfigs &configs);
    ~USRP() = default;

    py::tuple capture_iq(double center, double bw, double file_size_gb);
    void set_stream_args(int spp);

    const std::string &dev_args() const;
    uint64_t samples_per_capture() const;
    const std::string &subdev() const;
    const std::string &ref() const;
    double rate() const;
    double gain() const;

  private:
    typedef std::vector<std::complex<COMPLEX_TEMPLATE_TYPE>> complex_vec;

    struct Capture {
        std::complex<COMPLEX_TEMPLATE_TYPE> *buf;
        double *timestamp;
        // Diagnostic info
        size_t samples;
    };

    USRPconfigs _configs;
    uhd::usrp::multi_usrp::sptr usrp;
    std::shared_ptr<uhd::rx_streamer> rx_streamer;
    uhd::rx_metadata_t rx_meta;
    int _spp = 200;
    bool configured = false;

    void _open_usrp();
    void _configure_usrp(double center, double bw);
    void _start_stream();
    void _stop_stream();
};

#endif // ARES_IQ_USRP_HPP
