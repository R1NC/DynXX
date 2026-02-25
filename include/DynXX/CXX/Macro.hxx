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

#ifndef NO_UNIQUE_ADDRESS
#  if defined(_MSC_VER) && _MSC_VER >= 1929
#    define NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#  elif defined(__has_cpp_attribute) && __has_cpp_attribute(no_unique_address)
#    define NO_UNIQUE_ADDRESS [[no_unique_address]]
#  else
#    define NO_UNIQUE_ADDRESS
#  endif
#endif
