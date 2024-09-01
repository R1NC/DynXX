#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../external/cjson/cJSON.h"

#define JSON_READ_STR(j, k, v)                                                 \
    do                                                                         \
    {                                                                          \
        if (j->string && strcmp(j->string, k) == 0 && j->type == cJSON_String) \
        {                                                                      \
            v = j->valuestring;                                                \
        }                                                                      \
    } while (0)

#define JSON_READ_INT(j, k, v)                                                 \
    do                                                                         \
    {                                                                          \
        if (j->string && strcmp(j->string, k) == 0 && j->type == cJSON_Number) \
        {                                                                      \
            v = j->valueint;                                                   \
        }                                                                      \
    } while (0)

#define JSON_READ_DOUBLE(j, k, v)                                              \
    do                                                                         \
    {                                                                          \
        if (j->string && strcmp(j->string, k) == 0 && j->type == cJSON_Number) \
        {                                                                      \
            v = j->valuedouble;                                                \
        }                                                                      \
    } while (0)

extern "C"
{
#include "../../../external/lua/lauxlib.h"
}

#ifdef __cplusplus

#include <string>
#include "log/Log.hxx"
#include "net/HttpClient.hxx"
#include "lua/LuaBridge.hxx"

// WARNING: Export with `EMSCRIPTEN_KEEPALIVE` will cause Lua running automatically.
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define EXPORT_WASM extern "C" EMSCRIPTEN_KEEPALIVE
#define EXPORT_WASM_LUA extern "C"
#endif

namespace NGenXX
{

    static inline const std::string GetVersion(void)
    {
        static const std::string s("0.0.1");
        return s;
    }

}

#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_init(void)
{
    NGenXX::Net::HttpClient::create();
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_release(void)
{
    NGenXX::Net::HttpClient::destroy();
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
const char *ngenxx_get_version(void)
{
    auto s = NGenXX::GetVersion();
    char *c = (char *)malloc(s.size());
    strcpy(c, s.c_str());
    return c;
}

int ngenxx_get_versionL(lua_State *L)
{
    const char *res = ngenxx_get_version();
    lua_pushstring(L, res);
    return 1;
}

#pragma mark Log

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_log_set_level(int level)
{
    NGenXX::Log::setLevel(level);
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_log_set_callback(void (*callback)(int level, const char *log))
{
    NGenXX::Log::setCallback(callback);
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_log_print(int level, const char *content)
{
    NGenXX::Log::print(level, content);
}

int ngenxx_log_printL(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    int level;
    char *content;
    cJSON *json = cJSON_Parse(s);
    if (json)
    {
        cJSON *cj = json->child;
        while (cj)
        {
            JSON_READ_INT(cj, "level", level);
            JSON_READ_STR(cj, "content", content);
            cj = cj->next;
        }
        cJSON_free(json);
    }
    ngenxx_log_print(level, content);
    return 0;
}

#pragma mark Net

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
const char *ngenxx_net_http_request(const char *url, const char *params)
{
    const std::string sUrl(url);
    const std::string sParams(params);
    auto s = NGenXX::Net::HttpClient::request(sUrl, sParams);
    char *c = (char *)malloc(s.size());
    strcpy(c, s.c_str());
    return c;
}

int ngenxx_net_http_requestL(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    char *url, *params;
    cJSON *json = cJSON_Parse(s);
    if (json)
    {
        cJSON *cj = json->child;
        while (cj)
        {
            JSON_READ_STR(cj, "url", url);
            JSON_READ_STR(cj, "params", params);
            cj = cj->next;
        }
        cJSON_free(json);
    }
    const char *res = ngenxx_net_http_request(url, params);
    lua_pushstring(L, res);
    return 1;
}

#pragma mark Lua

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
void *ngenxx_L_create(void)
{
    lua_State *lstate = NGenXX::LuaBridge::create();
    
    NGenXX::LuaBridge::bindFunc(lstate, "ngenxx_get_versionL", ngenxx_get_versionL);

    NGenXX::LuaBridge::bindFunc(lstate, "ngenxx_log_printL", ngenxx_log_printL);

    NGenXX::LuaBridge::bindFunc(lstate, "ngenxx_net_http_requestL", ngenxx_net_http_requestL);

    return lstate;
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
void ngenxx_L_destroy(void *lstate)
{
    NGenXX::LuaBridge::destroy((lua_State *)lstate);
}

#ifndef __EMSCRIPTEN__
int ngenxx_L_loadF(void *lstate, const char *file)
{
    return NGenXX::LuaBridge::loadFile((lua_State *)lstate, file);
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
int ngenxx_L_loadS(void *lstate, const char *file)
{
    return NGenXX::LuaBridge::loadScript((lua_State *)lstate, file);
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
const char *ngenxx_L_call(void *lstate, const char *func, const char *params)
{
    return NGenXX::LuaBridge::callFunc((lua_State *)lstate, func, params);
}