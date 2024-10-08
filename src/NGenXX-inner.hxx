#ifndef NGENXX_INNER_HXX_
#define NGENXX_INNER_HXX_

#ifdef __cplusplus

// WARNING: Export with `EMSCRIPTEN_KEEPALIVE` will cause Lua running automatically.
#define EXPORT extern "C"
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#define EXPORT_AUTO EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EXPORT_AUTO EXPORT
#endif

constexpr size HTTP_HEADERS_MAX_COUNT = 100;
constexpr size HTTP_HEADER_MAX_LENGTH = 8190;
constexpr size HTTP_FORM_FIELD_MAX_LENGTH = 100 * 1024 * 1024;

#endif

#endif // NGENXX_INNER_HXX_