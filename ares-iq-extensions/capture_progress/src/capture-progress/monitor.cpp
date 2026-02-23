/**
 * @file monitor.cpp
 *
 * @brief
 *
 * @date 2/23/26
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#include <chrono>
#include <capture-progress/monitor.hpp>

using namespace std::chrono_literals;

namespace CaptureProgress {
    MemoryMonitor::MemoryMonitor(size_t item_size, const std::function<size_t()> &size_cb, uint64_t max_mem_usage, bool hide) {
        _hide = hide;
        _element_size = item_size;
        _max_mem_usage = max_mem_usage;
        _size_cb = size_cb;
    }

    MemoryMonitor::~MemoryMonitor() {
        stop();
    }

    void MemoryMonitor::start() {
        _start = std::chrono::steady_clock::now();
        if (_hide) {
            return;
        }
        _refresh_thread = std::thread(&MemoryMonitor::_refresh_task, this);
    }

    void MemoryMonitor::stop(const void *exception) {
        if (_hide) {
            return;
        }
        (void)exception;
        _stop = std::chrono::steady_clock::now();
        _terminate.store(true);
        if (_refresh_thread.joinable()) {
            _refresh_thread.join();
        }

    }

    bool MemoryMonitor::out_of_memory() const {
        bool out = _out_of_memory;
        return out;
    }

    std::chrono::steady_clock::duration MemoryMonitor::duration() const {
        return _stop - _start;
    }

    void MemoryMonitor::_refresh_task() {
        // todo init

        while (!_terminate) {
            std::this_thread::sleep_for(1s);
            _draw();
        }

        // todo finalize
        // todo restore cursor
    }
}
