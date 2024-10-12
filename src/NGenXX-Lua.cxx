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

#define DEF_LUA_FUNC_VOID(fL, fS)                  \
    int fL(lua_State *L)                           \
    {                                              \
        const char *json = luaL_checkstring(L, 1); \
        fS(json);                                  \
        return 1;                                  \
    }

#define DEF_LUA_FUNC_STRING(fL, fS)                \
    int fL(lua_State *L)                           \
    {                                              \
        const char *json = luaL_checkstring(L, 1); \
        const char *res = fS(json);                \
        lua_pushstring(L, res ?: "");              \
        return 1;                                  \
    }

#define DEF_LUA_FUNC_INTEGER(fL, fS)               \
    int fL(lua_State *L)                           \
    {                                              \
        const char *json = luaL_checkstring(L, 1); \
        auto res = (long long)fS(json);            \
        lua_pushinteger(L, res);                   \
        return 1;                                  \
    }

#define DEF_LUA_FUNC_BOOL(fL, fS)                  \
    int fL(lua_State *L)                           \
    {                                              \
        const char *json = luaL_checkstring(L, 1); \
        bool res = fS(json);                       \
        lua_pushboolean(L, res);                   \
        return 1;                                  \
    }

#define DEF_LUA_FUNC_FLOAT(fL, fS)                 \
    int fL(lua_State *L)                           \
    {                                              \
        const char *json = luaL_checkstring(L, 1); \
        double res = fS(json);                     \
        lua_pushnumber(L, res);                    \
        return 1;                                  \
    }

DEF_LUA_FUNC_STRING(ngenxx_get_versionL, ngenxx_get_versionS)

DEF_LUA_FUNC_INTEGER(ngenxx_device_typeL, ngenxx_device_typeS)
DEF_LUA_FUNC_STRING(ngenxx_device_nameL, ngenxx_device_nameS)
DEF_LUA_FUNC_STRING(ngenxx_device_manufacturerL, ngenxx_device_manufacturerS)
DEF_LUA_FUNC_STRING(ngenxx_device_os_versionL, ngenxx_device_os_versionS)
DEF_LUA_FUNC_INTEGER(ngenxx_device_cpu_archL, ngenxx_device_cpu_archS)

DEF_LUA_FUNC_VOID(ngenxx_log_printL, ngenxx_log_printS)

DEF_LUA_FUNC_STRING(ngenxx_net_http_requestL, ngenxx_net_http_requestS)

DEF_LUA_FUNC_INTEGER(ngenxx_store_sqlite_openL, ngenxx_store_sqlite_openS)
DEF_LUA_FUNC_BOOL(ngenxx_store_sqlite_executeL, ngenxx_store_sqlite_executeS)
DEF_LUA_FUNC_INTEGER(ngenxx_store_sqlite_query_doL, ngenxx_store_sqlite_query_doS)
DEF_LUA_FUNC_BOOL(ngenxx_store_sqlite_query_read_rowL, ngenxx_store_sqlite_query_read_rowS)
DEF_LUA_FUNC_STRING(ngenxx_store_sqlite_query_read_column_textL, ngenxx_store_sqlite_query_read_column_textS)
DEF_LUA_FUNC_INTEGER(ngenxx_store_sqlite_query_read_column_integerL, ngenxx_store_sqlite_query_read_column_integerS)
DEF_LUA_FUNC_FLOAT(ngenxx_store_sqlite_query_read_column_floatL, ngenxx_store_sqlite_query_read_column_floatS)
DEF_LUA_FUNC_VOID(ngenxx_store_sqlite_query_dropL, ngenxx_store_sqlite_query_dropS)
DEF_LUA_FUNC_VOID(ngenxx_store_sqlite_closeL, ngenxx_store_sqlite_closeS)

