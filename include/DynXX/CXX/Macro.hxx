#pragma once

#if defined(__cpp_lib_to_chars) && defined(DYNXX_USE_STD_TO_CHARS)
        #if (__cpp_lib_to_chars >= 201611L)
                #define DYNXX_USE_STD_CHAR_CONV_INT
        #endif
        #if (__cpp_lib_to_chars >= 201907L)
                #define DYNXX_USE_STD_CHAR_CONV_FLOAT
        #endif
#endif

#if defined(__cpp_lib_format) && defined(DYNXX_USE_STD_TO_CHARS)
        #define DYNXX_USE_STD_FORMAT
#endif

#ifndef DYNXX_NO_UNIQUE_ADDRESS
#  if defined(_MSC_VER) && _MSC_VER >= 1929
#    define DYNXX_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#  elif defined(__has_cpp_attribute) && __has_cpp_attribute(no_unique_address)
#    define DYNXX_NO_UNIQUE_ADDRESS [[no_unique_address]]
#  else
#    define DYNXX_NO_UNIQUE_ADDRESS
#  endif
#endif

#ifndef DYNXX_USE_DEDUCING_THIS
#if defined(__cpp_explicit_this_parameter) && (__cpp_explicit_this_parameter >= 202110L) && !defined(__OHOS__)
#define DYNXX_USE_DEDUCING_THIS 1
#else
#define DYNXX_USE_DEDUCING_THIS 0
#endif
#endif
