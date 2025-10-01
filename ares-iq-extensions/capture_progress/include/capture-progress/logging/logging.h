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

    // todo refactor this
#define LOG_DBG(msg_, ...) COND_CODE_0(IS_EMPTY(__VA_ARGS__), ((void)printf("[DBG] " msg_ "\n", __VA_ARGS__)), ((void)printf("[DBG] " msg_ "\n")))
#define LOG_INF(msg_, ...) (void)printf("[INF] " msg_ "\n", __VA_ARGS__)
#define LOG_WRN(msg_, ...) (void)printf("[WRN] " msg_ "\n", __VA_ARGS__)
#define LOG_ERR(msg_, ...) (void)printf("[ERR] " msg_ "\n", __VA_ARGS__)

}

#endif //CAPTURE_PROGRESS_LOGGING_H
