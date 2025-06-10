#ifndef NGENXX_SRC_CORE_UTIL_MACROTUTIL_HXX_
#define NGENXX_SRC_CORE_UTIL_MACROTUTIL_HXX_

#if defined(__cplusplus)

// WARNING: Export with `EMSCRIPTEN_KEEPALIVE` will cause Lua running automatically.
#define EXPORT extern "C"
    #if defined(__EMSCRIPTEN__)
        #include <emscripten/emscripten.h>
        #define EXPORT_AUTO EXPORT EMSCRIPTEN_KEEPALIVE
    #else
        #define EXPORT_AUTO EXPORT
#endif

#if defined(__cpp_lib_to_chars)
    #if !defined(__IPHONE_OS_VERSION_MIN_REQUIRED) || (__IPHONE_OS_VERSION_MIN_REQUIRED >= 160500)
        #if (__cpp_lib_to_chars >= 201611L) 
            #define USE_STD_CHAR_CONV_INT
        #endif
        #if (__cpp_lib_to_chars >= 201907L)
            #define USE_STD_CHAR_CONV_FLOAT
        #endif
    #endif
#endif

#endif

#endif // NGENXX_SRC_CORE_UTIL_MACROTUTIL_HXX_
