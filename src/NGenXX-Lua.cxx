#if defined(USE_LUA)
#include "NGenXX-Lua.hxx"

#include <memory>

#include "lua/LuaBridge.hxx"
#include "util/TypeUtil.hxx"
#include "NGenXX-inner.hxx"
#include "NGenXX-Script.hxx"

std::unique_ptr<NGenXX::LuaBridge> _ngenxx_lua = nullptr;

#define BIND_LUA_FUNC(f)                                                       \
  if (_ngenxx_lua) [[likely]] {                                                \
    _ngenxx_lua->bindFunc(std::string(#f), f);                                 \
  }

DEF_LUA_FUNC_STRING(ngenxx_get_versionL, ngenxx_get_versionS)
DEF_LUA_FUNC_STRING(ngenxx_root_pathL, ngenxx_root_pathS)

DEF_LUA_FUNC_INTEGER(ngenxx_device_typeL, ngenxx_device_typeS)
DEF_LUA_FUNC_STRING(ngenxx_device_nameL, ngenxx_device_nameS)
DEF_LUA_FUNC_STRING(ngenxx_device_manufacturerL, ngenxx_device_manufacturerS)
DEF_LUA_FUNC_STRING(ngenxx_device_os_versionL, ngenxx_device_os_versionS)
DEF_LUA_FUNC_INTEGER(ngenxx_device_cpu_archL, ngenxx_device_cpu_archS)

DEF_LUA_FUNC_VOID(ngenxx_log_printL, ngenxx_log_printS)

DEF_LUA_FUNC_STRING(ngenxx_net_http_requestL, ngenxx_net_http_requestS)
DEF_LUA_FUNC_BOOL(ngenxx_net_http_downloadL, ngenxx_net_http_downloadS)

DEF_LUA_FUNC_STRING(ngenxx_store_sqlite_openL, ngenxx_store_sqlite_openS)
DEF_LUA_FUNC_BOOL(ngenxx_store_sqlite_executeL, ngenxx_store_sqlite_executeS)
DEF_LUA_FUNC_STRING(ngenxx_store_sqlite_query_doL, ngenxx_store_sqlite_query_doS)
DEF_LUA_FUNC_BOOL(ngenxx_store_sqlite_query_read_rowL, ngenxx_store_sqlite_query_read_rowS)
DEF_LUA_FUNC_STRING(ngenxx_store_sqlite_query_read_column_textL, ngenxx_store_sqlite_query_read_column_textS)
DEF_LUA_FUNC_INTEGER(ngenxx_store_sqlite_query_read_column_integerL, ngenxx_store_sqlite_query_read_column_integerS)
DEF_LUA_FUNC_FLOAT(ngenxx_store_sqlite_query_read_column_floatL, ngenxx_store_sqlite_query_read_column_floatS)
DEF_LUA_FUNC_VOID(ngenxx_store_sqlite_query_dropL, ngenxx_store_sqlite_query_dropS)
DEF_LUA_FUNC_VOID(ngenxx_store_sqlite_closeL, ngenxx_store_sqlite_closeS)

DEF_LUA_FUNC_STRING(ngenxx_store_kv_openL, ngenxx_store_kv_openS)
DEF_LUA_FUNC_STRING(ngenxx_store_kv_read_stringL, ngenxx_store_kv_read_stringS)
DEF_LUA_FUNC_BOOL(ngenxx_store_kv_write_stringL, ngenxx_store_kv_write_stringS)
DEF_LUA_FUNC_INTEGER(ngenxx_store_kv_read_integerL, ngenxx_store_kv_read_integerS)
DEF_LUA_FUNC_BOOL(ngenxx_store_kv_write_integerL, ngenxx_store_kv_write_integerS)
DEF_LUA_FUNC_FLOAT(ngenxx_store_kv_read_floatL, ngenxx_store_kv_read_floatS)
DEF_LUA_FUNC_BOOL(ngenxx_store_kv_write_floatL, ngenxx_store_kv_write_floatS)
DEF_LUA_FUNC_STRING(ngenxx_store_kv_all_keysL, ngenxx_store_kv_all_keysS)
DEF_LUA_FUNC_BOOL(ngenxx_store_kv_containsL, ngenxx_store_kv_containsS)
DEF_LUA_FUNC_BOOL(ngenxx_store_kv_removeL, ngenxx_store_kv_removeS)
DEF_LUA_FUNC_VOID(ngenxx_store_kv_clearL, ngenxx_store_kv_clearS)
DEF_LUA_FUNC_VOID(ngenxx_store_kv_closeL, ngenxx_store_kv_closeS)

DEF_LUA_FUNC_STRING(ngenxx_coding_hex_bytes2strL, ngenxx_coding_hex_bytes2strS)
DEF_LUA_FUNC_STRING(ngenxx_coding_hex_str2bytesL, ngenxx_coding_hex_str2bytesS)
DEF_LUA_FUNC_STRING(ngenxx_coding_case_upperL, ngenxx_coding_case_upperS)
DEF_LUA_FUNC_STRING(ngenxx_coding_case_lowerL, ngenxx_coding_case_lowerS)
DEF_LUA_FUNC_STRING(ngenxx_coding_bytes2strL, ngenxx_coding_bytes2strS)
DEF_LUA_FUNC_STRING(ngenxx_coding_str2bytesL, ngenxx_coding_str2bytesS)

DEF_LUA_FUNC_STRING(ngenxx_crypto_randL, ngenxx_crypto_randS)
DEF_LUA_FUNC_STRING(ngenxx_crypto_aes_encryptL, ngenxx_crypto_aes_encryptS)
DEF_LUA_FUNC_STRING(ngenxx_crypto_aes_decryptL, ngenxx_crypto_aes_decryptS)
DEF_LUA_FUNC_STRING(ngenxx_crypto_aes_gcm_encryptL, ngenxx_crypto_aes_gcm_encryptS)
DEF_LUA_FUNC_STRING(ngenxx_crypto_aes_gcm_decryptL, ngenxx_crypto_aes_gcm_decryptS)
DEF_LUA_FUNC_STRING(ngenxx_crypto_hash_md5L, ngenxx_crypto_hash_md5S)
DEF_LUA_FUNC_STRING(ngenxx_crypto_hash_sha256L, ngenxx_crypto_hash_sha256S)
DEF_LUA_FUNC_STRING(ngenxx_crypto_base64_encodeL, ngenxx_crypto_base64_encodeS)
DEF_LUA_FUNC_STRING(ngenxx_crypto_base64_decodeL, ngenxx_crypto_base64_decodeS)

DEF_LUA_FUNC_STRING(ngenxx_z_zip_initL, ngenxx_z_zip_initS)
DEF_LUA_FUNC_INTEGER(ngenxx_z_zip_inputL, ngenxx_z_zip_inputS)
DEF_LUA_FUNC_STRING(ngenxx_z_zip_process_doL, ngenxx_z_zip_process_doS)
DEF_LUA_FUNC_BOOL(ngenxx_z_zip_process_finishedL, ngenxx_z_zip_process_finishedS)
DEF_LUA_FUNC_VOID(ngenxx_z_zip_releaseL, ngenxx_z_zip_releaseS)
DEF_LUA_FUNC_STRING(ngenxx_z_unzip_initL, ngenxx_z_unzip_initS)
DEF_LUA_FUNC_INTEGER(ngenxx_z_unzip_inputL, ngenxx_z_unzip_inputS)
DEF_LUA_FUNC_STRING(ngenxx_z_unzip_process_doL, ngenxx_z_unzip_process_doS)
DEF_LUA_FUNC_BOOL(ngenxx_z_unzip_process_finishedL, ngenxx_z_unzip_process_finishedS)
DEF_LUA_FUNC_VOID(ngenxx_z_unzip_releaseL, ngenxx_z_unzip_releaseS)
DEF_LUA_FUNC_STRING(ngenxx_z_bytes_zipL, ngenxx_z_bytes_zipS)
DEF_LUA_FUNC_STRING(ngenxx_z_bytes_unzipL, ngenxx_z_bytes_unzipS)

#pragma mark Lua

bool ngenxxLuaLoadF(const std::string &f)
{
    if (!_ngenxx_lua || f.empty()) [[unlikely]]
    {
        return false;
    }
    return _ngenxx_lua->loadFile(f);
}

bool ngenxxLuaLoadS(const std::string &s)
{
    if (!_ngenxx_lua || s.empty()) [[unlikely]]
    {
        return false;
    }
    return _ngenxx_lua->loadScript(s);
}

std::string ngenxxLuaCall(const std::string &f, const std::string &ps)
{
    std::string s;
    if (!_ngenxx_lua || f.empty()) [[unlikely]]
    {
        return s;
    }
    return _ngenxx_lua->callFunc(f, ps);
}

#if !defined(__EMSCRIPTEN__)
EXPORT
bool ngenxx_lua_loadF(const char *file)
{
    return ngenxxLuaLoadF(wrapStr(file));
}
#endif

EXPORT
bool ngenxx_lua_loadS(const char *script)
{
    return ngenxxLuaLoadS(wrapStr(script));
}

EXPORT
const char *ngenxx_lua_call(const char *f, const char *ps)
{
    return copyStr(ngenxxLuaCall(wrapStr(f), wrapStr(ps)));
}

void _ngenxx_lua_registerFuncs()
{
    BIND_LUA_FUNC(ngenxx_get_versionL);
    BIND_LUA_FUNC(ngenxx_root_pathL);

    BIND_LUA_FUNC(ngenxx_log_printL);

    BIND_LUA_FUNC(ngenxx_device_typeL);
    BIND_LUA_FUNC(ngenxx_device_nameL);
    BIND_LUA_FUNC(ngenxx_device_manufacturerL);
    BIND_LUA_FUNC(ngenxx_device_os_versionL);
    BIND_LUA_FUNC(ngenxx_device_cpu_archL);

    BIND_LUA_FUNC(ngenxx_net_http_requestL);
    BIND_LUA_FUNC(ngenxx_net_http_downloadL);

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
    BIND_LUA_FUNC(ngenxx_store_kv_all_keysL);
    BIND_LUA_FUNC(ngenxx_store_kv_containsL);
    BIND_LUA_FUNC(ngenxx_store_kv_removeL);
    BIND_LUA_FUNC(ngenxx_store_kv_clearL);
    BIND_LUA_FUNC(ngenxx_store_kv_closeL);

    BIND_LUA_FUNC(ngenxx_coding_hex_bytes2strL);
    BIND_LUA_FUNC(ngenxx_coding_hex_str2bytesL);
    BIND_LUA_FUNC(ngenxx_coding_case_upperL);
    BIND_LUA_FUNC(ngenxx_coding_case_lowerL);
    BIND_LUA_FUNC(ngenxx_coding_bytes2strL);
    BIND_LUA_FUNC(ngenxx_coding_str2bytesL);

    BIND_LUA_FUNC(ngenxx_crypto_randL);
    BIND_LUA_FUNC(ngenxx_crypto_aes_encryptL);
    BIND_LUA_FUNC(ngenxx_crypto_aes_decryptL);
    BIND_LUA_FUNC(ngenxx_crypto_aes_gcm_encryptL);
    BIND_LUA_FUNC(ngenxx_crypto_aes_gcm_decryptL);
    BIND_LUA_FUNC(ngenxx_crypto_hash_md5L);
    BIND_LUA_FUNC(ngenxx_crypto_hash_sha256L);
    BIND_LUA_FUNC(ngenxx_crypto_base64_encodeL);
    BIND_LUA_FUNC(ngenxx_crypto_base64_decodeL);

    BIND_LUA_FUNC(ngenxx_z_zip_initL);
    BIND_LUA_FUNC(ngenxx_z_zip_inputL);
    BIND_LUA_FUNC(ngenxx_z_zip_process_doL);
    BIND_LUA_FUNC(ngenxx_z_zip_process_finishedL);
    BIND_LUA_FUNC(ngenxx_z_zip_releaseL);
    BIND_LUA_FUNC(ngenxx_z_unzip_initL);
    BIND_LUA_FUNC(ngenxx_z_unzip_inputL);
    BIND_LUA_FUNC(ngenxx_z_unzip_process_doL);
    BIND_LUA_FUNC(ngenxx_z_unzip_process_finishedL);
    BIND_LUA_FUNC(ngenxx_z_unzip_releaseL);
    BIND_LUA_FUNC(ngenxx_z_bytes_zipL)
    BIND_LUA_FUNC(ngenxx_z_bytes_unzipL)
}

void _ngenxx_lua_init(void)
{
    if (_ngenxx_lua) [[unlikely]]
    {
        return;
    }
    _ngenxx_lua = std::make_unique<NGenXX::LuaBridge>();
    _ngenxx_lua_registerFuncs();
}

void _ngenxx_lua_release(void)
{
    if (!_ngenxx_lua) [[unlikely]]
    {
        return;
    }
    _ngenxx_lua.reset();
}
#endif