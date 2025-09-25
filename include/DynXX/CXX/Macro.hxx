#pragma once

#if defined(__cpp_lib_to_chars) && defined(USE_STD_TO_CHARS)
        #if (__cpp_lib_to_chars >= 201611L)
                #define USE_STD_CHAR_CONV_INT
        #endif
        #if (__cpp_lib_to_chars >= 201907L)
                #define USE_STD_CHAR_CONV_FLOAT
        #endif
#endif

#if defined(__cpp_lib_format) && defined(USE_STD_TO_CHARS)
        #define USE_STD_FORMAT
#endif
