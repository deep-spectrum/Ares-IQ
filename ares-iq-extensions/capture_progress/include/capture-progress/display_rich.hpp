/**
 * @file display_rich.hpp
 *
 * @brief
 *
 * @date 2/23/26
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#ifndef ARES_DISPLAY_RICH_HPP
#define ARES_DISPLAY_RICH_HPP

#include <sstream>
#include <string>
#include <type_traits>

namespace CaptureProgressInternal {
template <typename T, typename = void>
struct is_streamable : std::false_type {};

template <typename T>
struct is_streamable<T, std::void_t<decltype(std::declval<std::ostream &>()
                                             << std::declval<T>())>>
    : std::true_type {};

enum FontStyle : unsigned int {
    FONT_BOLD = 1,
    FONT_ITALIC = 2,
    FONT_UNDERLINE = 4,
    FONT_STRIKETHROUGH = 8,
};

class RichBase {
    std::string _msg;

  protected:
    std::string _escape_enter;

  public:
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichBase(const char *escape, Args &&...args)
        : _escape_enter(escape) {
        std::stringstream ss;
        (ss << ... << std::forward<Args>(args));
        _msg = ss.str();
    }

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichBase(const FontStyle style, const char *base_escape,
                      Args &&...args)
        : _escape_enter(base_escape) {

        if (style & FONT_BOLD) {
            _escape_enter += ";1";
        }
        if (style & FONT_ITALIC) {
            _escape_enter += ";3";
        }
        if (style & FONT_UNDERLINE) {
            _escape_enter += ";4";
        }
        if (style & FONT_STRIKETHROUGH) {
            _escape_enter += ";9";
        }
        _escape_enter += "m";

        std::stringstream ss;
        (ss << ... << std::forward<Args>(args));
        _msg = ss.str();
    }

    ~RichBase() = default;
    friend std::ostream &operator<<(std::ostream &os, const RichBase &rich);
};

class RichBlack : public RichBase {
  public:
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichBlack(Args &&...args)
        : RichBase("\033[30m", std::forward<Args>(args)...) {}

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichBlack(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[30", std::forward<Args>(args)...) {}
};

class RichRed : public RichBase {
  public:
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichRed(Args &&...args)
        : RichBase("\033[31m", std::forward<Args>(args)...) {}

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichRed(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[31", std::forward<Args>(args)...) {}
};

class RichGreen : public RichBase {
  public:
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichGreen(Args &&...args)
        : RichBase("\033[32m", std::forward<Args>(args)...) {}

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichGreen(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[32", std::forward<Args>(args)...) {}
};

class RichYellow : public RichBase {
  public:
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichYellow(Args &&...args)
        : RichBase("\033[33m", std::forward<Args>(args)...) {}

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichYellow(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[33", std::forward<Args>(args)...) {}
};

class RichBlue : public RichBase {
  public:
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichBlue(Args &&...args)
        : RichBase("\033[34m", std::forward<Args>(args)...) {}

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichBlue(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[34", std::forward<Args>(args)...) {}
};

class RichMagenta : public RichBase {
  public:
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichMagenta(Args &&...args)
        : RichBase("\033[35m", std::forward<Args>(args)...) {}

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichMagenta(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[35", std::forward<Args>(args)...) {}
};

class RichCyan : public RichBase {
  public:
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichCyan(Args &&...args)
        : RichBase("\033[36m", std::forward<Args>(args)...) {}

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichCyan(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[36", std::forward<Args>(args)...) {}
};

class RichWhite : public RichBase {
  public:
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichWhite(Args &&...args)
        : RichBase("\033[37m", std::forward<Args>(args)...) {}

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichWhite(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[37", std::forward<Args>(args)...) {}
};

class RichDefault : public RichBase {
  public:
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichDefault(Args &&...args)
        : RichBase("\033[0m", std::forward<Args>(args)...) {}
};

class RichRgb : public RichBase {
    struct Rgb {
        Rgb() = default;
        explicit Rgb(int red, int green, int blue)
            : red(red), green(green), blue(blue) {}
        int red = 0;
        int green = 0;
        int blue = 0;
    };

  public:
    struct ForegroundRgb : Rgb {
        explicit ForegroundRgb(int red, int green, int blue)
            : Rgb(red, green, blue) {}
    };
    struct BackgroundRgb : Rgb {
        explicit BackgroundRgb(int red, int green, int blue)
            : Rgb(red, green, blue) {}
    };

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichRgb(const ForegroundRgb &rgb, Args &&...args)
        : RichBase("", std::forward<Args>(args)...) {
        std::stringstream oss;
        auto [red, green, blue] = _sanitize_rgb(rgb);

        oss << "\033[38;2;" << red << ";" << green << ";" << blue << "m";
        _escape_enter = oss.str();
    }

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichRgb(const FontStyle style, const ForegroundRgb &rgb,
                     Args &&...args)
        : RichBase(style, "", std::forward<Args>(args)...) {
        std::stringstream oss;
        auto [red, green, blue] = _sanitize_rgb(rgb);

        oss << "\033[38;2;" << red << ";" << green << ";" << blue;
        _escape_enter = oss.str() + _escape_enter;
    }

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichRgb(const BackgroundRgb &rgb, Args &&...args)
        : RichBase("", std::forward<Args>(args)...) {
        std::stringstream oss;
        auto [red, green, blue] = _sanitize_rgb(rgb);

        oss << "\033[48;2;" << red << ";" << green << ";" << blue << "m";
        _escape_enter = oss.str();
    }

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichRgb(const FontStyle style, const BackgroundRgb &rgb,
                     Args &&...args)
        : RichBase(style, "", std::forward<Args>(args)...) {
        std::stringstream oss;
        auto [red, green, blue] = _sanitize_rgb(rgb);

        oss << "\033[48;2;" << red << ";" << green << ";" << blue;
        _escape_enter = oss.str() + _escape_enter;
    }

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichRgb(const ForegroundRgb &foreground,
                     const BackgroundRgb &background, Args &&...args)
        : RichBase("", std::forward<Args>(args)...) {
        std::stringstream oss;
        auto [fred, fgreen, fblue] = _sanitize_rgb(foreground);
        auto [bred, bgreen, bblue] = _sanitize_rgb(background);

        oss << "\033[38;2;" << fred << ";" << fgreen << ";" << fblue << ";48;2;"
            << bred << ";" << bgreen << ";" << bblue << "m";
        _escape_enter = oss.str();
    }

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichRgb(const FontStyle style, const ForegroundRgb &foreground,
                     const BackgroundRgb &background, Args &&...args)
        : RichBase(style, "", std::forward<Args>(args)...) {
        std::stringstream oss;
        auto [fred, fgreen, fblue] = _sanitize_rgb(foreground);
        auto [bred, bgreen, bblue] = _sanitize_rgb(background);

        oss << "\033[38;2;" << fred << ";" << fgreen << ";" << fblue << ";48;2;"
            << bred << ";" << bgreen << ";" << bblue;
        _escape_enter = oss.str() + _escape_enter;
    }

  private:
    static Rgb _sanitize_rgb(const Rgb &rgb);
};

class Rich8bitColor : public RichBase {
  public:
    struct Foreground {
        int code = 0;
    };
    struct Background {
        int code = 0;
    };

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit Rich8bitColor(const Foreground &foreground_code, Args &&...args)
        : RichBase("", std::forward<Args>(args)...) {
        int code = foreground_code.code;
        code = std::min(255, std::max(code, 0));
        std::stringstream oss;
        oss << "\033[38;5;" << code << "m";
        _escape_enter = oss.str();
    }

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit Rich8bitColor(const FontStyle style,
                           const Foreground &foreground_code, Args &&...args)
        : RichBase(style, "", std::forward<Args>(args)...) {
        int code = foreground_code.code;
        code = std::min(255, std::max(code, 0));
        std::stringstream oss;
        oss << "\033[38;5;" << code;
        _escape_enter = oss.str() + _escape_enter;
    }

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit Rich8bitColor(const Background &background_code, Args &&...args)
        : RichBase("", std::forward<Args>(args)...) {
        int code = background_code.code;
        code = std::min(255, std::max(code, 0));
        std::stringstream oss;
        oss << "\033[38;5;" << code << "m";
        _escape_enter = oss.str();
    }

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit Rich8bitColor(const FontStyle style,
                           const Background &background_code, Args &&...args)
        : RichBase(style, "", std::forward<Args>(args)...) {
        int code = background_code.code;
        code = std::min(255, std::max(code, 0));
        std::stringstream oss;
        oss << "\033[38;5;" << code;
        _escape_enter = oss.str() + _escape_enter;
    }

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit Rich8bitColor(const Foreground &foreground_code,
                           const Background &background_code, Args &&...args)
        : RichBase("", std::forward<Args>(args)...) {
        int fcode = foreground_code.code;
        int bcode = background_code.code;
        fcode = std::min(255, std::max(fcode, 0));
        bcode = std::min(255, std::max(bcode, 0));
        std::stringstream oss;
        oss << "\033[38;5;" << fcode << ";48;5;" << bcode << "m";
        _escape_enter = oss.str();
    }

    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit Rich8bitColor(const FontStyle style,
                           const Foreground &foreground_code,
                           const Background &background_code, Args &&...args)
        : RichBase(style, "", std::forward<Args>(args)...) {
        int fcode = foreground_code.code;
        int bcode = background_code.code;
        fcode = std::min(255, std::max(fcode, 0));
        bcode = std::min(255, std::max(bcode, 0));
        std::stringstream oss;
        oss << "\033[38;5;" << fcode << ";48;5;" << bcode;
        _escape_enter = oss.str() + _escape_enter;
    }
};

void reset_cursor(std::ostream &os);
void hide_cursor(std::ostream &os);
void restore_cursor(std::ostream &os);

std::ostream &operator<<(std::ostream &os, const RichBase &rich);
} // namespace CaptureProgressInternal

#endif // ARES_DISPLAY_RICH_HPP