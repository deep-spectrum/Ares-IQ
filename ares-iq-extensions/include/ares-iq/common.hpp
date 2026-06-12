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
#include <tuple>

using namespace std::chrono_literals;
namespace py = pybind11;

constexpr int64_t ms_per_sec = 1000;
constexpr int64_t us_per_ms = 1000;

inline std::tuple<int64_t, int64_t>
chrono_to_timeval(const std::chrono::milliseconds &ms) {
    return std::make_tuple(ms.count() / ms_per_sec,
                           (ms.count() % ms_per_sec) * us_per_ms);
}

inline std::chrono::milliseconds timeval_to_chrono_ms(int64_t sec,
                                                      int64_t usec) {
    return std::chrono::milliseconds{(sec * ms_per_sec) + (usec / us_per_ms)};
}

inline std::chrono::time_point<std::chrono::system_clock,
                               std::chrono::milliseconds>
timeval_to_timepoint(int64_t sec, int64_t usec) {
    using dest_timepoint_type =
        std::chrono::time_point<std::chrono::system_clock,
                                std::chrono::milliseconds>;
    return dest_timepoint_type{timeval_to_chrono_ms(sec, usec)};
}

inline std::tuple<int64_t, int64_t> time_now() {
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch());
    return chrono_to_timeval(ms);
}

/**
 * Spin until a certain system time.
 * @param tv_sec Time value second.
 * @param tv_usec Time value microsecond.
 * @param operation An operation to perform while in the spin loop.
 */
inline void spin_until_released(int64_t tv_sec, int64_t tv_usec,
                                const std::function<void()> &operation) {
    std::function operation_ = [] { std::this_thread::sleep_for(1us); };
    auto now = std::chrono::system_clock::now;
    std::chrono::time_point<std::chrono::system_clock,
                            std::chrono::milliseconds>
        target = timeval_to_timepoint(tv_sec, tv_usec);

    if (operation != nullptr) {
        operation_ = operation;
    }

    while (now() < target) {
        operation_();
    }
}

/**
 * @struct StreamParameters
 * Struct containing parameters for streaming. This is meant strictly for
 * internal API to make things nicer.
 */
struct StreamParameters {
    /**
     * Constructor.
     * @param center Center frequency.
     * @param bw Bandwidth.
     */
    StreamParameters(double center, double bw)
        : center_frequency(center), bandwidth(bw) {}

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
     * Start second for I/Q streaming.
     */
    int64_t start_time_sec = 0;

    /**
     * Start time microsecond for I/Q streaming.
     * @note This parameter used if GPS timestamping is disabled.
     */
    int64_t start_time_usec = 0;

    /**
     * Return the struct as a Python dictionary.
     * @return The configurations as a dictionary.
     * @warning Not all parameters are in the dictionary. See the parameter
     * comments as to why.
     */
    py::dict as_dict();
};

#endif // ARES_COMMON_HPP
