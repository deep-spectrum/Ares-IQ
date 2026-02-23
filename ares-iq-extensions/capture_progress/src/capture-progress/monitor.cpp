/**
 * @file monitor.cpp
 *
 * @brief
 *
 * @date 2/23/26
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#include <capture-progress/monitor.hpp>
#include <chrono>
#include <fstream>
#include <string>

using namespace std::chrono_literals;

namespace CaptureProgress {
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
    // todo init

    while (!_terminate) {
        std::this_thread::sleep_for(1s);
        _draw();
    }

    // todo finalize
    // todo restore cursor
}

constexpr const char *procmem = "/proc/meminfo";

void MemoryMonitor::scan_memory_info(Memory &memory) {
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
            try_read("MemAvailable:", line, availableMem);
            try_read("MemFree:", line, freeMem);
            try_read("MemTotal:", line, totalMem);
            break;
        }
        case 'B': {
            try_read("Buffers:", line, buffersMem);
            break;
        }
        case 'C': {
            try_read("Cached:", line, cachedMem);
            break;
        }
        case 'S': {
            switch (line[1]) {
            case 'h': {
                try_read("Shmem:", line, sharedMem);
                break;
            }
            case 'w': {
                try_read("SwapTotal:", line, swapTotalMem);
                try_read("SwapCached:", line, swapCacheMem);
                try_read("SwapFree:", line, swapFreeMem);
                break;
            }
            case 'R': {
                try_read("SReclaimable:", line, swapReclaimableMem);
                break;
            }
            default:
                break;
            }
            break;
        }
        case 'Z': {
            try_read("Zswap:", line, zSwapCompMem);
            try_read("Zswapped", line, zSwapOrigMem);
            break;
        }
        default:
            break;
        }
    }
    file.close();

    memory.totalMem = totalMem;
    memory.cachedMem = cachedMem + swapReclaimableMem - sharedMem;
    memory.sharedMem = sharedMem;
    const int64_t usedDiff =
        freeMem + cachedMem + swapReclaimableMem + buffersMem;
    memory.usedMem =
        (totalMem >= usedDiff) ? totalMem - usedDiff : totalMem - freeMem;
    memory.buffersMem = buffersMem;
    memory.avilableMem =
        (availableMem != 0) ? std::min(availableMem, totalMem) : freeMem;
    memory.totalSwap = swapTotalMem;
    memory.usedSwap = swapTotalMem - swapFreeMem - swapCacheMem;
    memory.cachedSwap = swapCacheMem;
    memory.zswap.usedZswapComp = zSwapCompMem;
    memory.zswap.usedZswapOrig = zSwapOrigMem;
}

void MemoryMonitor::try_read(const std::string &label,
                             const std::string &buffer, int64_t &variable) {
    if (buffer.compare(0, label.length(), label) != 0) {
        return;
    }

    variable = std::stoll(buffer.c_str() + label.length());
}
} // namespace CaptureProgress
