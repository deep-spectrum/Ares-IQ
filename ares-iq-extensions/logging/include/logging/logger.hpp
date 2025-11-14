//
// Created by tschmitz on 11/14/25.
//

#ifndef VERSION_LOGGER_HPP
#define VERSION_LOGGER_HPP

class Logger {
public:
    enum LogLevel {
        LOG_LEVEL_OFF,
        LOG_LEVEL_DBG,
        LOG_LEVEL_INFO,
        LOG_LEVEL_WARN,
        LOG_LEVEL_ERROR,
        LOG_LEVEL_CRITICAL,
    };

    explicit Logger(const char *name, LogLevel level);
    ~Logger() = default;

    void set_log_level(LogLevel level);
    void log(LogLevel level, const char *fmt, ...) const;
private:
    const char *_name;
    LogLevel _level;

    void _log_dbg(const char *msg) const;
    void _log_inf(const char *msg) const;
    void _log_wrn(const char *msg) const;
    void _log_err(const char *msg) const;
    void _log_crit(const char *msg) const;
};

#endif //VERSION_LOGGER_HPP