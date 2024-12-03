#ifndef NGENXX_SRC_INNER_HXX_
#define NGENXX_SRC_INNER_HXX_

#if defined(__cplusplus)

// WARNING: Export with `EMSCRIPTEN_KEEPALIVE` will cause Lua running automatically.
#define EXPORT extern "C"
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#define EXPORT_AUTO EXPORT EMSCRIPTEN_KEEPALIVE
#else
#define EXPORT_AUTO EXPORT
#endif

#endif

#endif // NGENXX_SRC_INNER_HXX_