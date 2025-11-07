//
// Created by tschmitz on 11/7/25.
//

#ifndef VERSION_UTIL_HPP
#define VERSION_UTIL_HPP

#define KWARG_TO_STRUCT_PARAM(_kwargs, _key)                                   \
    do {                                                                       \
        if (_kwargs.contains(#_key)) {                                         \
            _key = _kwargs[#_key].cast<decltype(_key)>();                      \
        }                                                                      \
    } while (false)

#endif // VERSION_UTIL_HPP