DEF_LUA_FUNC_INTEGER(ngenxx_store_kv_openL, ngenxx_store_kv_openS)
DEF_LUA_FUNC_STRING(ngenxx_store_kv_read_stringL, ngenxx_store_kv_read_stringS)
DEF_LUA_FUNC_BOOL(ngenxx_store_kv_write_stringL, ngenxx_store_kv_write_stringS)
DEF_LUA_FUNC_INTEGER(ngenxx_store_kv_read_integerL, ngenxx_store_kv_read_integerS)
DEF_LUA_FUNC_BOOL(ngenxx_store_kv_write_integerL, ngenxx_store_kv_write_integerS)
DEF_LUA_FUNC_FLOAT(ngenxx_store_kv_read_floatL, ngenxx_store_kv_read_floatS)
DEF_LUA_FUNC_BOOL(ngenxx_store_kv_write_floatL, ngenxx_store_kv_write_floatS)
DEF_LUA_FUNC_BOOL(ngenxx_store_kv_containsL, ngenxx_store_kv_containsS)
DEF_LUA_FUNC_VOID(ngenxx_store_kv_clearL, ngenxx_store_kv_clearS)
DEF_LUA_FUNC_VOID(ngenxx_store_kv_closeL, ngenxx_store_kv_closeS)

DEF_LUA_FUNC_STRING(ngenxx_coding_hex_bytes2strL, ngenxx_coding_hex_bytes2strS)
DEF_LUA_FUNC_STRING(ngenxx_coding_hex_str2bytesL, ngenxx_coding_hex_str2bytesS)

DEF_LUA_FUNC_STRING(ngenxx_crypto_randL, ngenxx_crypto_randS)
DEF_LUA_FUNC_STRING(ngenxx_crypto_aes_encryptL, ngenxx_crypto_aes_encryptS)
DEF_LUA_FUNC_STRING(ngenxx_crypto_aes_decryptL, ngenxx_crypto_aes_decryptS)
DEF_LUA_FUNC_STRING(ngenxx_crypto_hash_md5L, ngenxx_crypto_hash_md5S)
DEF_LUA_FUNC_STRING(ngenxx_crypto_hash_sha256L, ngenxx_crypto_hash_sha256S)
DEF_LUA_FUNC_STRING(ngenxx_crypto_base64_encodeL, ngenxx_crypto_base64_encodeS)
DEF_LUA_FUNC_STRING(ngenxx_crypto_base64_decodeL, ngenxx_crypto_base64_decodeS)

#pragma mark Lua

#ifndef __EMSCRIPTEN__
EXPORT
bool ngenxx_L_loadF(const char *file)
{
    if (_ngenxx_lua == NULL || file == NULL)
        return false;
    return _ngenxx_lua->loadFile(std::string(file)) == LUA_OK;
}
#endif

EXPORT
bool ngenxx_L_loadS(const char *script)
{
    if (_ngenxx_lua == NULL || script == NULL)
        return false;
    return _ngenxx_lua->loadScript(std::string(script)) == LUA_OK;
}

EXPORT
const char *ngenxx_L_call(const char *func, const char *params)
{
    if (_ngenxx_lua == NULL || func == NULL)
        return NULL;
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

    BIND_LUA_FUNC(ngenxx_crypto_randL);
    BIND_LUA_FUNC(ngenxx_crypto_aes_encryptL);
    BIND_LUA_FUNC(ngenxx_crypto_aes_decryptL);
    BIND_LUA_FUNC(ngenxx_crypto_hash_md5L);
    BIND_LUA_FUNC(ngenxx_crypto_hash_sha256L);
    BIND_LUA_FUNC(ngenxx_crypto_base64_encodeL);
    BIND_LUA_FUNC(ngenxx_crypto_base64_decodeL);
}

void _ngenxx_lua_init(void)
{
    if (_ngenxx_lua != NULL)
        return;
    _ngenxx_lua = new NGenXX::LuaBridge();
    _ngenxx_export_funcs_for_lua();
}

void _ngenxx_lua_release(void)
{
    if (_ngenxx_lua == NULL)
        return;
    delete _ngenxx_lua;
    _ngenxx_lua = NULL;
}