/**
 * @file monitor.hpp
 *
 * @brief
 *
 * @date 2/23/26
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#ifndef ARES_MONITOR_HPP
#define ARES_MONITOR_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

namespace CaptureProgress {
/**
 * @class MemoryMonitor
 * Memory monitor view for progress capture.
 */
class MemoryMonitor {
  public:
    /**
     * .
     * @param item_size The size of each element in bytes.
     * @param size_cb The function to call to get the number of elements in the
     * container.
     * @param max_mem_usage The maximum number of bytes the container is allowed
     * to use. Default is `0`.
     * @param hide Flag to hide this view. Default is `false`.
     *
     * @note If `max_mem_usage` is set to `0`, then the memory monitor will not
     * raise its `out_of_memory` flag.
     */
    explicit MemoryMonitor(size_t item_size,
                           const std::function<size_t()> &size_cb,
                           uint64_t max_mem_usage = UINT64_C(0),
                           bool hide = false);

    /**
     * .
     */
    ~MemoryMonitor();

    /**
     * Start the memory monitor view.
     */
    void start();

    /**
     * Stop the memory monitor view.
     * @param exception The exception thrown during execution.
     */
    void stop(const void *exception = nullptr);

    /**
     * Check for resource exhaustion.
     * @return Flag indicating if the allowable memory resources got exhausted.
     */
    bool out_of_memory() const;

    /**
     * Retrieve the duration between the start and stop calls of the memory
     * monitor.
     * @return The duration between the start and stop calls.
     */
    std::chrono::steady_clock::duration duration() const;

  private:
    bool _hide;
    std::thread _refresh_thread;
    std::mutex _memory_mutex;
    std::atomic_bool _terminate{false};
    std::atomic_bool _out_of_memory{false};

    std::function<size_t()> _size_cb;
    size_t _element_size;
    size_t _max_mem_usage;

    std::chrono::steady_clock::time_point _start;
    std::chrono::steady_clock::time_point _stop;

    struct Memory {
        int64_t totalMem = 0;
        int64_t usedMem = 0;
        int64_t buffersMem = 0;
        int64_t cachedMem = 0;
    };

    void _refresh_task();
    void _draw(const Memory &mem) const;
    static void _draw_opening();
    static void _draw_memory(const Memory &mem);
    void _draw_time_elapsed() const;
    void _draw_closing() const;

    void _check_usage();

    void _memory_usage_burndown() const;
    void _draw_mem_burn(size_t items) const;
    static void _mem_burn_open();
    void _mem_burn_memory_bar(size_t items) const;
    static void _mem_burn_gen_green(double percent, std::string &bars);
    static void _mem_burn_gen_yellow(double percent, std::string &bars);
    static void _mem_burn_gen_red(double percent, std::string &bars);

    static void _scan_memory_info(Memory &memory);
    static void _try_read(const std::string &label, const std::string &buffer,
                          int64_t &variable);
};
} // namespace CaptureProgress

#endif // ARES_MONITOR_HPP