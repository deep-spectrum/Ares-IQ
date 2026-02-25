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

/**
 * @enum FontStyle
 * @brief Values used for styling the font. These values can be OR'ed together
 * for a combination of font styles.
 */
enum FontStyle : unsigned int {
    FONT_BOLD = 1,          ///< Bold font
    FONT_ITALIC = 2,        ///< Italic font
    FONT_UNDERLINE = 4,     ///< Underlined Font
    FONT_STRIKETHROUGH = 8, ///< Strikethrough font
};

inline FontStyle operator|(const FontStyle lhs, const FontStyle rhs) {
    return static_cast<FontStyle>(
        static_cast<std::underlying_type_t<FontStyle>>(lhs) |
        static_cast<std::underlying_type_t<FontStyle>>(rhs));
}

/**
 * @class RichBase
 * @brief Base class for Rich text objects.
 * @note This class should not be used directly, however, no one is stopping you
 * from using it directly.
 */
class RichBase {
    std::string _msg;

  protected:
    std::string _escape_enter;

  public:
    /**
     * .
     * @param escape Escape sequence for the Rich text.
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichBase(const char *escape, Args &&...args)
        : _escape_enter(escape) {
        std::stringstream ss;
        (ss << ... << std::forward<Args>(args));
        _msg = ss.str();
    }

    /**
     * .
     * @param style The font style.
     * @param base_escape The base escape sequence.
     * @param args The objects to print to the output stream.
     */
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

    /**
     * .
     */
    ~RichBase() = default;
    friend std::ostream &operator<<(std::ostream &os, const RichBase &rich);
};

/**
 * @class RichBlack
 * @brief Colors text black.
 */
class RichBlack : public RichBase {
  public:
    /**
     * .
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichBlack(Args &&...args)
        : RichBase("\033[30m", std::forward<Args>(args)...) {}

    /**
     * .
     * @param style The font style.
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichBlack(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[30", std::forward<Args>(args)...) {}
};

/**
 * @class RichRed
 * @brief Colors text red.
 */
class RichRed : public RichBase {
  public:
    /**
     * .
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichRed(Args &&...args)
        : RichBase("\033[31m", std::forward<Args>(args)...) {}

    /**
     * .
     * @param style The font style.
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichRed(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[31", std::forward<Args>(args)...) {}
};

/**
 * @class RichGreen
 * @brief Colors text green.
 */
class RichGreen : public RichBase {
  public:
    /**
     * .
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichGreen(Args &&...args)
        : RichBase("\033[32m", std::forward<Args>(args)...) {}

    /**
     * .
     * @param style The font style.
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichGreen(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[32", std::forward<Args>(args)...) {}
};

/**
 * @class RichYellow
 * @brief Colors text yellow.
 */
class RichYellow : public RichBase {
  public:
    /**
     * .
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichYellow(Args &&...args)
        : RichBase("\033[33m", std::forward<Args>(args)...) {}

    /**
     * .
     * @param style The font style.
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichYellow(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[33", std::forward<Args>(args)...) {}
};

/**
 * @class RichBlue
 * @brief Colors text blue.
 */
class RichBlue : public RichBase {
  public:
    /**
     * .
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichBlue(Args &&...args)
        : RichBase("\033[34m", std::forward<Args>(args)...) {}

    /**
     * .
     * @param style The font style.
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichBlue(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[34", std::forward<Args>(args)...) {}
};

/**
 * @class RichMagenta
 * @brief Colors text magenta.
 */
class RichMagenta : public RichBase {
  public:
    /**
     * .
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichMagenta(Args &&...args)
        : RichBase("\033[35m", std::forward<Args>(args)...) {}

    /**
     * .
     * @param style The font style.
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichMagenta(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[35", std::forward<Args>(args)...) {}
};

/**
 * @class RichCyan
 * @brief Colors text cyan.
 */
class RichCyan : public RichBase {
  public:
    /**
     * .
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichCyan(Args &&...args)
        : RichBase("\033[36m", std::forward<Args>(args)...) {}

    /**
     * .
     * @param style The font style.
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichCyan(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[36", std::forward<Args>(args)...) {}
};

/**
 * @class RichWhite
 * @brief Colors text white.
 */
class RichWhite : public RichBase {
  public:
    /**
     * .
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichWhite(Args &&...args)
        : RichBase("\033[37m", std::forward<Args>(args)...) {}

    /**
     * .
     * @param style The font style.
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichWhite(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[37", std::forward<Args>(args)...) {}
};

/**
 * @class RichDefault
 * @brief Colors text in the default color.
 */
class RichDefault : public RichBase {
  public:
    /**
     * .
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichDefault(Args &&...args)
        : RichBase("\033[0m", std::forward<Args>(args)...) {}

    /**
     * .
     * @param style The font style.
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichDefault(const FontStyle style, Args &&...args)
        : RichBase(style, "\033[0", std::forward<Args>(args)...) {}
};

/**
 * @class RichRgb
 * @brief Colors text in a custom foreground and/or background color.
 */
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
    /**
     * @struct ForegroundRgb
     * Foreground (text) RGB color.
     */
    struct ForegroundRgb : Rgb {
        /**
         * .
         * @param red Red part of the RGB value.
         * @param green Green part of the RGB value.
         * @param blue Blue part of the RGB value.
         *
         * @note If any value is <0, then it is rounded up to 0.
         * @note If any value is >255, then it is rounded down to 255.
         */
        explicit ForegroundRgb(int red, int green, int blue)
            : Rgb(red, green, blue) {}
    };

