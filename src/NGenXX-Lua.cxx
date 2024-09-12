#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <functional>

#include "../../../external/cjson/cJSON.h"
extern "C"
{
#include "../../../external/lua/lauxlib.h"
}

#include "util/JsonUtil.hxx"
#include "lua/LuaBridge.hxx"
#include "../include/NGenXX.h"
#include "NGenXX-Lua.hxx"

static inline void parse_lua_func_params(lua_State *L, std::function<void(cJSON *)> callback)
{
    const char *s = luaL_checkstring(L, 1);
    cJSON *json = cJSON_Parse(s);
    if (json)
    {
        if (json->type == cJSON_Object)
        {
            cJSON *oj = json->child;
            while (oj)
            {
                callback(oj);
                oj = oj->next;
            }
        }
        cJSON_free(json);
    }
}

#define BIND_LUA_FUNC(h, f) NGenXX::LuaBridge::bindFunc((lua_State *)(h->lua), #f, f);

int ngenxx_get_versionL(lua_State *L)
{
    const char *res = ngenxx_get_version();
    lua_pushstring(L, res);
    return 1;
}

#pragma mark Device.DeviceInfo

int ngenxx_device_typeL(lua_State *L)
{
    int res = ngenxx_device_type();
    lua_pushinteger(L, res);
    return 1;
}

int ngenxx_device_nameL(lua_State *L)
{
    const char *res = ngenxx_device_name();
    lua_pushstring(L, res);
    return 1;
}

int ngenxx_device_os_versionL(lua_State *L)
{
    const char *res = ngenxx_device_os_version();
    lua_pushstring(L, res);
    return 1;
}

int ngenxx_device_cpu_archL(lua_State *L)
{
    int res = ngenxx_device_cpu_arch();
    lua_pushinteger(L, res);
    return 1;
}

#pragma mark Log

int ngenxx_log_printL(lua_State *L)
{
    int level = -1;
    char *content = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, level);
        JSON_READ_STR(j, content); 
    });
    if (level == -1 || content == NULL)
        return LUA_ERRRUN;
    ngenxx_log_print(level, content);
    return 1;
}

#pragma mark Net.Http

int ngenxx_net_http_requestL(lua_State *L)
{
    char *url = NULL, *params = NULL;
    int method = -1;
    char **headers_v = NULL;
    int headers_c;
    long timeout;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_STR(j, url);
        JSON_READ_STR(j, params);
        JSON_READ_NUM(j, method);
        JSON_READ_NUM(j, headers_c);
        JSON_READ_NUM(j, timeout);
        JSON_READ_STR_ARRAY(j, headers_v, HTTP_HEADERS_MAX_COUNT, HTTP_HEADER_MAX_LENGTH);
    });
    if (method == -1 || url == NULL)
        return LUA_ERRRUN;
    const char *res = ngenxx_net_http_request(url, params, method, headers_v, headers_c, timeout);
    lua_pushstring(L, res);
    return 1;
}

#pragma mark Store.SQLite

int ngenxx_store_sqlite_openL(lua_State *L)
{
    long sdk;
    char *file = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, sdk);
        JSON_READ_STR(j, file);
    });
    if (sdk <= 0 || file == NULL)
        return LUA_ERRRUN;
    void *db = ngenxx_store_sqlite_open((void *)sdk, file);
    lua_pushinteger(L, (long)db);
    return 1;
}

int ngenxx_store_sqlite_executeL(lua_State *L)
{
    long conn;
    char *sql = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, conn);
        JSON_READ_STR(j, sql);
    });
    if (conn <= 0 || sql == NULL)
        return LUA_ERRRUN;
    bool res = ngenxx_store_sqlite_execute((void *)conn, sql);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_sqlite_query_doL(lua_State *L)
{
    long conn;
    char *sql = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, conn);
        JSON_READ_STR(j, sql);
    });
    if (conn <= 0 || sql == NULL)
        return LUA_ERRRUN;
    void *res = ngenxx_store_sqlite_query_do((void *)conn, sql);
    lua_pushinteger(L, (long)res);
    return 1;
}

