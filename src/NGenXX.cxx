#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../external/cjson/cJSON.h"

#define HTTP_HEADERS_MAX_COUNT 100
#define HTTP_HEADER_MAX_LENGTH 8190

extern "C"
{
#include "../../../external/lua/lauxlib.h"

    typedef struct NGenXXHandle
    {
#ifdef USE_LUA
        lua_State *lua_state;
#endif
    } NGenXXHandle;
}

#ifdef __cplusplus

#include <string>
#include "log/Log.hxx"
#include "net/HttpClient.hxx"
#include "util/JsonUtil.hxx"
#include <vector>
#include <string>
#ifdef USE_LUA
#include "lua/LuaBridge.hxx"
#endif

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
const char *ngenxx_get_version(void)
{
    auto s = NGenXX::GetVersion();
    char *c = (char *)malloc(s.size());
    strcpy(c, s.c_str());
    return c;
}

#ifdef USE_LUA
int ngenxx_get_versionL(lua_State *L)
{
    const char *res = ngenxx_get_version();
    lua_pushstring(L, res);
    return 1;
}
#endif

#ifdef USE_LUA
void export_funcs_for_lua(void *handle);
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
void *ngenxx_init(bool use_lua)
{
    NGenXXHandle *handle = (NGenXXHandle *)malloc(sizeof(NGenXXHandle));
#ifdef USE_LUA
    handle->lua_state = NGenXX::LuaBridge::create();
    export_funcs_for_lua(handle);
#endif
    NGenXX::Net::HttpClient::create();
    return handle;
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
void ngenxx_release(void *handle)
{
    if (handle == NULL)
        return;
    NGenXXHandle *h = (NGenXXHandle *)handle;
#ifdef USE_LUA
    if (h->lua_state)
    {
        NGenXX::LuaBridge::destroy(h->lua_state);
    }
#endif
    NGenXX::Net::HttpClient::destroy();
    free(h);
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

#ifdef USE_LUA
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
            JSON_READ_NUM(cj, level);
            JSON_READ_STR(cj, content);
            cj = cj->next;
        }
        cJSON_free(json);
    }
    ngenxx_log_print(level, content);
    return 0;
}
#endif

#pragma mark Net

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
const char *ngenxx_net_http_request(const char *url, const char *params, int method, char **headers_v, int headers_c, long timeout)
{
    if (url == NULL)
        return NULL;
    const std::string sUrl(url);
    const std::string sParams(params);
    std::vector<std::string> vHeaders;
    if (headers_v != NULL && headers_c > 0)
    {
        vHeaders = std::vector<std::string>(headers_v, headers_v + headers_c);
    }

    auto s = NGenXX::Net::HttpClient::request(sUrl, sParams, method, vHeaders, timeout);
    char *c = (char *)malloc(s.size());
    strcpy(c, s.c_str());
    return c;
}

#ifdef USE_LUA
int ngenxx_net_http_requestL(lua_State *L)
{
    const char *s = luaL_checkstring(L, 1);
    char *url = NULL, *params = NULL;
    int method;
    char **headers = NULL;
    int headers_c;
    long timeout;
    cJSON *json = cJSON_Parse(s);
    if (json)
    {
        if (json->type == cJSON_Object)
        {
            cJSON *oj = json->child;
            while (oj)
            {
                JSON_READ_STR(oj, url);
                JSON_READ_STR(oj, params);
                JSON_READ_NUM(oj, method);
                JSON_READ_NUM(oj, timeout);
                JSON_READ_STR_ARRAY(oj, headers, headers_c, HTTP_HEADERS_MAX_COUNT, HTTP_HEADER_MAX_LENGTH);
                oj = oj->next;
            }
        }
        cJSON_free(json);
    }
    const char *res = ngenxx_net_http_request(url, params, method, headers, headers_c, timeout);
    lua_pushstring(L, res);
    return 1;
}
#endif

#ifdef USE_LUA

#pragma mark Lua

#ifndef __EMSCRIPTEN__
bool ngenxx_L_loadF(void *handle, const char *file)
{
    NGenXXHandle *h = (NGenXXHandle *)handle;
    return NGenXX::LuaBridge::loadFile(h->lua_state, file) == LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
bool ngenxx_L_loadS(void *handle, const char *file)
{
    NGenXXHandle *h = (NGenXXHandle *)handle;
    return NGenXX::LuaBridge::loadScript(h->lua_state, file) == LUA_OK;
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
const char *ngenxx_L_call(void *handle, const char *func, const char *params)
{
    NGenXXHandle *h = (NGenXXHandle *)handle;
    return NGenXX::LuaBridge::callFunc(h->lua_state, func, params);
}

void export_funcs_for_lua(void *handle)
{
    NGenXXHandle *h = (NGenXXHandle *)handle;
    NGenXX::LuaBridge::bindFunc(h->lua_state, "ngenxx_get_versionL", ngenxx_get_versionL);
    NGenXX::LuaBridge::bindFunc(h->lua_state, "ngenxx_log_printL", ngenxx_log_printL);
    NGenXX::LuaBridge::bindFunc(h->lua_state, "ngenxx_net_http_requestL", ngenxx_net_http_requestL);
}

#endif