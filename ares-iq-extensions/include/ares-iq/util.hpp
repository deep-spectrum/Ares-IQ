//
// Created by tschmitz on 11/7/25.
//

#ifndef VERSION_UTIL_HPP
#define VERSION_UTIL_HPP

#include "../logging/include/logging/internal/logging_utils.h"

#define KWARG_TO_STRUCT_PARAM(kwargs_, key_)                                   \
    do {                                                                       \
        if ((kwargs_).contains(#key_)) {                                       \
            (key_) = (kwargs_)[#key_].cast<decltype(key_)>();                  \
        }                                                                      \
    } while (false)

#define Z_STRUCT_PARAM_TO_DICT(dict_, parameter_)                              \
    dict_[#parameter_] = this->parameter_

#define Z_STRUCT_CONTAINER_TO_DICT(dict_, parameter_, container_)              \
    dict_[#parameter_] = (container_).parameter_

#define STRUCT_PARAM_TO_DICT(dict_, parameter_, container_...)                 \
    COND_CODE_0(IS_EMPTY(container_),                                          \
                (Z_STRUCT_CONTAINER_TO_DICT(dict_, parameter_, container_)),   \
                (Z_STRUCT_PARAM_TO_DICT(dict_, parameter_)))

#endif // VERSION_UTIL_HPP