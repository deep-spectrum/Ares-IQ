/**
 * @file monitor.cpp
 *
 * @brief
 *
 * @date 2/23/26
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#include <capture-progress/display_rich.hpp>
#include <capture-progress/monitor.hpp>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <string>
#include <cstdlib>

using namespace std::chrono_literals;
using CaptureProgressInternal::RichBlue;
using CaptureProgressInternal::RichCyan;
using CaptureProgressInternal::RichDefault;
using CaptureProgressInternal::RichGreen;
using CaptureProgressInternal::RichRed;
using CaptureProgressInternal::RichRgb;
using CaptureProgressInternal::RichWhite;
using CaptureProgressInternal::RichYellow;

namespace CaptureProgress {

static const RichRgb::ForegroundRgb usage_color(81, 78, 94);
constexpr long seconds_per_hour = 3600;
constexpr long seconds_per_minute = 60;
constexpr long minutes_per_hour = 60;
constexpr uint32_t bar_length = 40;
constexpr uint32_t stats_length = 11;
constexpr char bar_char = '|';

MemoryMonitor::MemoryMonitor(size_t item_size,
                             const std::function<size_t()> &size_cb,
                             uint64_t max_mem_usage, bool hide) {
    _hide = hide;
    _element_size = item_size;
    _max_mem_usage = max_mem_usage;
    _size_cb = size_cb;
}

MemoryMonitor::~MemoryMonitor() { stop(); }

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
    CaptureProgressInternal::hide_cursor(std::cout);

    Memory mem;
    while (!_terminate) {
        std::this_thread::sleep_for(100ms);
        _scan_memory_info(mem);
        // todo: run memory check
        _draw(mem);
    }

    // todo finalize
    CaptureProgressInternal::restore_cursor(std::cout);
}

void MemoryMonitor::_draw(const Memory &mem) {
    _draw_opening();
    _draw_memory(mem);
    _draw_time_elapsed();
    _draw_closing();
    CaptureProgressInternal::reset_cursor(std::cout);
}

void MemoryMonitor::_draw_opening() {
    std::cout << RichCyan("Mem") << RichWhite("[");
}

void MemoryMonitor::_draw_memory(const Memory &mem) {
    double used_percent =
        static_cast<double>(mem.usedMem) / static_cast<double>(mem.totalMem);
    double buffer_percent =
        static_cast<double>(mem.buffersMem) / static_cast<double>(mem.totalMem);
    double cache_percent =
        static_cast<double>(mem.cachedMem) / static_cast<double>(mem.totalMem);

    int used_bars =
        static_cast<int>(static_cast<double>(bar_length) * used_percent);
    int buffer_bars =
        static_cast<int>(static_cast<double>(bar_length) * buffer_percent);
    int cache_bars =
        static_cast<int>(static_cast<double>(bar_length) * cache_percent);
    int empty_bars = bar_length - (used_bars + buffer_bars + cache_bars);

    double used_mem = static_cast<double>(mem.usedMem) / 1e6;
    double tot_mem = static_cast<double>(mem.totalMem) / 1e6;

    std::string used_bar(used_bars, bar_char);
    std::string buffer_bar(buffer_bars, bar_char);
    std::string cache_bar(cache_bars, bar_char);
    std::string empty(empty_bars, ' ');

    std::cout << RichGreen(used_bars) << RichBlue(buffer_bar)
              << RichYellow(cache_bar) << RichDefault(empty) << std::fixed
              << std::setprecision(1)
              << RichRgb(usage_color, used_mem, "G/", tot_mem, "G")
              << RichWhite("] ");
}

void MemoryMonitor::_draw_time_elapsed() const {
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::steady_clock::now() - _start)
                       .count();
    long hrs = elapsed / seconds_per_hour;
    long min = (elapsed / seconds_per_minute) % minutes_per_hour;
    long sec = elapsed % seconds_per_minute;

    std::stringstream oss;
    oss << std::setw(1) << std::setfill('0') << hrs << ":" << std::setw(2)
        << std::setfill('0') << min << ":" << std::setw(2) << std::setfill('0')
        << sec;
    std::cout << RichYellow(oss.str());
}

void MemoryMonitor::_draw_closing() const {
    if (_out_of_memory) {
        std::cout << RichRed(" Terminated: Resources Exhausted");
    }
}

constexpr const char *procmem = "/proc/meminfo";

void MemoryMonitor::_scan_memory_info(Memory &memory) {
    /*
     * This was taken from htop:
     * https://github.com/htop-dev/htop/blob/2b95568f443af8730d6b27552334f095f4382120/linux/LinuxMachine.c#L130-L219
     */
    int64_t availableMem = 0;
    int64_t freeMem = 0;
    int64_t totalMem = 0;
    int64_t buffersMem = 0;
    int64_t cachedMem = 0;
    int64_t sharedMem = 0;
    int64_t swapTotalMem = 0;
    int64_t swapCacheMem = 0;
    int64_t swapFreeMem = 0;
    int64_t swapReclaimableMem = 0;
    int64_t zSwapCompMem = 0;
    int64_t zSwapOrigMem = 0;

    std::ifstream file(procmem);
    if (!file.is_open()) {
        // error
        return;
    }

    std::string line;

    while (std::getline(file, line)) {
        switch (line[0]) {
        case 'M': {
            _try_read("MemAvailable:", line, availableMem);
            _try_read("MemFree:", line, freeMem);
            _try_read("MemTotal:", line, totalMem);
            break;
        }
        case 'B': {
            _try_read("Buffers:", line, buffersMem);
            break;
        }
        case 'C': {
            _try_read("Cached:", line, cachedMem);
            break;
        }
        case 'S': {
            switch (line[1]) {
            case 'h': {
                _try_read("Shmem:", line, sharedMem);
                break;
            }
            case 'w': {
                _try_read("SwapTotal:", line, swapTotalMem);
                _try_read("SwapCached:", line, swapCacheMem);
                _try_read("SwapFree:", line, swapFreeMem);
                break;
            }
            case 'R': {
                _try_read("SReclaimable:", line, swapReclaimableMem);
                break;
            }
            default:
                break;
            }
            break;
        }
        case 'Z': {
            _try_read("Zswap:", line, zSwapCompMem);
            _try_read("Zswapped", line, zSwapOrigMem);
            break;
        }
        default:
            break;
        }
    }
    file.close();

    memory.totalMem = totalMem;
    memory.cachedMem = cachedMem + swapReclaimableMem - sharedMem;
    const int64_t usedDiff =
        freeMem + cachedMem + swapReclaimableMem + buffersMem;
    memory.usedMem =
        (totalMem >= usedDiff) ? totalMem - usedDiff : totalMem - freeMem;
    memory.buffersMem = buffersMem;
}

void MemoryMonitor::_try_read(const std::string &label,
                              const std::string &buffer, int64_t &variable) {
    if (buffer.compare(0, label.length(), label) != 0) {
        return;
    }

    variable = strtoll(buffer.c_str() + label.length(), nullptr, 10);
}
} // namespace CaptureProgress
