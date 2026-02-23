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
#include <mutex>
#include <string>
#include <thread>
#include <functional>

namespace CaptureProgress {
    class MemoryMonitor {
    public:
        explicit MemoryMonitor(size_t item_size, const std::function<size_t()> &size_cb, uint64_t max_mem_usage = UINT64_C(0), bool hide = false);
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
    };
}

#endif //ARES_MONITOR_HPP