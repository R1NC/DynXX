#ifndef DYNXX_INCLUDE_C_MACRO_H_
#define DYNXX_INCLUDE_C_MACRO_H_

#ifdef __cplusplus
#define DYNXX_EXTERN_C_BEGIN extern "C" {
#define DYNXX_EXTERN_C_END }
#define DYNXX_EXPORT extern "C"
#if defined(__EMSCRIPTEN__)
    #include <emscripten/emscripten.h>
    #define DYNXX_EXPORT_AUTO DYNXX_EXPORT EMSCRIPTEN_KEEPALIVE
#else
    #define DYNXX_EXPORT_AUTO DYNXX_EXPORT
#endif
#else
#define DYNXX_EXTERN_C_BEGIN
#define DYNXX_EXTERN_C_END
#endif

#ifdef _MSC_VER
    #define DYNXX_FORCE_USED __pragma(comment(linker, "/include:" __FUNCSIG__))
    #define DYNXX_EXPORT_SYMBOL __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
    #define DYNXX_FORCE_USED __attribute__((used))
    #define DYNXX_EXPORT_SYMBOL __attribute__((visibility("default")))
#else
    #define DYNXX_FORCE_USED
    #define DYNXX_EXPORT_SYMBOL
#endif
#define DYNXX_DONTSTRIP DYNXX_FORCE_USED DYNXX_EXPORT_SYMBOL

#ifdef _MSC_VER
    #define DYNXX_DEPRECATED(msg) __declspec(deprecated(msg))
#elif defined(__GNUC__) || defined(__clang__)
    #define DYNXX_DEPRECATED(msg) __attribute__((__deprecated__(msg)))
#else
    #define DYNXX_DEPRECATED(msg)
#endif

#endif // DYNXX_INCLUDE_C_MACRO_H_
