/**
 * @file progress.cpp
 *
 * @brief The class implementation of the progress bar.
 *
 * @date 10/1/25
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#include <capture-progress/logging/logging.h>
#include <capture-progress/progress.hpp>
#include <cmath>
#include <codecvt>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>

namespace CaptureProgress {
constexpr char32_t bar_char(U'━');
constexpr char32_t progress_stub(U'╸');
constexpr uint32_t bar_length = 40;
constexpr char current_progress_color[] = "\033[38;2;249;39;114m";
constexpr char remaining_progress_color[] = "\033[38;2;58;59;58m";
constexpr char magenta_color[] = "\033[38;2;162;70;187m";
constexpr char yellow_color[] = "\033[38;2;163;115;76m";
constexpr char green_color[] = "\033[38;2;115;157;30m";
constexpr char default_color[] = "\033[0m";

constexpr char opening_statement[] = "Capturing... ";

constexpr long seconds_per_hour = 3600;
constexpr long seconds_per_minute = 60;
constexpr long minutes_per_hour = 60;

Progress::Progress(uint64_t captures, uint64_t samples_per_capture) {
    _total_samples = captures * samples_per_capture;
    _spc = samples_per_capture;
    LOG_DBG("Number of captures: %" PRIu64, captures);
    LOG_DBG("Samples per capture: %" PRIu64, samples_per_capture);
    LOG_DBG("Total samples to capture: %" PRIu64, _total_samples);
}

Progress::~Progress() { stop(); }

void Progress::start() {
    LOG_INF("Starting progress bar");
    _start = std::chrono::system_clock::now();
    _refresh_thread = std::thread(&Progress::_refresh_task, this);
}

void Progress::update() {
    LOG_DBG("Updating progress bar");
    std::lock_guard<std::mutex> guard(this->_samples_mtx);
    _samples_captured += _spc;
}

void Progress::stop() {
    LOG_INF("Stopping progress bar");
    _terminate.store(true);
    if (_refresh_thread.joinable()) {
        _refresh_thread.join();
    }
}

void Progress::_refresh_task() {
    _init_bar();

    while (!_completed() && !_terminate) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        _draw();
    }

    _finalize();
    _restore_cursor();
}

void Progress::_init_bar() {
    LOG_INF("Initializing");
    _hide_cursor();
    _draw_opening();
    _draw_bar();
    _draw_rate();
    _draw_time_elapsed();
    _reset_cursor();
}

void Progress::_draw() {
    LOG_DBG("Refreshing");
    _update_rate();
    _draw_opening();
    _draw_bar();
    _draw_rate();
    _draw_time_elapsed();
    _reset_cursor();
}

void Progress::_finalize() {
    LOG_INF("Finalizing");
    if (!_completed()) {
        // leave bar as is
        return;
    }

    std::u32string complete_bar(bar_length, bar_char);
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::string bar = converter.to_bytes(complete_bar);

    _draw_opening();
    std::cout << green_color << bar << magenta_color << " 100% ";
    _draw_rate();
    _draw_time_elapsed();
}

bool Progress::_completed() {
    std::lock_guard<std::mutex> guard(this->_samples_mtx);
    return _total_samples <= _samples_captured;
}

void Progress::_update_rate() {
    std::lock_guard<std::mutex> guard(this->_samples_mtx);
    auto time_diff =
        std::chrono::duration<double>(std::chrono::system_clock::now() - _start)
            .count();
    if (iszero(time_diff)) {
        return;
    }
    double rate = static_cast<double>(_samples_captured) / time_diff / 1e6;
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << rate << " MS/sec ";
    _rate = oss.str();
    _percent_complete = static_cast<double>(_samples_captured) /
                        static_cast<double>(_total_samples);
}

void Progress::_draw_opening() {
    std::cout << default_color << opening_statement;
}

void Progress::_draw_bar() const {
    uint64_t complete_bars = static_cast<uint64_t>(
        static_cast<double>(bar_length) * _percent_complete);
    std::u32string complete_bar;
    std::u32string remaining_bar;

    switch (complete_bars) {
    case 0:
        complete_bar = U"";
        remaining_bar = std::u32string(bar_length, bar_char);
        break;
    case 1:
        complete_bar = progress_stub;
        remaining_bar = std::u32string(bar_length - 1, bar_char);
        break;
    default:
        complete_bar =
            std::u32string(complete_bars - 1, bar_char) + progress_stub;
        remaining_bar = std::u32string(bar_length - complete_bars, bar_char);
        break;
    }

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::string utf8_complete_bar = converter.to_bytes(complete_bar);
    std::string utf8_remaining_bar = converter.to_bytes(remaining_bar);

    std::cout << current_progress_color << utf8_complete_bar
              << remaining_progress_color << utf8_remaining_bar << " "
              << magenta_color << static_cast<int>(_percent_complete * 100.0)
              << "% ";
}

void Progress::_draw_rate() { std::cout << magenta_color << _rate; }

void Progress::_draw_time_elapsed() {
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now() - _start)
                       .count();
    long hrs = elapsed / seconds_per_hour;
    long min = (elapsed / seconds_per_minute) % minutes_per_hour;
    long sec = elapsed % seconds_per_minute;

    std::ostringstream oss;
    oss << std::setw(1) << std::setfill('0') << hrs << ":" << std::setw(2)
        << std::setfill('0') << min << ":" << std::setw(2) << std::setfill('0')
        << sec;
    std::cout << yellow_color << oss.str();
}

void Progress::_reset_cursor() {
    std::cout << default_color << "\r" << std::flush;
}

void Progress::_hide_cursor() { std::cout << "\033[?25l" << std::flush; }

void Progress::_restore_cursor() {
    std::cout << "\r\n" << default_color << "\033[?25h" << std::flush;
}
} // namespace CaptureProgress
