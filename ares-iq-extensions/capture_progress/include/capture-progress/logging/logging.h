/**
 * @file logging.h
 *
 * @brief
 *
 * @date 10/1/25
 *
 * @author Tom Schmitz \<tschmitz@andrew.cmu.edu\>
 */

#ifndef CAPTURE_PROGRESS_LOGGING_H
#define CAPTURE_PROGRESS_LOGGING_H

extern "C" {

#include <stdio.h>
#include <inttypes.h>
#include <capture-progress/logging/logging_utils.h>

#define LOG_LEVEL_OFF 4
#define LOG_LEVEL_ERR 3
#define LOG_LEVEL_WRN 2
#define LOG_LEVEL_INF 1
#define LOG_LEVEL_DBG 0

#define _RESET_COLOR "\033[0m"
#define _ERR_COLOR "\033[38;2;193;29;40m"
#define _WRN_COLOR "\033[38;2;163;115;76m"
#define _INF_COLOR "\033[38;2;39;163;105m"
#define _DBG_COLOR _RESET_COLOR

#define _LOG_MSG_NO_ARGS(msg_) (void)printf(msg_ "\n")
#define _LOG_MSG_ARGS(msg_, ...) (void)printf(msg_ "\n", __VA_ARGS__)
#define _LOG_MSG(escape_code_, level_, msg_, ...) COND_CODE_0(IS_EMPTY(__VA_ARGS__), (_LOG_MSG_ARGS(escape_code_ level_ _RESET_COLOR msg_, __VA_ARGS__)), (_LOG_MSG_NO_ARGS(escape_code_ level_ _RESET_COLOR msg_)))

#if LOG_LEVEL < LOG_LEVEL_OFF
#define LOG_ERR(msg_, ...) _LOG_MSG(_ERR_COLOR, "[ERR]", msg_, __VA_ARGS__)
#else
#define LOG_ERR(...) (void)0
#endif // LOG_LEVEL < LOG_LEVEL_OFF

#if LOG_LEVEL < LOG_LEVEL_ERR
#define LOG_WRN(msg_, ...) _LOG_MSG(_WRN_COLOR, "[WRN]", msg_, __VA_ARGS__)
#else
#define LOG_WRN(...) (void)0
#endif // LOG_LEVEL < LOG_LEVEL_ERR

#if LOG_LEVEL < LOG_LEVEL_WRN
#define LOG_INF(msg_, ...) _LOG_MSG(_INF_COLOR, "[INF]", msg_, __VA_ARGS__)
#else
#define LOG_INF(...) (void)0
#endif // LOG_LEVEL < LOG_LEVEL_WRN

#if LOG_LEVEL < LOG_LEVEL_INF
#define LOG_DBG(msg_, ...) _LOG_MSG(_DBG_COLOR, "[DBG]", msg_, __VA_ARGS__)
#else
#define LOG_DBG(...) (void)0
#endif // LOG_LEVEL < LOG_LEVEL_INF
}

#endif //CAPTURE_PROGRESS_LOGGING_H