int ngenxx_store_sqlite_query_read_rowL(lua_State *L)
{
    long query_result;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, query_result); 
    });
    if (query_result <= 0)
        return LUA_ERRRUN;
    bool res = ngenxx_store_sqlite_query_read_row((void *)query_result);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_sqlite_query_read_column_textL(lua_State *L)
{
    long query_result;
    char *column = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, query_result);
        JSON_READ_STR(j, column);
    });
    if (query_result <= 0 || column == NULL)
        return LUA_ERRRUN;
    const char *res = ngenxx_store_sqlite_query_read_column_text((void *)query_result, column);
    lua_pushstring(L, res);
    return 1;
}

int ngenxx_store_sqlite_query_read_column_integerL(lua_State *L)
{
    long query_result;
    char *column = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, query_result);
        JSON_READ_STR(j, column);
    });
    if (query_result <= 0 || column == NULL)
        return LUA_ERRRUN;
    long long res = ngenxx_store_sqlite_query_read_column_integer((void *)query_result, column);
    lua_pushinteger(L, res);
    return 1;
}

int ngenxx_store_sqlite_query_read_column_floatL(lua_State *L)
{
    long query_result;
    char *column = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, query_result);
        JSON_READ_STR(j, column);
    });
    if (query_result <= 0 || column == NULL)
        return LUA_ERRRUN;
    double res = ngenxx_store_sqlite_query_read_column_float((void *)query_result, column);
    lua_pushnumber(L, res);
    return 1;
}

int ngenxx_store_sqlite_query_dropL(lua_State *L)
{
    long query_result;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, query_result);
    });
    if (query_result <= 0)
        return LUA_ERRRUN;
    ngenxx_store_sqlite_query_drop((void *)query_result);
    return 1;
}

int ngenxx_store_sqlite_closeL(lua_State *L)
{
    long conn;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, conn);
    });
    if (conn <= 0)
        return LUA_ERRRUN;
    ngenxx_store_sqlite_close((void *)conn);
    return 1;
}

#pragma mark Store.KV

int ngenxx_store_kv_openL(lua_State *L)
{
    long sdk;
    char *_id = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, sdk);
        JSON_READ_STR(j, _id);
    });
    if (sdk <= 0 || _id == NULL)
        return LUA_ERRRUN;
    void *res = ngenxx_store_kv_open((void *)sdk, _id);
    lua_pushinteger(L, (long)res);
    return 1;
}

int ngenxx_store_kv_read_stringL(lua_State *L)
{
    long conn;
    char *k = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, conn);
        JSON_READ_STR(j, k);
    });
    if (conn <= 0 || k == NULL)
        return LUA_ERRRUN;
    const char *res = ngenxx_store_kv_read_string((void *)conn, k);
    lua_pushstring(L, res);
    return 1;
}

int ngenxx_store_kv_write_stringL(lua_State *L)
{
    long conn;
    char *k = NULL;
    char *v = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, conn);
        JSON_READ_STR(j, k);
        JSON_READ_STR(j, v);
    });
    if (conn <= 0 || k == NULL)
        return LUA_ERRRUN;
    bool res = ngenxx_store_kv_write_string((void *)conn, k, v);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_kv_read_integerL(lua_State *L)
{
    long conn;
    char *k = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, conn);
        JSON_READ_STR(j, k);
    });
    if (conn <= 0 || k == NULL)
        return LUA_ERRRUN;
    long long res = ngenxx_store_kv_read_integer((void *)conn, k);
    lua_pushinteger(L, res);
    return 1;
}

int ngenxx_store_kv_write_integerL(lua_State *L)
{
    long conn;
    char *k = NULL;
    long long v;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, conn);
        JSON_READ_STR(j, k);
        JSON_READ_NUM(j, v);
    });
    if (conn <= 0 || k == NULL)
        return LUA_ERRRUN;
    bool res = ngenxx_store_kv_write_integer((void *)conn, k, v);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_kv_read_floatL(lua_State *L)
{
    long conn;
    char *k = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, conn);
        JSON_READ_STR(j, k);
    });
    if (conn <= 0 || k == NULL)
        return LUA_ERRRUN;
    double res = ngenxx_store_kv_read_float((void *)conn, k);
    lua_pushnumber(L, res);
    return 1;
}

