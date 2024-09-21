#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

extern "C"
{
#include "../../../external/lua/lauxlib.h"
}

#include "json/JsonDecoder.hxx"
#include "util/TypeUtil.hxx"
#include "lua/LuaBridge.hxx"
#include "../include/NGenXX.h"
#include "NGenXX-inner.hxx"
#include "NGenXX-Lua.hxx"

static NGenXX::LuaBridge *_ngenxx_lua;

#define BIND_LUA_FUNC(f) _ngenxx_lua->bindFunc(std::string(#f), f);

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

int ngenxx_device_manufacturerL(lua_State *L)
{
    const char *res = ngenxx_device_manufacturer();
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
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    int level = decoder.readNumber(decoder.readNode(NULL, "level"));
    const char *content = str2charp(decoder.readString(decoder.readNode(NULL, "content")));
    if (level < 0 || content == NULL)
        return 1;
    ngenxx_log_print(level, content);
    return 1;
}

#pragma mark Net.Http

int ngenxx_net_http_requestL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(luaL_checkstring(L, 1));
    const char *url = str2charp(decoder.readString(decoder.readNode(NULL, "url")));
    const char *params = str2charp(decoder.readString(decoder.readNode(NULL, "params")));
    int method = decoder.readNumber(decoder.readNode(NULL, "method"));
    int headers_c = decoder.readNumber(decoder.readNode(NULL, "headers_c"));
    long timeout = decoder.readNumber(decoder.readNode(NULL, "timeout"));

    char **headers_v = (char **)malloc(HTTP_HEADERS_MAX_COUNT * sizeof(char *));
    void *headersNode = decoder.readNode(NULL, "headers_v");
    if (headersNode)
    {
        void *headerNode = decoder.readChild(headersNode);
        int idx = 0;
        while (headerNode && idx < HTTP_HEADERS_MAX_COUNT)
        {
            headers_v[idx] = (char *)malloc(HTTP_HEADER_MAX_LENGTH * sizeof(char) + 1);
            strcpy(headers_v[idx], decoder.readString(headerNode).c_str());
            headerNode = decoder.readNext(headerNode);
        }
    }

    if (method < 0 || url == NULL || headers_c > HTTP_HEADERS_MAX_COUNT)
        return 1;
    const char *res = ngenxx_net_http_request(url, params, method, headers_v, headers_c, timeout);
    lua_pushstring(L, res);
    return 1;
}

#pragma mark Store.SQLite

int ngenxx_store_sqlite_openL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    const char *_id = str2charp(decoder.readString(decoder.readNode(NULL, "_id")));
    if (_id == NULL)
        return 1;
    void *db = ngenxx_store_sqlite_open(_id);
    lua_pushinteger(L, (long)db);
    return 1;
}

int ngenxx_store_sqlite_executeL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *sql = str2charp(decoder.readString(decoder.readNode(NULL, "sql")));
    if (conn <= 0 || sql == NULL)
        return 1;
    bool res = ngenxx_store_sqlite_execute((void *)conn, sql);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_sqlite_query_doL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *sql = str2charp(decoder.readString(decoder.readNode(NULL, "sql")));
    if (conn <= 0 || sql == NULL)
        return 1;
    void *res = ngenxx_store_sqlite_query_do((void *)conn, sql);
    lua_pushinteger(L, (long)res);
    return 1;
}

int ngenxx_store_sqlite_query_read_rowL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long query_result = decoder.readNumber(decoder.readNode(NULL, "query_result"));
    if (query_result <= 0)
        return 1;
    bool res = ngenxx_store_sqlite_query_read_row((void *)query_result);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_sqlite_query_read_column_textL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long query_result = decoder.readNumber(decoder.readNode(NULL, "query_result"));
    const char *column = str2charp(decoder.readString(decoder.readNode(NULL, "column")));
    if (query_result <= 0 || column == NULL)
        return 1;
    const char *res = ngenxx_store_sqlite_query_read_column_text((void *)query_result, column);
    lua_pushstring(L, res);
    return 1;
}

