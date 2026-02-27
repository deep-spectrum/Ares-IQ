/**
 * @file common.hpp
 *
 * @brief
 *
 * @date 2/27/26
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#ifndef ARES_COMMON_HPP
#define ARES_COMMON_HPP

#include <chrono>
#include <cstdint>
#include <functional>
#include <pybind11/pybind11.h>
#include <string>

using namespace std::chrono_literals;
namespace py = pybind11;

struct StreamParameters {
    explicit StreamParameters(const py::kwargs &kwargs);

    double center_frequency = -1.0;
    double bandwidth = -1.0;
    uint64_t file_chunk_size = 4000000000;
    std::chrono::milliseconds duration = 0s;
    std::string save_directory;
    bool silent = true;
    bool verbose = false;
    bool stop_on_sample_loss = false;
    std::function<void()> done_cb = nullptr;
    uint64_t max_buffer_size = 0;

    std::function<void()> stream_cb = nullptr;

    py::dict as_dict();
};

#endif // ARES_COMMON_HPP
