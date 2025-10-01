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

#include <uhd/usrp/multi_usrp.hpp>
#include <string>
#include <cstdint>
#include <complex>
#include <vector>

#define COMPLEX_TEMPLATE_TYPE float

struct USRPconfigs {
    USRPconfigs() = default;
    std::string type = "";
    uint64_t samples_per_capture = 0;
    std::string subdev = "A:0";
    std::string ref = "internal";
    double rate = 25e6;
    double gain = 0;
};

class USRP {
public:
    explicit USRP(const struct USRPconfigs &configs);
    ~USRP() = default;

    void capture_iq(double center, double bw, double file_size_gb);
    void set_stream_args(int spp);

    const std::string &dev_args() const;
    uint64_t samples_per_capture() const;
    const std::string &subdev() const;
    const std::string &ref() const;
    double rate() const;
    double gain() const;

private:
    typedef std::vector<std::complex<COMPLEX_TEMPLATE_TYPE>> complex_vec;

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

#endif //ARES_IQ_USRP_HPP
