#include "NGenXX-JS.hxx"
#include "../external/quickjs/quickjs.h"
#include "util/TypeUtil.hxx"
#include "NGenXX-inner.hxx"
#include "js/JsBridge.hxx"
#include "NGenXX-S.hxx"

static NGenXX::JsBridge *_ngenxx_js;
#define BIND_JS_FUNC(f) _ngenxx_js->bindFunc(#f, f);

#define DEF_JS_FUNC_VOID(fJ, fS)                                                           \
    static JSValue fJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) \
    {                                                                                      \
        const char *json = JS_ToCString(ctx, argv[0]);                                     \
        fS(json);                                                                          \
        JS_FreeCString(ctx, json);                                                         \
        return JS_UNDEFINED;                                                               \
    }

#define DEF_JS_FUNC_STRING(fJ, fS)                                                         \
    static JSValue fJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) \
    {                                                                                      \
        const char *json = JS_ToCString(ctx, argv[0]);                                     \
        const std::string res = fS(json);                                                  \
        JS_FreeCString(ctx, json);                                                         \
        return JS_NewString(ctx, res.c_str());                                             \
    }

#define DEF_JS_FUNC_BOOL(fJ, fS)                                                           \
    static JSValue fJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) \
    {                                                                                      \
        const char *json = JS_ToCString(ctx, argv[0]);                                     \
        auto res = (bool)fS(json);                                                         \
        JS_FreeCString(ctx, json);                                                         \
        return JS_NewBool(ctx, res);                                                       \
    }

#define DEF_JS_FUNC_INT32(fJ, fS)                                                          \
    static JSValue fJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) \
    {                                                                                      \
        const char *json = JS_ToCString(ctx, argv[0]);                                     \
        auto res = (int32_t)fS(json);                                                      \
        JS_FreeCString(ctx, json);                                                         \
        return JS_NewInt32(ctx, res);                                                      \
    }

#define DEF_JS_FUNC_INT64(fJ, fS)                                                          \
    static JSValue fJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) \
    {                                                                                      \
        const char *json = JS_ToCString(ctx, argv[0]);                                     \
        auto res = (int64_t)fS(json);                                                      \
        JS_FreeCString(ctx, json);                                                         \
        return JS_NewInt64(ctx, res);                                                      \
    }

#define DEF_JS_FUNC_FLOAT(fJ, fS)                                                          \
    static JSValue fJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv) \
    {                                                                                      \
        const char *json = JS_ToCString(ctx, argv[0]);                                     \
        auto res = (double)fS(json);                                                       \
        JS_FreeCString(ctx, json);                                                         \
        return JS_NewFloat64(ctx, res);                                                    \
    }

DEF_JS_FUNC_STRING(ngenxx_get_versionJ, ngenxx_get_versionS)
DEF_JS_FUNC_INT32(ngenxx_device_typeJ, ngenxx_device_typeS)
DEF_JS_FUNC_STRING(ngenxx_device_nameJ, ngenxx_device_nameS)
DEF_JS_FUNC_STRING(ngenxx_device_manufacturerJ, ngenxx_device_manufacturerS)
DEF_JS_FUNC_STRING(ngenxx_device_os_versionJ, ngenxx_device_os_versionS)
DEF_JS_FUNC_INT32(ngenxx_device_cpu_archJ, ngenxx_device_cpu_archS)

DEF_JS_FUNC_VOID(ngenxx_log_printJ, ngenxx_log_printS)

DEF_JS_FUNC_STRING(ngenxx_net_http_requestJ, ngenxx_net_http_requestS)