int ngenxx_store_sqlite_query_read_column_integerL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long query_result = decoder.readNumber(decoder.readNode(NULL, "query_result"));
    const char *column = str2charp(decoder.readString(decoder.readNode(NULL, "column")));
    if (query_result <= 0 || column == NULL)
        return 1;
    long long res = ngenxx_store_sqlite_query_read_column_integer((void *)query_result, column);
    lua_pushinteger(L, res);
    return 1;
}

int ngenxx_store_sqlite_query_read_column_floatL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long query_result = decoder.readNumber(decoder.readNode(NULL, "query_result"));
    const char *column = str2charp(decoder.readString(decoder.readNode(NULL, "column")));
    if (query_result <= 0 || column == NULL)
        return 1;
    double res = ngenxx_store_sqlite_query_read_column_float((void *)query_result, column);
    lua_pushnumber(L, res);
    return 1;
}

int ngenxx_store_sqlite_query_dropL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long query_result = decoder.readNumber(decoder.readNode(NULL, "query_result"));
    if (query_result <= 0)
        return 1;
    ngenxx_store_sqlite_query_drop((void *)query_result);
    return 1;
}

int ngenxx_store_sqlite_closeL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    if (conn <= 0)
        return 1;
    ngenxx_store_sqlite_close((void *)conn);
    return 1;
}

#pragma mark Store.KV

int ngenxx_store_kv_openL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    const char *_id = str2charp(decoder.readString(decoder.readNode(NULL, "_id")));
    if (_id == NULL)
        return 1;
    void *res = ngenxx_store_kv_open(_id);
    lua_pushinteger(L, (long)res);
    return 1;
}

int ngenxx_store_kv_read_stringL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *k = str2charp(decoder.readString(decoder.readNode(NULL, "k")));
    if (conn <= 0 || k == NULL)
        return 1;
    const char *res = ngenxx_store_kv_read_string((void *)conn, k);
    lua_pushstring(L, res);
    return 1;
}

int ngenxx_store_kv_write_stringL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *k = str2charp(decoder.readString(decoder.readNode(NULL, "k")));
    const char *v = str2charp(decoder.readString(decoder.readNode(NULL, "v")));
    if (conn <= 0 || k == NULL)
        return 1;
    bool res = ngenxx_store_kv_write_string((void *)conn, k, v);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_kv_read_integerL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *k = str2charp(decoder.readString(decoder.readNode(NULL, "k")));
    if (conn <= 0 || k == NULL)
        return 1;
    long long res = ngenxx_store_kv_read_integer((void *)conn, k);
    lua_pushinteger(L, res);
    return 1;
}

int ngenxx_store_kv_write_integerL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *k = str2charp(decoder.readString(decoder.readNode(NULL, "k")));
    long long v = decoder.readNumber(decoder.readNode(NULL, "v"));
    if (conn <= 0 || k == NULL)
        return 1;
    bool res = ngenxx_store_kv_write_integer((void *)conn, k, v);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_kv_read_floatL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *k = str2charp(decoder.readString(decoder.readNode(NULL, "k")));
    if (conn <= 0 || k == NULL)
        return 1;
    double res = ngenxx_store_kv_read_float((void *)conn, k);
    lua_pushnumber(L, res);
    return 1;
}

int ngenxx_store_kv_write_floatL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *k = str2charp(decoder.readString(decoder.readNode(NULL, "k")));
    double v = decoder.readNumber(decoder.readNode(NULL, "v"));
    if (conn <= 0 || k == NULL)
        return 1;
    bool res = ngenxx_store_kv_write_float((void *)conn, k, v);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_kv_containsL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *k = str2charp(decoder.readString(decoder.readNode(NULL, "k")));
    if (conn <= 0 || k == NULL)
        return 1;
    bool res = ngenxx_store_kv_contains((void *)conn, k);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_kv_clearL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    if (conn <= 0)
        return 1;
    ngenxx_store_kv_clear((void *)conn);
    return 1;
}

