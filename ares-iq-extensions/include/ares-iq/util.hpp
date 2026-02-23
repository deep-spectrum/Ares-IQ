//
// Created by tschmitz on 11/7/25.
//

#ifndef VERSION_UTIL_HPP
#define VERSION_UTIL_HPP

#define KWARG_TO_STRUCT_PARAM(kwargs_, key_)                                   \
    do {                                                                       \
        if ((kwargs_).contains(#key_)) {                                       \
            (key_) = (kwargs_)[#key_].cast<decltype(key_)>();                  \
        }                                                                      \
    } while (false)

#define STRUCT_PARAM_TO_DICT(dict_, parameter_)                                \
    dict_[#parameter_] = this->parameter_

#endif // VERSION_UTIL_HPP