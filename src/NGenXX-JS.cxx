#include "NGenXX-JS.hxx"
#include "../external/quickjs/quickjs.h"
#include "util/TypeUtil.hxx"
#include "NGenXX-inner.hxx"
#include "js/JsBridge.hxx"
#include "NGenXX-S.hxx"

static NGenXX::JsBridge *_ngenxx_js;
#define BIND_JS_FUNC(f) _ngenxx_js->bindFunc(#f, f);

static JSValue ngenxx_get_versionJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *res = ngenxx_get_versionS(NULL);
    return JS_NewString(ctx, res ?: "");
}

#pragma mark Device.DeviceInfo

static JSValue ngenxx_device_typeJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    int res = ngenxx_device_typeS(NULL);
    return JS_NewInt32(ctx, res);
}

static JSValue ngenxx_device_nameJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *res = ngenxx_device_nameS(NULL);
    return JS_NewString(ctx, res ?: "");
}

static JSValue ngenxx_device_manufacturerJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *res = ngenxx_device_manufacturerS(NULL);
    return JS_NewString(ctx, res ?: "");
}

static JSValue ngenxx_device_os_versionJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *res = ngenxx_device_os_versionS(NULL);
    return JS_NewString(ctx, res ?: "");
}

static JSValue ngenxx_device_cpu_archJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    int res = ngenxx_device_cpu_archS(NULL);
    return JS_NewInt32(ctx, res);
}

#pragma mark Log

static JSValue ngenxx_log_printJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    ngenxx_log_printS(json);
    return JS_UNDEFINED;
}

#pragma mark Net.Http

static JSValue ngenxx_net_http_requestJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    const char *res = ngenxx_net_http_requestS(json);
    JS_FreeCString(ctx, json);
    return JS_NewString(ctx, res ?: "");
}

#pragma mark Store.SQLite

static JSValue ngenxx_store_sqlite_openJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    void *res = ngenxx_store_sqlite_openS(json);
    JS_FreeCString(ctx, json);
    return JS_NewInt64(ctx, (int64_t)res);
}

static JSValue ngenxx_store_sqlite_executeJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    bool res = ngenxx_store_sqlite_executeS(json);
    JS_FreeCString(ctx, json);
    return JS_NewBool(ctx, res);
}

static JSValue ngenxx_store_sqlite_query_doJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    void *res = ngenxx_store_sqlite_query_doS(json);
    JS_FreeCString(ctx, json);
    return JS_NewInt64(ctx, (int64_t)res);
}

static JSValue ngenxx_store_sqlite_query_read_rowJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    bool res = ngenxx_store_sqlite_query_read_rowS(json);
    JS_FreeCString(ctx, json);
    return JS_NewBool(ctx, res);
}

static JSValue ngenxx_store_sqlite_query_read_column_textJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    const char *res = ngenxx_store_sqlite_query_read_column_textS(json);
    JS_FreeCString(ctx, json);
    return JS_NewString(ctx, res ?: "");
}

static JSValue ngenxx_store_sqlite_query_read_column_integerJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    long long res = ngenxx_store_sqlite_query_read_column_integerS(json);
    JS_FreeCString(ctx, json);
    return JS_NewInt64(ctx, (int64_t)res);
}

static JSValue ngenxx_store_sqlite_query_read_column_floatJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    double res = ngenxx_store_sqlite_query_read_column_floatS(json);
    JS_FreeCString(ctx, json);
    return JS_NewFloat64(ctx, res);
}

static JSValue ngenxx_store_sqlite_query_dropJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    ngenxx_store_sqlite_query_dropS(json);
    JS_FreeCString(ctx, json);
    return JS_UNDEFINED;
}

static JSValue ngenxx_store_sqlite_closeJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    ngenxx_store_sqlite_closeS(json);
    JS_FreeCString(ctx, json);
    return JS_UNDEFINED;
}

#pragma mark Store.KV

static JSValue ngenxx_store_kv_openJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    void *res = ngenxx_store_kv_openS(json);
    JS_FreeCString(ctx, json);
    return JS_NewInt64(ctx, (int64_t)res);
}

static JSValue ngenxx_store_kv_read_stringJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    const char *res = ngenxx_store_kv_read_stringS(json);
    JS_FreeCString(ctx, json);
    return JS_NewString(ctx, res ?: "");
}

static JSValue ngenxx_store_kv_write_stringJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    bool res = ngenxx_store_kv_write_stringS(json);
    JS_FreeCString(ctx, json);
    return JS_NewBool(ctx, res);
}

static JSValue ngenxx_store_kv_read_integerJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    long long res = ngenxx_store_kv_read_integerS(json);
    JS_FreeCString(ctx, json);
    return JS_NewInt64(ctx, (int64_t)res);
}

static JSValue ngenxx_store_kv_write_integerJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    bool res = ngenxx_store_kv_write_integerS(json);
    JS_FreeCString(ctx, json);
    return JS_NewBool(ctx, res);
}

static JSValue ngenxx_store_kv_read_floatJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    double res = ngenxx_store_kv_read_floatS(json);
    JS_FreeCString(ctx, json);
    return JS_NewFloat64(ctx, res);
}

static JSValue ngenxx_store_kv_write_floatJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    bool res = ngenxx_store_kv_write_floatS(json);
    JS_FreeCString(ctx, json);
    return JS_NewBool(ctx, res);
}

static JSValue ngenxx_store_kv_containsJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    bool res = ngenxx_store_kv_containsS(json);
    JS_FreeCString(ctx, json);
    return JS_NewBool(ctx, res);
}

static JSValue ngenxx_store_kv_clearJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    ngenxx_store_kv_clearS(json);
    JS_FreeCString(ctx, json);
    return JS_UNDEFINED;
}

static JSValue ngenxx_store_kv_closeJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    ngenxx_store_kv_closeS(json);
    JS_FreeCString(ctx, json);
    return JS_UNDEFINED;
}

#pragma mark Coding

static JSValue ngenxx_coding_hex_bytes2strJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    const char *res = ngenxx_coding_hex_bytes2strS(json);
    JS_FreeCString(ctx, json);
    return JS_NewString(ctx, res ?: "");
}

static JSValue ngenxx_coding_hex_str2bytesJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    const char *res = ngenxx_coding_hex_str2bytesS(json);
    JS_FreeCString(ctx, json);
    return JS_NewString(ctx, res ?: "");
}

#pragma mark Crypto

static JSValue ngenxx_crypto_base64_encodeJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    const char *res = ngenxx_crypto_base64_encodeS(json);
    JS_FreeCString(ctx, json);
    return JS_NewString(ctx, res ?: "");
}

static JSValue ngenxx_crypto_base64_decodeJ(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char *json = JS_ToCString(ctx, argv[0]);
    const char *res = ngenxx_crypto_base64_decodeS(json);
    JS_FreeCString(ctx, json);
    return JS_NewString(ctx, res ?: "");
}

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
    return str2charp(_ngenxx_js->callFunc(std::string(func), std::string(params)));
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
    BIND_JS_FUNC(ngenxx_store_kv_clearJ);
    BIND_JS_FUNC(ngenxx_store_kv_closeJ);

    BIND_JS_FUNC(ngenxx_coding_hex_bytes2strJ);
    BIND_JS_FUNC(ngenxx_coding_hex_str2bytesJ);

    BIND_JS_FUNC(ngenxx_crypto_base64_encodeJ);
    BIND_JS_FUNC(ngenxx_crypto_base64_decodeJ);
}

void _ngenxx_js_init(void)
{
    if (_ngenxx_js != NULL) return;
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