    /**
     * @struct BackgroundRgb
     * Background (color behind text) RGB color.
     */
    struct BackgroundRgb : Rgb {
        /**
         * .
         * @param red Red part of the RGB value.
         * @param green Green part of the RGB value.
         * @param blue Blue part of the RGB value.
         *
         * @note If any value is <0, then it is rounded up to 0.
         * @note If any value is >255, then it is rounded down to 255.
         */
        explicit BackgroundRgb(int red, int green, int blue)
            : Rgb(red, green, blue) {}
    };

    /**
     * .
     * @param rgb The foreground color.
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichRgb(const ForegroundRgb &rgb, Args &&...args)
        : RichBase("", std::forward<Args>(args)...) {
        std::stringstream oss;
        auto [red, green, blue] = _sanitize_rgb(rgb);

        oss << "\033[38;2;" << red << ";" << green << ";" << blue << "m";
        _escape_enter = oss.str();
    }

    /**
     * .
     * @param style The font style.
     * @param rgb The foreground color.
     * @param args The objects to print to the output stream.
     */
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

    /**
     * .
     * @param rgb The background color.
     * @param args The objects to print to the output stream.
     */
    template <typename... Args,
              typename = std::enable_if_t<(is_streamable<Args>::value && ...)>>
    explicit RichRgb(const BackgroundRgb &rgb, Args &&...args)
        : RichBase("", std::forward<Args>(args)...) {
        std::stringstream oss;
        auto [red, green, blue] = _sanitize_rgb(rgb);

        oss << "\033[48;2;" << red << ";" << green << ";" << blue << "m";
        _escape_enter = oss.str();
    }

    /**
     * .
     * @param style The font style.
     * @param rgb The background color.
     * @param args The objects to print to the output stream.
     */
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

    /**
     * .
     * @param foreground The foreground color.
     * @param background The background color.
     * @param args The objects to print to the output stream.
     */
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

    /**
     * .
     * @param style The font style.
     * @param foreground The foreground color.
     * @param background The background color.
     * @param args The objects to print to the output stream.
     */
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

/**
 * @struct Rich8bitColor
 * @brief Colors the text in the given 8-bit color.
 */
class Rich8bitColor : public RichBase {
  public:
    /**
     * @struct Foreground
     * @brief 8-bit foreground color code.
     */
    struct Foreground {
        /**
         * .
         * @param value The 8-bit color code.
         * @note If value <0, then it is rounded up to 0.
         * @note If value is >255, then it is rounded down to 255.
         */
        explicit Foreground(const int value) : code(value) {}
        int code = 0;
    };

    /**
     * @struct Background
     * @brief 8-bit background color code.
     */
    struct Background {
        /**
         * .
         * @param value The 8-bit color code.
         * @note If value <0, then it is rounded up to 0.
         * @note If value is >255, then it is rounded down to 255.
         */
        explicit Background(const int value) : code(value) {}
        int code = 0;
    };

    /**
     * .
     * @param foreground_code The foreground color.
     * @param args The objects to print to the output stream.
     */
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

    /**
     * .
     * @param style The font style.
     * @param foreground_code The foreground color.
     * @param args The objects to print to the output stream.
     */
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

    /**
     * .
     * @param background_code The background color.
     * @param args The objects to print to the output stream.
     */
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

    /**
     * .
     * @param style The font style.
     * @param background_code The background color.
     * @param args The objects to print to the output stream.
     */
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

    /**
     * .
     * @param foreground_code The foreground color.
     * @param background_code The background color.
     * @param args The objects to print to the output stream.
     */
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

    /**
     * .
     * @param style The font style.
     * @param foreground_code The foreground color.
     * @param background_code The background color.
     * @param args The objects to print to the output stream.
     */
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

/**
 * Resets the cursor to the beginning of the line. This will also clear any
 * formatting.
 * @param os The output stream to reset the cursor on.
 */
void reset_cursor(std::ostream &os);

/**
 * Hides the cursor visualization in the shell.
 * @param os The output stream to reset the cursor on.
 */
void hide_cursor(std::ostream &os);

/**
 * Restores the cursor visualization in the shell.
 * @param os The output stream to reset the cursor on.
 */
void restore_cursor(std::ostream &os);

/**
 * Operator overload for Rich objects.
 * @param os The output stream.
 * @param rich The Rich instance to print to the output stream.
 * @return The updated output stream.
 */
std::ostream &operator<<(std::ostream &os, const RichBase &rich);
} // namespace CaptureProgressInternal

#endif // ARES_DISPLAY_RICH_HPP