int ngenxx_store_kv_closeL(lua_State *L)
{
    NGenXX::Json::Decoder decoder(std::string(luaL_checkstring(L, 1)));
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    if (conn <= 0)
        return 1;
    ngenxx_store_kv_close((void *)conn);
    return 1;
}

#pragma mark Lua

#ifndef __EMSCRIPTEN__
bool ngenxx_L_loadF(const char *file)
{
    if (_ngenxx_lua == NULL || file == NULL) return false;
    return _ngenxx_lua->loadFile(std::string(file)) == LUA_OK;
}
#endif

EXPORT
bool ngenxx_L_loadS(const char *script)
{
    if (_ngenxx_lua == NULL || script == NULL) return false;
    return _ngenxx_lua->loadScript(std::string(script)) == LUA_OK;
}

EXPORT
const char *ngenxx_L_call(const char *func, const char *params)
{
    if (_ngenxx_lua == NULL || func == NULL) return NULL;
    return str2charp(_ngenxx_lua->callFunc(std::string(func), std::string(params)));
}

void _ngenxx_export_funcs_for_lua()
{
    BIND_LUA_FUNC(ngenxx_get_versionL);

    BIND_LUA_FUNC(ngenxx_log_printL);

    BIND_LUA_FUNC(ngenxx_device_typeL);
    BIND_LUA_FUNC(ngenxx_device_nameL);
    BIND_LUA_FUNC(ngenxx_device_manufacturerL);
    BIND_LUA_FUNC(ngenxx_device_os_versionL);
    BIND_LUA_FUNC(ngenxx_device_cpu_archL);

    BIND_LUA_FUNC(ngenxx_net_http_requestL);

    BIND_LUA_FUNC(ngenxx_store_sqlite_openL);
    BIND_LUA_FUNC(ngenxx_store_sqlite_executeL);
    BIND_LUA_FUNC(ngenxx_store_sqlite_query_doL);
    BIND_LUA_FUNC(ngenxx_store_sqlite_query_read_rowL);
    BIND_LUA_FUNC(ngenxx_store_sqlite_query_read_column_textL);
    BIND_LUA_FUNC(ngenxx_store_sqlite_query_read_column_integerL);
    BIND_LUA_FUNC(ngenxx_store_sqlite_query_read_column_floatL);
    BIND_LUA_FUNC(ngenxx_store_sqlite_query_dropL);
    BIND_LUA_FUNC(ngenxx_store_sqlite_closeL);

    BIND_LUA_FUNC(ngenxx_store_kv_openL);
    BIND_LUA_FUNC(ngenxx_store_kv_read_stringL);
    BIND_LUA_FUNC(ngenxx_store_kv_write_stringL);
    BIND_LUA_FUNC(ngenxx_store_kv_read_integerL);
    BIND_LUA_FUNC(ngenxx_store_kv_write_integerL);
    BIND_LUA_FUNC(ngenxx_store_kv_read_floatL);
    BIND_LUA_FUNC(ngenxx_store_kv_write_floatL);
    BIND_LUA_FUNC(ngenxx_store_kv_containsL);
    BIND_LUA_FUNC(ngenxx_store_kv_clearL);
    BIND_LUA_FUNC(ngenxx_store_kv_closeL);
}

bool _ngenxx_lua_init(void)
{
    if (_ngenxx_lua != NULL) return true;
    _ngenxx_lua = new NGenXX::LuaBridge();
    _ngenxx_export_funcs_for_lua();
    return true;
}

void _ngenxx_lua_release(void)
{
    if (_ngenxx_lua == NULL) return;
    delete _ngenxx_lua;
    _ngenxx_lua = NULL;
}