DEF_JS_FUNC_INT64(ngenxx_store_sqlite_openJ, ngenxx_store_sqlite_openS)
DEF_JS_FUNC_BOOL(ngenxx_store_sqlite_executeJ, ngenxx_store_sqlite_executeS)
DEF_JS_FUNC_INT64(ngenxx_store_sqlite_query_doJ, ngenxx_store_sqlite_query_doS)
DEF_JS_FUNC_BOOL(ngenxx_store_sqlite_query_read_rowJ, ngenxx_store_sqlite_query_read_rowS)
DEF_JS_FUNC_STRING(ngenxx_store_sqlite_query_read_column_textJ, ngenxx_store_sqlite_query_read_column_textS)
DEF_JS_FUNC_INT64(ngenxx_store_sqlite_query_read_column_integerJ, ngenxx_store_sqlite_query_read_column_integerS)
DEF_JS_FUNC_FLOAT(ngenxx_store_sqlite_query_read_column_floatJ, ngenxx_store_sqlite_query_read_column_floatS)
DEF_JS_FUNC_VOID(ngenxx_store_sqlite_query_dropJ, ngenxx_store_sqlite_query_dropS)
DEF_JS_FUNC_VOID(ngenxx_store_sqlite_closeJ, ngenxx_store_sqlite_closeS)

DEF_JS_FUNC_INT64(ngenxx_store_kv_openJ, ngenxx_store_kv_openS)
DEF_JS_FUNC_STRING(ngenxx_store_kv_read_stringJ, ngenxx_store_kv_read_stringS)
DEF_JS_FUNC_BOOL(ngenxx_store_kv_write_stringJ, ngenxx_store_kv_write_stringS)
DEF_JS_FUNC_INT64(ngenxx_store_kv_read_integerJ, ngenxx_store_kv_read_integerS)
DEF_JS_FUNC_BOOL(ngenxx_store_kv_write_integerJ, ngenxx_store_kv_write_integerS)
DEF_JS_FUNC_FLOAT(ngenxx_store_kv_read_floatJ, ngenxx_store_kv_read_floatS)
DEF_JS_FUNC_BOOL(ngenxx_store_kv_write_floatJ, ngenxx_store_kv_write_floatS)
DEF_JS_FUNC_BOOL(ngenxx_store_kv_containsJ, ngenxx_store_kv_containsS)
DEF_JS_FUNC_VOID(ngenxx_store_kv_removeJ, ngenxx_store_kv_removeS)
DEF_JS_FUNC_VOID(ngenxx_store_kv_clearJ, ngenxx_store_kv_clearS)
DEF_JS_FUNC_VOID(ngenxx_store_kv_closeJ, ngenxx_store_kv_closeS)

DEF_JS_FUNC_STRING(ngenxx_coding_hex_bytes2strJ, ngenxx_coding_hex_bytes2strS)
DEF_JS_FUNC_STRING(ngenxx_coding_hex_str2bytesJ, ngenxx_coding_hex_str2bytesS)

DEF_JS_FUNC_STRING(ngenxx_crypto_randJ, ngenxx_crypto_randS)
DEF_JS_FUNC_STRING(ngenxx_crypto_aes_encryptJ, ngenxx_crypto_aes_encryptS)
DEF_JS_FUNC_STRING(ngenxx_crypto_aes_decryptJ, ngenxx_crypto_aes_decryptS)
DEF_JS_FUNC_STRING(ngenxx_crypto_aes_gcm_encryptJ, ngenxx_crypto_aes_gcm_encryptS)
DEF_JS_FUNC_STRING(ngenxx_crypto_aes_gcm_decryptJ, ngenxx_crypto_aes_gcm_decryptS)
DEF_JS_FUNC_STRING(ngenxx_crypto_hash_md5J, ngenxx_crypto_hash_md5S)
DEF_JS_FUNC_STRING(ngenxx_crypto_hash_sha256J, ngenxx_crypto_hash_sha256S)
DEF_JS_FUNC_STRING(ngenxx_crypto_base64_encodeJ, ngenxx_crypto_base64_encodeS)
DEF_JS_FUNC_STRING(ngenxx_crypto_base64_decodeJ, ngenxx_crypto_base64_decodeS)

