/**
 * @file progress.hpp
 *
 * @brief Class declaration for a progress bar.
 *
 * @date 10/1/25
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#ifndef CAPTURE_PROGRESS_PROGRESS_HPP
#define CAPTURE_PROGRESS_PROGRESS_HPP

#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>

namespace CaptureProgress {
/**
 * @class Progress
 * Implementation of a progress bar in C++.
 */
class Progress {
  public:
    /**
     * .
     * @param captures The number of captures needed for completion
     * @param samples_per_capture The number of samples per a capture
     */
    explicit Progress(uint64_t captures, uint64_t samples_per_capture);

    /**
     * .
     */
    ~Progress();

    /**
     * Start the progress bar.
     */
    void start();

    /**
     * Take 1 step in the progress bar.
     */
    void update();

    /**
     * Stop the progress bar.
     */
    void stop();

  private:
    std::thread _refresh_thread;
    std::mutex _samples_mtx;
    uint64_t _spc;
    uint64_t _total_samples;
    uint64_t _samples_captured = 0;
    double _percent_complete = 0;
    std::atomic_bool _terminate{false};
    std::chrono::system_clock::time_point _start;
    std::string _rate = "0.0 MS/sec ";

    void _refresh_task();
    void _init_bar();
    void _draw();
    void _finalize();

    bool _completed();

    void _update_rate();
    static void _draw_opening();
    void _draw_bar() const;
    void _draw_rate();
    void _draw_time_elapsed();

    static void _reset_cursor();
    static void _hide_cursor();
    static void _restore_cursor();
};
} // namespace CaptureProgress

#endif // CAPTURE_PROGRESS_PROGRESS_HPP
