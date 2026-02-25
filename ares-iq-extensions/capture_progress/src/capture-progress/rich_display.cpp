/**
 * @file rich_display.cpp
 *
 * @brief
 *
 * @date 2/23/26
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#include <capture-progress/display_rich.hpp>
#include <iostream>

namespace CaptureProgressInternal {
constexpr char default_color[] = "\033[0m";

RichRgb::Rgb RichRgb::_sanitize_rgb(const Rgb &rgb) {
    Rgb rgb_;
    rgb_.red = std::min(255, std::max(rgb.red, 0));
    rgb_.green = std::min(255, std::max(rgb.green, 0));
    rgb_.blue = std::min(255, std::max(rgb.blue, 0));
    return rgb_;
}

void reset_cursor(std::ostream &os) {
    os << default_color << "\r" << std::flush;
}

void hide_cursor(std::ostream &os) { os << "\033[?25l" << std::flush; }

void restore_cursor(std::ostream &os) {
    os << "\r\n" << default_color << "\033[?25h" << std::flush;
}

std::ostream &operator<<(std::ostream &os, const RichBase &rich) {
    os << rich._escape_enter << rich._msg << default_color;
    return os;
}
} // namespace CaptureProgressInternal
