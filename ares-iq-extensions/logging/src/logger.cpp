//
// Created by tschmitz on 11/14/25.
//

#include <logging/logger.hpp>
#include <cstdio>
#include <cstdarg>

constexpr const char *reset_color = "\033[0m";
constexpr const char *dbg_color = reset_color;
constexpr const char *inf_color = "\033[38;2;39;163;105m";
constexpr const char *wrn_color = "\033[38;2;163;115;76m";
constexpr const char *err_color = "\033[38;2;193;29;40m";
constexpr const char *crit_color = "\033[38;2;117;80;123m";

Logger::Logger(const char *name, LogLevel level) {
    _name = name;
    _level = level;
}

void Logger::set_log_level(LogLevel level) {
    _level = level;
}

void Logger::log(LogLevel level, const char *fmt, ...) const {
    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);

    int len = vsnprintf(nullptr, 0, fmt, args_copy);
    va_end(args_copy);

    if (len < 0) {
        return;
    }

    char *msg = new char[len + 1];
    vsnprintf(msg, len + 1, fmt, args);
    va_end(args);

    switch (level) {
        case LOG_LEVEL_DBG: {
            _log_dbg(msg);
            break;
        }
        case LOG_LEVEL_INFO: {
            _log_inf(msg);
            break;
        }
        case LOG_LEVEL_WARN: {
            _log_wrn(msg);
            break;
        }
        case LOG_LEVEL_ERROR: {
            _log_err(msg);
            break;
        }
        case LOG_LEVEL_CRITICAL: {
            _log_crit(msg);
            break;
        }
        default:
            break;
    }

    delete[] msg;
}

void Logger::_log_dbg(const char *msg) const {
    printf("%s[DBG]%s %s: %s\n", dbg_color, reset_color, _name, msg);
}

void Logger::_log_inf(const char *msg) const {
    printf("%s[INFO]%s %s: %s\n", inf_color, reset_color, _name, msg);
}

void Logger::_log_wrn(const char *msg) const {
    printf("%s[WARN]%s %s: %s\n", wrn_color, reset_color, _name, msg);
}

void Logger::_log_err(const char *msg) const {
    printf("%s[ERR]%s %s: %s\n", err_color, reset_color, _name, msg);
}

void Logger::_log_crit(const char *msg) const {
    printf("%s[CRIT]%s %s: %s\n", crit_color, reset_color, _name, msg);
}
