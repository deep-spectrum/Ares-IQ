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
class MemoryMonitor {
  public:
    explicit MemoryMonitor(size_t item_size,
                           const std::function<size_t()> &size_cb,
                           uint64_t max_mem_usage = UINT64_C(0),
                           bool hide = false);
    ~MemoryMonitor();

    void start();
    void stop(const void *exception = nullptr);
    bool out_of_memory() const;
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

    void _refresh_task();
    void _draw();

    struct Memory {
        int64_t totalMem = 0;
        int64_t cachedMem = 0;
        int64_t sharedMem = 0;
        int64_t usedMem = 0;
        int64_t buffersMem = 0;
        int64_t avilableMem = 0;
        int64_t totalSwap = 0;
        int64_t usedSwap = 0;
        int64_t cachedSwap = 0;
        struct {
            int64_t usedZswapComp;
            int64_t usedZswapOrig;
        } zswap;
    };

    static void scan_memory_info(Memory &memory);
    static void try_read(const std::string &label, const std::string &buffer,
                         int64_t &variable);
};
} // namespace CaptureProgress

#endif // ARES_MONITOR_HPP