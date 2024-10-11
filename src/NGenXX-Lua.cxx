#include "NGenXX-Lua.hxx"

extern "C"
{
#include "../../../external/lua/lauxlib.h"
}

#include "lua/LuaBridge.hxx"
#include "util/TypeUtil.hxx"
#include "NGenXX-inner.hxx"
#include "NGenXX-S.hxx"

static NGenXX::LuaBridge *_ngenxx_lua;
#define BIND_LUA_FUNC(f) _ngenxx_lua->bindFunc(std::string(#f), f);

int ngenxx_get_versionL(lua_State *L)
{
    const char *res = ngenxx_get_versionS(NULL);
    lua_pushstring(L, res ?: "");
    return 1;
}

#pragma mark Device.DeviceInfo

int ngenxx_device_typeL(lua_State *L)
{
    int res = ngenxx_device_typeS(NULL);
    lua_pushinteger(L, res);
    return 1;
}

int ngenxx_device_nameL(lua_State *L)
{
    const char *res = ngenxx_device_nameS(NULL);
    lua_pushstring(L, res ?: "");
    return 1;
}

int ngenxx_device_manufacturerL(lua_State *L)
{
    const char *res = ngenxx_device_manufacturerS(NULL);
    lua_pushstring(L, res ?: "");
    return 1;
}

int ngenxx_device_os_versionL(lua_State *L)
{
    const char *res = ngenxx_device_os_versionS(NULL);
    lua_pushstring(L, res ?: "");
    return 1;
}

int ngenxx_device_cpu_archL(lua_State *L)
{
    int res = ngenxx_device_cpu_archS(NULL);
    lua_pushinteger(L, res);
    return 1;
}

#pragma mark Log

int ngenxx_log_printL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    ngenxx_log_printS(json);
    return 1;
}

#pragma mark Net.Http

int ngenxx_net_http_requestL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    const char *res = ngenxx_net_http_requestS(json);
    lua_pushstring(L, res ?: "");
    return 1;
}

#pragma mark Store.SQLite

int ngenxx_store_sqlite_openL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    void *db = ngenxx_store_sqlite_openS(json);
    lua_pushinteger(L, (long)db);
    return 1;
}

int ngenxx_store_sqlite_executeL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    bool res = ngenxx_store_sqlite_executeS(json);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_sqlite_query_doL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    void *res = ngenxx_store_sqlite_query_doS(json);
    lua_pushinteger(L, (long)res);
    return 1;
}

int ngenxx_store_sqlite_query_read_rowL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    bool res = ngenxx_store_sqlite_query_read_rowS(json);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_sqlite_query_read_column_textL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    const char *res = ngenxx_store_sqlite_query_read_column_textS(json);
    lua_pushstring(L, res ?: "");
    return 1;
}

int ngenxx_store_sqlite_query_read_column_integerL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    long long res = ngenxx_store_sqlite_query_read_column_integerS(json);
    lua_pushinteger(L, res);
    return 1;
}

int ngenxx_store_sqlite_query_read_column_floatL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    double res = ngenxx_store_sqlite_query_read_column_floatS(json);
    lua_pushnumber(L, res);
    return 1;
}

int ngenxx_store_sqlite_query_dropL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    ngenxx_store_sqlite_query_dropS(json);
    return 1;
}

int ngenxx_store_sqlite_closeL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    ngenxx_store_sqlite_closeS(json);
    return 1;
}

#pragma mark Store.KV

int ngenxx_store_kv_openL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    void *res = ngenxx_store_kv_openS(json);
    lua_pushinteger(L, (long)res);
    return 1;
}

int ngenxx_store_kv_read_stringL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    const char *res = ngenxx_store_kv_read_stringS(json);
    lua_pushstring(L, res ?: "");
    return 1;
}

int ngenxx_store_kv_write_stringL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    bool res = ngenxx_store_kv_write_stringS(json);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_kv_read_integerL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    long long res = ngenxx_store_kv_read_integerS(json);
    lua_pushinteger(L, res);
    return 1;
}

int ngenxx_store_kv_write_integerL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    bool res = ngenxx_store_kv_write_integerS(json);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_kv_read_floatL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    double res = ngenxx_store_kv_read_floatS(json);
    lua_pushnumber(L, res);
    return 1;
}

int ngenxx_store_kv_write_floatL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    bool res = ngenxx_store_kv_write_floatS(json);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_kv_containsL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    bool res = ngenxx_store_kv_containsS(json);
    lua_pushboolean(L, res);
    return 1;
}

int ngenxx_store_kv_clearL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    ngenxx_store_kv_clearS(json);
    return 1;
}

int ngenxx_store_kv_closeL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    ngenxx_store_kv_closeS(json);
    return 1;
}

#pragma mark Coding

int ngenxx_coding_hex_bytes2strL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    const char *res = ngenxx_coding_hex_bytes2strS(json);
    lua_pushstring(L, res);
    return 1;
}

int ngenxx_coding_hex_str2bytesL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    const char *res = ngenxx_coding_hex_str2bytesS(json);
    lua_pushstring(L, res);
    return 1;
}

#pragma mark Crypto

int ngenxx_crypto_base64_encodeL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    const char *res = ngenxx_crypto_base64_encodeS(json);
    lua_pushstring(L, res);
    return 1;
}

int ngenxx_crypto_base64_decodeL(lua_State *L)
{
    const char *json = luaL_checkstring(L, 1);
    const char *res = ngenxx_crypto_base64_decodeS(json);
    lua_pushstring(L, res);
    return 1;
}

#pragma mark Lua

#ifndef __EMSCRIPTEN__
EXPORT
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

    BIND_LUA_FUNC(ngenxx_coding_hex_bytes2strL);
    BIND_LUA_FUNC(ngenxx_coding_hex_str2bytesL);

    BIND_LUA_FUNC(ngenxx_crypto_base64_encodeL);
    BIND_LUA_FUNC(ngenxx_crypto_base64_decodeL);
}

void _ngenxx_lua_init(void)
{
    if (_ngenxx_lua != NULL) return;
    _ngenxx_lua = new NGenXX::LuaBridge();
    _ngenxx_export_funcs_for_lua();
}

void _ngenxx_lua_release(void)
{
    if (_ngenxx_lua == NULL) return;
    delete _ngenxx_lua;
    _ngenxx_lua = NULL;
}