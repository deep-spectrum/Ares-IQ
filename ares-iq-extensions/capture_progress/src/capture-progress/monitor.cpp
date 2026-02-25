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
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std::chrono_literals;
using CaptureProgressInternal::FontStyle;
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
        _check_usage();
        _draw(mem);
    }

    _memory_usage_burndown();
    CaptureProgressInternal::restore_cursor(std::cout);
}

void MemoryMonitor::_draw(const Memory &mem) const {
    _draw_opening();
    _draw_memory(mem);
    _draw_time_elapsed();
    _draw_closing();
    CaptureProgressInternal::reset_cursor(std::cout);
}

void MemoryMonitor::_draw_opening() {
    std::cout << RichCyan(FontStyle::FONT_BOLD, "Mem")
              << RichWhite(FontStyle::FONT_BOLD, "[");
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
    uint32_t empty_bars = bar_length - (used_bars + buffer_bars + cache_bars);

    if (used_bars > 0) {
        std::string used_bar(used_bars, bar_char);
        std::cout << RichGreen(used_bar);
    }

    if (buffer_bars > 0) {
        std::string buffer_bar(buffer_bars, bar_char);
        std::cout << RichBlue(buffer_bar);
    }

    if (cache_bars > 0) {
        std::string cache_bar(cache_bars, bar_char);
        std::cout << RichYellow(cache_bar);
    }

    if (empty_bars > 0) {
        std::string empty(empty_bars, ' ');
        std::cout << RichDefault(empty);
    }

    double used_mem = static_cast<double>(mem.usedMem) / 1e6;
    double tot_mem = static_cast<double>(mem.totalMem) / 1e6;

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << std::setw(5) << used_mem << "G/"
       << std::fixed << std::setprecision(1) << std::setw(5) << tot_mem << "G";

    std::cout << RichRgb(FontStyle::FONT_BOLD, usage_color, ss.str())
              << RichWhite(FontStyle::FONT_BOLD, "] ");
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

void MemoryMonitor::_check_usage() {
    size_t elements = _size_cb();
    size_t mem_usage = elements * _element_size;
    if (mem_usage > _max_mem_usage) {
        _out_of_memory = true;
    }
}

void MemoryMonitor::_memory_usage_burndown() const {
    std::cout << "\n";

    if (!_out_of_memory) {
        return;
    }

    size_t usage = _size_cb();
    while (usage != 0u) {
        std::this_thread::sleep_for(100ms);
        usage = _size_cb();
        _draw_mem_burn(usage);
    }
}

void MemoryMonitor::_draw_mem_burn(size_t items) const {
    _mem_burn_open();
    _mem_burn_memory_bar(items);
    _draw_time_elapsed();
    CaptureProgressInternal::reset_cursor(std::cout);
}

void MemoryMonitor::_mem_burn_open() {
    std::cout << RichCyan(FontStyle::FONT_BOLD, "Memory Usage")
              << RichWhite(FontStyle::FONT_BOLD, "[");
}

void MemoryMonitor::_mem_burn_memory_bar(size_t items) const {
    double usage_gb = static_cast<double>(items * _element_size) / 1e9;
    double max_usage_gb = static_cast<double>(_max_mem_usage) / 1e9;
    double percentage = (usage_gb / max_usage_gb) * 100.0;

    std::string green_bar;
    std::string yellow_bar;
    std::string red_bar;

    _mem_burn_gen_green(percentage, green_bar);
    _mem_burn_gen_yellow(percentage, yellow_bar);
    _mem_burn_gen_red(percentage, red_bar);

    size_t n_empty =
        bar_length - (green_bar.size() + yellow_bar.size() + red_bar.size());
    std::string empty = std::string(n_empty, ' ');

    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << std::setw(4) << usage_gb << "G/"
       << std::fixed << std::setprecision(1) << max_usage_gb << "G";

    std::cout << RichGreen(green_bar) << RichYellow(yellow_bar)
              << RichRed(red_bar) << RichDefault(empty)
              << RichRgb(FontStyle::FONT_BOLD, usage_color, ss.str())
              << RichWhite(FontStyle::FONT_BOLD, "] ");
}

constexpr uint32_t max_green_bars = 10;
constexpr uint32_t max_yellow_bars = 20;
constexpr uint32_t max_red_bars = 10;

void MemoryMonitor::_mem_burn_gen_green(double percent, std::string &bars) {
    if (std::isgreater(percent, 25.0)) {
        bars = std::string(max_green_bars, bar_char);
        return;
    }
    double p = percent / 25.0;
    auto n = static_cast<uint32_t>(static_cast<double>(max_green_bars) * p);
    if (n > max_green_bars) {
        n = max_green_bars;
    }
    bars = std::string(n, bar_char);
}

void MemoryMonitor::_mem_burn_gen_yellow(double percent, std::string &bars) {
    if (std::isgreater(percent, 75.0)) {
        bars = std::string(max_yellow_bars, bar_char);
        return;
    }
    if (std::isless(percent, 25.0)) {
        return;
    }
    double p = (percent - 25.0) / 50.0;
    auto n = static_cast<uint32_t>(static_cast<double>(max_yellow_bars) * p);
    if (n > max_yellow_bars) {
        n = max_yellow_bars;
    }
    bars = std::string(n, bar_char);
}

void MemoryMonitor::_mem_burn_gen_red(double percent, std::string &bars) {
    if (std::isgreater(percent, 100.0)) {
        bars = std::string(max_red_bars, bar_char);
        return;
    }
    if (std::isless(percent, 75.0)) {
        return;
    }
    double p = (percent - 75.0) / 25.0;
    auto n = static_cast<uint32_t>(static_cast<double>(max_red_bars) * p);
    if (n > max_red_bars) {
        n = max_red_bars;
    }
    bars = std::string(n, bar_char);
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

constexpr const char *digits = "1234567890";

void MemoryMonitor::_try_read(const std::string &label,
                              const std::string &buffer, int64_t &variable) {
    if (buffer.compare(0, label.length(), label) != 0) {
        return;
    }
    size_t pos = buffer.find_first_of(digits);

    variable = strtoll(buffer.c_str() + pos, nullptr, 10);
}
} // namespace CaptureProgress
