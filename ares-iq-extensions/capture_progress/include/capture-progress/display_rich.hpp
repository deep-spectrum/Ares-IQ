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

#include <string>
#include <sstream>

namespace CaptureProgressInternal {
    class RichBase {
        std::string _msg;

    protected:
        std::string _escape_enter;
    public:
        explicit RichBase(const std::string &msg, const std::string &escape) : _escape_enter(escape), _msg(msg) {}
        ~RichBase() = default;
        friend std::ostream &operator<<(std::ostream &os, const RichBase &rich);
    };

    class RichBlack : public RichBase {
    public:
        explicit RichBlack(const std::string &msg) : RichBase(msg, "\033[30m") {}
    };

    class RichRed : public RichBase {
    public:
        explicit RichRed(const std::string &msg) : RichBase(msg, "\033[31m") {}
    };

    class RichGreen : public RichBase {
    public:
        explicit RichGreen(const std::string &msg) : RichBase(msg, "\033[32m") {}
    };

    class RichYellow : public RichBase {
    public:
        explicit RichYellow(const std::string &msg) : RichBase(msg, "\033[33m") {}
    };

    class RichBlue : public RichBase {
    public:
        explicit RichBlue(const std::string &msg) : RichBase(msg, "\033[34m") {}
    };

    class RichMagenta : public RichBase {
    public:
        explicit RichMagenta(const std::string &msg) : RichBase(msg, "\033[35m") {}
    };

    class RichCyan : public RichBase {
    public:
        explicit RichCyan(const std::string &msg) : RichBase(msg, "\033[36m") {}
    };

    class RichWhite : public RichBase {
    public:
        explicit RichWhite(const std::string &msg) : RichBase(msg, "\033[37m") {}
    };

    class RichRgb : public RichBase {
        struct Rgb {
            Rgb() = default;
            explicit Rgb(int red, int green, int blue) : red(red), green(green), blue(blue) {}
            int red = 0;
            int green = 0;
            int blue = 0;
        };
    public:
        struct ForegroundRgb : Rgb {};
        struct BackgroundRgb : Rgb {};
        explicit RichRgb(const std::string &msg, const ForegroundRgb &rgb) : RichBase(msg, "") {
            std::stringstream oss;
            auto [red, green, blue] = _sanitize_rgb(rgb);

            oss << "\033[38;2" << red << ";" << green << ";" << blue << "m";
            _escape_enter = oss.str();
        }
        explicit RichRgb(const std::string &msg, const BackgroundRgb &rgb) : RichBase(msg, "") {
            std::stringstream oss;
            auto [red, green, blue] = _sanitize_rgb(rgb);

            oss << "\033[48;2" << red << ";" << green << ";" << blue << "m";
            _escape_enter = oss.str();
        }
        explicit RichRgb(const std::string &msg, const ForegroundRgb &foreground, const BackgroundRgb &background) : RichBase(msg, "") {
            std::stringstream oss;
            auto [fred, fgreen, fblue] = _sanitize_rgb(foreground);
            auto [bred, bgreen, bblue] = _sanitize_rgb(background);

            oss << "\033[38;2;" << fred << ";" << fgreen << ";" << fblue << ";48;2;" << bred << ";" << bgreen << ";" << bblue << "m";
            _escape_enter = oss.str();
        }

    private:
        static Rgb _sanitize_rgb(const Rgb &rgb);
    };

    class Rich8bitColor : public RichBase {
    public:
        struct Foreground { int code = 0; };
        struct Background { int code = 0; };
        Rich8bitColor(const std::string &msg, const Foreground &foreground_code) : RichBase(msg, "") {
            int code = foreground_code.code;
            code = std::min(255, std::max(code, 0));
            std::stringstream oss;
            oss << "\033[38;5;" << code << "m";
            _escape_enter = oss.str();
        }
        Rich8bitColor(const std::string &msg, const Background &background_code) : RichBase(msg, "") {
            int code = background_code.code;
            code = std::min(255, std::max(code, 0));
            std::stringstream oss;
            oss << "\033[38;5;" << code << "m";
            _escape_enter = oss.str();
        }
        Rich8bitColor(const std::string &msg, const Foreground &foreground_code, const Background &background_code) : RichBase(msg, "") {
            int fcode = foreground_code.code;
            int bcode = background_code.code;
            fcode = std::min(255, std::max(fcode, 0));
            bcode = std::min(255, std::max(bcode, 0));
            std::stringstream oss;
            oss << "\033[38;5;" << fcode << ";48;5;" << bcode << "m";
            _escape_enter = oss.str();
        }
    };

    std::ostream& operator<<(std::ostream &os, const RichBase &rich);
}

#endif //ARES_DISPLAY_RICH_HPP