#pragma mark JS

EXPORT_AUTO
bool ngenxx_J_loadF(const char *file)
{
    if (_ngenxx_js == NULL || file == NULL)
        return false;
    return _ngenxx_js->loadFile(std::string(file));
}

EXPORT_AUTO
bool ngenxx_J_loadS(const char *script, const char *name)
{
    if (_ngenxx_js == NULL || script == NULL || name == NULL)
        return false;
    return _ngenxx_js->loadScript(std::string(script), std::string(name));
}

EXPORT_AUTO
const char *ngenxx_J_call(const char *func, const char *params)
{
    if (_ngenxx_js == NULL || func == NULL)
        return NULL;
    return str2charp(_ngenxx_js->callFunc(std::string(func), std::string(params ? : "")));
}

#pragma mark JS Module Register

void registerJsModule()
{
    BIND_JS_FUNC(ngenxx_get_versionJ);

    BIND_JS_FUNC(ngenxx_log_printJ);

    BIND_JS_FUNC(ngenxx_device_typeJ);
    BIND_JS_FUNC(ngenxx_device_nameJ);
    BIND_JS_FUNC(ngenxx_device_manufacturerJ);
    BIND_JS_FUNC(ngenxx_device_os_versionJ);
    BIND_JS_FUNC(ngenxx_device_cpu_archJ);

    BIND_JS_FUNC(ngenxx_net_http_requestJ);

    BIND_JS_FUNC(ngenxx_store_sqlite_openJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_executeJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_query_doJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_query_read_rowJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_query_read_column_textJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_query_read_column_integerJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_query_read_column_floatJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_query_dropJ);
    BIND_JS_FUNC(ngenxx_store_sqlite_closeJ);

    BIND_JS_FUNC(ngenxx_store_kv_openJ);
    BIND_JS_FUNC(ngenxx_store_kv_read_stringJ);
    BIND_JS_FUNC(ngenxx_store_kv_write_stringJ);
    BIND_JS_FUNC(ngenxx_store_kv_read_integerJ);
    BIND_JS_FUNC(ngenxx_store_kv_write_integerJ);
    BIND_JS_FUNC(ngenxx_store_kv_read_floatJ);
    BIND_JS_FUNC(ngenxx_store_kv_write_floatJ);
    BIND_JS_FUNC(ngenxx_store_kv_containsJ);
    BIND_JS_FUNC(ngenxx_store_kv_removeJ);
    BIND_JS_FUNC(ngenxx_store_kv_clearJ);
    BIND_JS_FUNC(ngenxx_store_kv_closeJ);

    BIND_JS_FUNC(ngenxx_coding_hex_bytes2strJ);
    BIND_JS_FUNC(ngenxx_coding_hex_str2bytesJ);

    BIND_JS_FUNC(ngenxx_crypto_randJ);
    BIND_JS_FUNC(ngenxx_crypto_aes_encryptJ);
    BIND_JS_FUNC(ngenxx_crypto_aes_decryptJ);
    BIND_JS_FUNC(ngenxx_crypto_aes_gcm_encryptJ);
    BIND_JS_FUNC(ngenxx_crypto_aes_gcm_decryptJ);
    BIND_JS_FUNC(ngenxx_crypto_hash_md5J);
    BIND_JS_FUNC(ngenxx_crypto_hash_sha256J);
    BIND_JS_FUNC(ngenxx_crypto_base64_encodeJ);
    BIND_JS_FUNC(ngenxx_crypto_base64_decodeJ);
}

void _ngenxx_js_init(void)
{
    if (_ngenxx_js != NULL)
        return;
    _ngenxx_js = new NGenXX::JsBridge();
    registerJsModule();
}

void _ngenxx_js_release(void)
{
    if (_ngenxx_js == NULL)
        return;
    delete _ngenxx_js;
    _ngenxx_js = NULL;
}