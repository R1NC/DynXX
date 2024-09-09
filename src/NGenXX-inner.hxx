#ifndef NGENXX_INNER_HXX_
#define NGENXX_INNER_HXX_

#ifdef __cplusplus

typedef struct NGenXXHandle
{
    void *sqlite;
    void *kv;
    void *lua;
} NGenXXHandle;

#define HTTP_HEADERS_MAX_COUNT 100
#define HTTP_HEADER_MAX_LENGTH 8190

// WARNING: Export with `EMSCRIPTEN_KEEPALIVE` will cause Lua running automatically.
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define EXPORT_WASM extern "C" EMSCRIPTEN_KEEPALIVE
#define EXPORT_WASM_LUA extern "C"
#endif

#endif

#endif // NGENXX_INNER_HXX_