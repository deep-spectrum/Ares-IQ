//
// Created by tschmitz on 11/17/25.
//

#ifndef LOGGING_LOG_HPP
#define LOGGING_LOG_HPP

#include <logging/logger.hpp>
#include <logging/logging_utils.h>

#define Z_LOG_LEVEL_DBG  0
#define Z_LOG_LEVEL_INF  1
#define Z_LOG_LEVEL_WRN  2
#define Z_LOG_LEVEL_ERR  3
#define Z_LOG_LEVEL_CRIT 4
#define Z_LOG_LEVEL_OFF  5

#if LOG_LEVEL == Z_LOG_LEVEL_DBG
#define DEFAULT_LOG_LEVEL Logger::LogLevel::LOG_LEVEL_DBG
#elif LOG_LEVEL == Z_LOG_LEVEL_INF
#define DEFAULT_LOG_LEVEL Logger::LogLevel::LOG_LEVEL_INFO
#elif LOG_LEVEL == Z_LOG_LEVEL_WRN
#define DEFAULT_LOG_LEVEL Logger::LogLevel::LOG_LEVEL_WARN
#elif LOG_LEVEL == Z_LOG_LEVEL_ERR
#define DEFAULT_LOG_LEVEL Logger::LogLevel::LOG_LEVEL_ERROR
#elif LOG_LEVEL == Z_LOG_LEVEL_CRIT
#define DEFAULT_LOG_LEVEL Logger::LogLevel::LOG_LEVEL_CRITICAL
#else
#define DEFAULT_LOG_LEVEL Logger::LogLevel::LOG_LEVEL_OFF
#endif

#define Z_REGISTER_LOGGER_DEFAULT(name_)                                       \
    static Logger __logger__(#name_, DEFAULT_LOG_LEVEL);                       \
    static Logger::LogLevel __saved_level__ = DEFAULT_LOG_LEVEL;
#define Z_REGISTER_LOGGER(name_, level_)                                       \
    static Logger __logger__(#name_, Logger::LogLevel::level_);                \
    static Logger::LogLevel __saved_level__ = Logger::LogLevel::level_;

#define LOG_MODULE_REGISTER(name_, level_...)                                  \
    COND_CODE_0(IS_EMPTY(level_), (Z_REGISTER_LOGGER(name_, level_)),          \
                (Z_REGISTER_LOGGER_DEFAULT(name_)))

#define LOG_DBG(msg_, ...)                                                     \
    COND_CODE_0(                                                               \
        IS_EMPTY(__VA_ARGS__),                                                 \
        (__logger__.log(Logger::LogLevel::LOG_LEVEL_DBG, msg_, __VA_ARGS__)),  \
        (__logger__.log(Logger::LogLevel::LOG_LEVEL_DBG, msg_)))
#define LOG_INF(msg_, ...)                                                     \
    COND_CODE_0(                                                               \
        IS_EMPTY(__VA_ARGS__),                                                 \
        (__logger__.log(Logger::LogLevel::LOG_LEVEL_INFO, msg_, __VA_ARGS__)), \
        (__logger__.log(Logger::LogLevel::LOG_LEVEL_INFO, msg_)))
#define LOG_WRN(msg_, ...)                                                     \
    COND_CODE_0(                                                               \
        IS_EMPTY(__VA_ARGS__),                                                 \
        (__logger__.log(Logger::LogLevel::LOG_LEVEL_WARN, msg_, __VA_ARGS__)), \
        (__logger__.log(Logger::LogLevel::LOG_LEVEL_WARN, msg_)))
#define LOG_ERR(msg_, ...)                                                     \
    COND_CODE_0(IS_EMPTY(__VA_ARGS__),                                         \
                (__logger__.log(Logger::LogLevel::LOG_LEVEL_ERROR, msg_,       \
                                __VA_ARGS__)),                                 \
                (__logger__.log(Logger::LogLevel::LOG_LEVEL_ERROR, msg_)))
#define LOG_CRIT(msg_, ...)                                                    \
    COND_CODE_0(IS_EMPTY(__VA_ARGS__),                                         \
                (__logger__.log(Logger::LogLevel::LOG_LEVEL_CRITICAL, msg_,    \
                                __VA_ARGS__)),                                 \
                (__logger__.log(Logger::LogLevel::LOG_LEVEL_CRITICAL, msg_)))

#define SAVE_LOG_LEVEL_AND_OVERRIDE(new_level)                                 \
    do {                                                                       \
        __saved_level__ = __logger__.get_log_level();                          \
        __logger__.set_log_level(Logger::LogLevel::new_level);                 \
    } while (false)
#define RESTORE_LOG_LEVEL()                                                    \
    do {                                                                       \
        __logger__.set_log_level(__saved_level__);                             \
    } while (false)

#endif // LOGGING_LOG_HPP