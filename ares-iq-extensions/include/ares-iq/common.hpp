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

/**
 * @struct StreamParameters
 * Struct containing parameters for streaming. This is meant strictly for
 * internal API to make things nicer.
 */
struct StreamParameters {
    /**
     * .
     * @param kwargs Python keyword arguments.
     */
    explicit StreamParameters(const py::kwargs &kwargs);

    /**
     * Center frequency in Hz.
     */
    double center_frequency = -1.0;

    /**
     * The bandwidth in Hz.
     */
    double bandwidth = -1.0;

    /**
     * The file chunk size in bytes.
     */
    uint64_t file_chunk_size = 4000000000;

    /**
     * The duration of the I/Q stream.
     */
    std::chrono::milliseconds duration = 0s;

    /**
     * The save directory for the captured I/Q data. Must already exist.
     */
    std::string save_directory;

    /**
     * Run the stream in silent mode (No status bars).
     * @note Not in dictionary representation. Not important for metadata.
     */
    bool silent = true;

    /**
     * Run the stream with logging messages.
     * @note Not in dictionary representation. Not important for metadata.
     */
    bool verbose = false;

    /**
     * Stop streaming if sample loss starts occurring.
     */
    bool stop_on_sample_loss = false;

    /**
     * The callback to call when streaming is aborted or is done.
     * @note Not in dictionary representation. Not important for metadata.
     */
    std::function<void()> done_cb = nullptr;

    /**
     * The maximum allowable queue size in bytes. This should be a few GB. If 0,
     * there is no maximum queue size.
     * @note Not in dictionary representation. Python API and C++ API are
     * different. It is better to use the Python input parameter in the metadata
     * since that is public facing.
     */
    uint64_t max_buffer_size = 0;

    /**
     * Custom hook for streamed data.
     * @note Not in dictionary representation. Not important for metadata.
     * @todo: Figure out what to pass in
     */
    std::function<void()> stream_cb = nullptr;

    /**
     * Start time for I/Q streaming.
     */
    int64_t start_time_gps_epoch = 0;

    /**
     * Return the struct as a Python dictionary.
     * @return The configurations as a dictionary.
     * @warning Not all parameters are in the dictionary. See the parameter
     * comments as to why.
     */
    py::dict as_dict();
};

#endif // ARES_COMMON_HPP