int ngenxx_store_kv_write_floatL(lua_State *L)
{
    long conn;
    char *k = NULL;
    double v;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, conn);
        JSON_READ_STR(j, k);
        JSON_READ_NUM(j, v);
    });
    if (conn <= 0 || k == NULL)
        return LUA_ERRRUN;
    bool res = ngenxx_store_kv_write_float((void *)conn, k, v);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_kv_containsL(lua_State *L)
{
    long conn;
    char *k = NULL;
    parse_lua_func_params(L, [&](cJSON *j) -> void { 
        JSON_READ_NUM(j, conn);
        JSON_READ_STR(j, k);
    });
    if (conn <= 0 || k == NULL)
        return LUA_ERRRUN;
    bool res = ngenxx_store_kv_contains((void *)conn, k);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_kv_clearL(lua_State *L)
{
    long conn;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, conn);
    });
    if (conn <= 0)
        return LUA_ERRRUN;
    ngenxx_store_kv_clear((void *)conn);
    return 1;
}

int ngenxx_store_kv_closeL(lua_State *L)
{
    long conn;
    parse_lua_func_params(L, [&](cJSON *j) -> void {
        JSON_READ_NUM(j, conn);
    });
    if (conn <= 0)
        return LUA_ERRRUN;
    ngenxx_store_kv_close((void *)conn);
    return 1;
}

#pragma mark Lua

#ifndef __EMSCRIPTEN__
bool ngenxx_L_loadF(void *sdk, const char *file)
{
    return NGenXX::LuaBridge::loadFile((lua_State *)(((NGenXXHandle *)sdk)->lua), file) == LUA_OK;
}
#endif

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
bool ngenxx_L_loadS(void *sdk, const char *script)
{
    return NGenXX::LuaBridge::loadScript((lua_State *)(((NGenXXHandle *)sdk)->lua), script) == LUA_OK;
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
const char *ngenxx_L_call(void *sdk, const char *func, const char *params)
{
    return NGenXX::LuaBridge::callFunc((lua_State *)(((NGenXXHandle *)sdk)->lua), func, params);
}

void _ngenxx_export_funcs_for_lua(NGenXXHandle *handle)
{
    BIND_LUA_FUNC(handle, ngenxx_get_versionL);

    BIND_LUA_FUNC(handle, ngenxx_log_printL);

    BIND_LUA_FUNC(handle, ngenxx_device_typeL);
    BIND_LUA_FUNC(handle, ngenxx_device_nameL);
    BIND_LUA_FUNC(handle, ngenxx_device_os_versionL);
    BIND_LUA_FUNC(handle, ngenxx_device_cpu_archL);

    BIND_LUA_FUNC(handle, ngenxx_net_http_requestL);

    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_openL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_executeL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_query_doL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_query_read_rowL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_query_read_column_textL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_query_read_column_integerL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_query_read_column_floatL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_query_dropL);
    BIND_LUA_FUNC(handle, ngenxx_store_sqlite_closeL);

    BIND_LUA_FUNC(handle, ngenxx_store_kv_openL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_read_stringL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_write_stringL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_read_integerL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_write_integerL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_read_floatL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_write_floatL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_containsL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_clearL);
    BIND_LUA_FUNC(handle, ngenxx_store_kv_closeL);
}

void _ngenxx_lua_init(NGenXXHandle *handle)
{
    handle->lua = NGenXX::LuaBridge::create();
    _ngenxx_export_funcs_for_lua(handle);
}

void _ngenxx_lua_release(NGenXXHandle *handle)
{
    if (handle == NULL || handle->lua == NULL) return;
    NGenXX::LuaBridge::destroy((lua_State *)(handle->lua));
}