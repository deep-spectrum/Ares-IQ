//
// Created by tschmitz on 11/14/25.
//

#ifndef VERSION_LOGGER_HPP
#define VERSION_LOGGER_HPP

#if defined(USE_PYTHON_LOGGERS)
#include <memory>
class PieceOfShitIdiom;
#endif // defined(USE_PYTHON_LOGGERS)

class Logger {
  public:
    enum LogLevel : unsigned int {
        LOG_LEVEL_DBG = 0,
        LOG_LEVEL_INFO = 1,
        LOG_LEVEL_WARN = 2,
        LOG_LEVEL_ERROR = 3,
        LOG_LEVEL_CRITICAL = 4,
        LOG_LEVEL_OFF = 5,
    };

    explicit Logger(const char *name, LogLevel level);
    ~Logger() = default;

    void set_log_level(LogLevel level);
    [[nodiscard]] LogLevel get_log_level() const;
    void log(LogLevel level, const char *fmt, ...) const;

  private:
#if defined(USE_PYTHON_LOGGERS)
    std::unique_ptr<PieceOfShitIdiom> _impl;
#else
    const char *_name;
    LogLevel _level;

    void _log_dbg(const char *msg) const;
    void _log_inf(const char *msg) const;
    void _log_wrn(const char *msg) const;
    void _log_err(const char *msg) const;
    void _log_crit(const char *msg) const;
#endif
};

#endif // VERSION_LOGGER_HPP