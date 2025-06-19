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

#endif

#endif // NGENXX_INTERNAL_HXX_
