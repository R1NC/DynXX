#ifndef NGENXX_INTERNAL_HXX_
#define NGENXX_INTERNAL_HXX_

#if defined(__cplusplus)

// WARNING: Export with `EMSCRIPTEN_KEEPALIVE` will cause Lua running automatically.
#define EXPORT extern "C"
#if defined(__EMSCRIPTEN__)
        #include <emscripten/emscripten.h>
        #define EXPORT_AUTO EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EXPORT_AUTO EXPORT
#endif

#define CHECK_APPLE_VERSION(iosV, macV) (!defined(__IPHONE_OS_VERSION_MIN_REQUIRED) || (__IPHONE_OS_VERSION_MIN_REQUIRED >= iosV) && (!defined(__MAC_OS_X_VERSION_MIN_REQUIRED) || (__MAC_OS_X_VERSION_MIN_REQUIRED >= macV)))

#if defined(__cpp_lib_to_chars)
#if CHECK_APPLE_VERSION(160500, 130400)
#if (__cpp_lib_to_chars >= 201611L)
#define USE_STD_CHAR_CONV_INT
#endif
#if (__cpp_lib_to_chars >= 201907L)
#define USE_STD_CHAR_CONV_FLOAT
#endif
#endif
#endif

#if defined(__cpp_lib_format)
#if CHECK_APPLE_VERSION(160500, 130400)
#define USE_STD_FORMAT
#endif
#endif

#endif

#endif // NGENXX_INTERNAL_HXX_
