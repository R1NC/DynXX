#ifndef NGENXX_INNER_HXX_
#define NGENXX_INNER_HXX_

#ifdef __cplusplus

// WARNING: Export with `EMSCRIPTEN_KEEPALIVE` will cause Lua running automatically.
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define EXPORT_WASM extern "C" EMSCRIPTEN_KEEPALIVE
#define EXPORT_WASM_LUA extern "C"
#endif

typedef struct NGenXXHandle
{
    void *sqlite;
    void *kv;
    void *lua;
    void *qjs;
} NGenXXHandle;

#define HTTP_HEADERS_MAX_COUNT 100
#define HTTP_HEADER_MAX_LENGTH 8190

static inline const char *str2charp(std::string s)
{
    const char *c = s.c_str();
    char *nc = (char *)malloc(strlen(c) + 1);
    strcpy(nc, c);
    return nc;
}

#endif

#endif // NGENXX_INNER_HXX_