#include "napi/native_api.h"
#include "napi_util.h"
#include "../../../../../../build.HarmonyOS/output/include/NGenXX.h"
#include <string.h>
#include <stdlib.h>

static napi_value GetVersion(napi_env env, napi_callback_info info) {
    const char *c = ngenxx_get_version();
    napi_value v = chars2NapiValue(env, c);
    free((void *)c);
    return v;
}

static napi_value Init(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    const char *root = napiValue2chars(env, argv[0]);

    bool b = ngenxx_init(root);
    napi_value v = bool2NapiValue(env, b);

    free((void *)root);
    free((void *)argv);
    return v;
}

static napi_value Release(napi_env env, napi_callback_info info) {
    ngenxx_release();

    return int2NapiValue(env, napi_ok);
}

#pragma mark Log Callback

typedef struct {
    napi_async_work tsWork;
    napi_threadsafe_function tsWorkFunc;
    int logLevel;
    const char *logContent;
} TSLogWorkData;

static napi_env sNapiEnv;
static napi_ref sTsLogCallbackRef;

static void OnLogWorkCallTS(napi_env env, napi_value ts_callback, void *context, void *data) {
    if (env == NULL || ts_callback == NULL || data == NULL)
        return;

    TSLogWorkData *tSLogWorkData = (TSLogWorkData *)data;

    size_t argc = 2;
    napi_value argv[2];
    argv[0] = int2NapiValue(env, tSLogWorkData->logLevel);
    argv[1] = chars2NapiValue(env, tSLogWorkData->logContent);

    napi_value vGlobal;
    napi_status status = napi_get_global(env, &vGlobal);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_get_global() failed");

    napi_get_reference_value(sNapiEnv, sTsLogCallbackRef, &ts_callback);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_get_reference_value() failed");

    status = napi_call_function(env, vGlobal, ts_callback, argc, argv, NULL);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_call_function() failed");

    free((void *)tSLogWorkData->logContent);
    free((void *)tSLogWorkData);
}

static void OnLogWorkExecute(napi_env env, void *data) {
    TSLogWorkData *tSLogWorkData = (TSLogWorkData *)data;

    napi_status status = napi_acquire_threadsafe_function(tSLogWorkData->tsWorkFunc);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_acquire_threadsafe_function() failed");

    status = napi_call_threadsafe_function(tSLogWorkData->tsWorkFunc, tSLogWorkData, napi_tsfn_blocking);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_call_threadsafe_function() failed");
}

static void OnLogWorkComplete(napi_env env, napi_status status, void *data) {
    TSLogWorkData *tSLogWorkData = (TSLogWorkData *)data;

    status = napi_release_threadsafe_function(tSLogWorkData->tsWorkFunc, napi_tsfn_release);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_release_threadsafe_function() failed");

    status = napi_delete_async_work(env, tSLogWorkData->tsWork);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_delete_async_work() failed");
}

static void engineLogCallback(int level, const char *content) {
    if (sNapiEnv == NULL || content == NULL)
        return;

    TSLogWorkData *tSLogWorkData = (TSLogWorkData *)malloc(sizeof(TSLogWorkData));
    tSLogWorkData->tsWork = NULL;
    tSLogWorkData->tsWorkFunc = NULL;
    tSLogWorkData->logLevel = level;
    tSLogWorkData->logContent = (char *)malloc(strlen(content) + 1);
    strcpy((char *)tSLogWorkData->logContent, content);
    free((void *)content);

    napi_value vWorkName = chars2NapiValue(sNapiEnv, "NAPI_LOG_CALLBACK_WORK");

    napi_value vTsCallback;
    napi_status status = napi_get_reference_value(sNapiEnv, sTsLogCallbackRef, &vTsCallback);
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_get_reference_value() failed");

    status = napi_create_threadsafe_function(sNapiEnv, vTsCallback, NULL, vWorkName, 0, 1, NULL, NULL, NULL,
                                             OnLogWorkCallTS, &(tSLogWorkData->tsWorkFunc));
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_create_threadsafe_function() failed");

    status = napi_create_async_work(sNapiEnv, NULL, vWorkName, OnLogWorkExecute, OnLogWorkComplete, tSLogWorkData,
                                    &(tSLogWorkData->tsWork));
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_create_async_work() failed");

    status = napi_queue_async_work(sNapiEnv, tSLogWorkData->tsWork);
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_queue_async_work() failed");
}

#pragma mark Log API

static napi_value LogSetLevel(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    int level = napiValue2int(env, argv[0]);

    ngenxx_log_set_level(level);

    free((void *)argv);
    return int2NapiValue(env, napi_ok);
}

static napi_value LogSetCallback(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    napi_value vLogCallback = argv[0];
    int status;
    if (vLogCallback == NULL) {
        sNapiEnv = NULL;

        status = napi_delete_reference(env, sTsLogCallbackRef);
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_delete_reference() failed");

        ngenxx_log_set_callback(NULL);
    } else {
        sNapiEnv = env;

        status = napi_create_reference(env, vLogCallback, 1, &sTsLogCallbackRef);
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_reference() failed");

        ngenxx_log_set_callback(engineLogCallback);
    }

    free((void *)argv);
    return int2NapiValue(env, napi_ok);
}

static napi_value LogPrint(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    int level = napiValue2int(env, argv[0]);
    const char *content = napiValue2chars(env, argv[1]);

    ngenxx_log_print(level, content);
    free((void *)content);

    free((void *)argv);
    return int2NapiValue(env, napi_ok);
}

#pragma mark Net

static napi_value NetHttpRequest(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 5);

    const char *cUrl = napiValue2chars(env, argv[0]);
    const char *cParams = napiValue2chars(env, argv[1]);
    int iMethod = napiValue2int(env, argv[2]);

    uint32_t headers_c = napiValueLen(env, argv[3]);
    const char **headers_v = napiValue2charsArray(env, argv[3], headers_c);

    long lTimeout = napiValue2long(env, argv[4]);

    const char *res = ngenxx_net_http_request(cUrl, cParams, iMethod, headers_v, headers_c, lTimeout);
    napi_value nv = chars2NapiValue(env, res);

    free((void *)res);
    for (int i = 0; i < headers_c; i++) {
        free((void *)headers_v[i]);
    }
    free((void *)headers_v);
    free((void *)cParams);
    free((void *)cUrl);
    free((void *)argv);
    return nv;
}

#pragma mark Store.SQLite

static napi_value StoreSQLiteOpen(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    const char *_id = napiValue2chars(env, argv[0]);
    long res = (long)ngenxx_store_sqlite_open(_id);
    napi_value nv = long2NapiValue(env, res);

    free((void *)_id);
    free((void *)argv);
    return nv;
}

static napi_value StoreSQLiteExecute(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long conn = napiValue2long(env, argv[0]);
    const char *sql = napiValue2chars(env, argv[1]);

    bool res = ngenxx_store_sqlite_execute((void *)conn, sql);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)sql);
    free((void *)argv);
    return nv;
}

static napi_value StoreSQLiteQueryDo(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long conn = napiValue2long(env, argv[0]);
    const char *sql = napiValue2chars(env, argv[1]);

    long res = (long)ngenxx_store_sqlite_query_do((void *)conn, sql);
    napi_value nv = long2NapiValue(env, res);

    free((void *)sql);
    free((void *)argv);
    return nv;
}

static napi_value StoreSQLiteQueryReadRow(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    long query_result = napiValue2long(env, argv[0]);

    bool res = ngenxx_store_sqlite_query_read_row((void *)query_result);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)argv);
    return nv;
}

static napi_value StoreSQLiteQueryReadColumnText(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long query_result = napiValue2long(env, argv[0]);
    const char *column = napiValue2chars(env, argv[1]);

    const char *res = ngenxx_store_sqlite_query_read_column_text((void *)query_result, column);
    napi_value nv = chars2NapiValue(env, res);

    free((void *)res);
    free((void *)column);
    free((void *)argv);
    return nv;
}

static napi_value StoreSQLiteQueryReadColumnInteger(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long query_result = napiValue2long(env, argv[0]);
    const char *column = napiValue2chars(env, argv[1]);

    long res = ngenxx_store_sqlite_query_read_column_integer((void *)query_result, column);
    napi_value nv = long2NapiValue(env, res);

    free((void *)column);
    free((void *)argv);
    return nv;
}

static napi_value StoreSQLiteQueryReadColumnFloat(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long query_result = napiValue2long(env, argv[0]);
    const char *column = napiValue2chars(env, argv[1]);

    double res = ngenxx_store_sqlite_query_read_column_float((void *)query_result, column);
    napi_value nv = double2NapiValue(env, res);

    free((void *)column);
    free((void *)argv);
    return nv;
}

static napi_value StoreSQLiteQueryDrop(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    long query_result = napiValue2long(env, argv[0]);
    ngenxx_store_sqlite_query_drop((void *)query_result);

    free((void *)argv);
    return int2NapiValue(env, napi_ok);
}

static napi_value StoreSQLiteClose(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    long conn = napiValue2long(env, argv[0]);
    ngenxx_store_sqlite_close((void *)conn);

    free((void *)argv);
    return int2NapiValue(env, napi_ok);
}

#pragma mark Store.KV

static napi_value StoreKVOpen(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    const char *_id = napiValue2chars(env, argv[0]);
    long res = (long)ngenxx_store_kv_open(_id);
    napi_value nv = long2NapiValue(env, res);

    free((void *)_id);
    free((void *)argv);
    return nv;
}

static napi_value StoreKVReadString(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);

    const char *res = ngenxx_store_kv_read_string((void *)conn, k);
    napi_value nv = chars2NapiValue(env, res);

    free((void *)res);
    free((void *)k);
    free((void *)argv);
    return nv;
}

static napi_value StoreKVWriteString(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 3);

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);
    const char *v = napiValue2chars(env, argv[2]);

    bool res = ngenxx_store_kv_write_string((void *)conn, k, v);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)v);
    free((void *)k);
    free((void *)argv);
    return nv;
}

static napi_value StoreKVReadInteger(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);

    long res = ngenxx_store_kv_read_integer((void *)conn, k);
    napi_value nv = long2NapiValue(env, res);

    free((void *)k);
    free((void *)argv);
    return nv;
}

static napi_value StoreKVWriteInteger(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 3);

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);
    long v = napiValue2long(env, argv[2]);

    bool res = ngenxx_store_kv_write_integer((void *)conn, k, v);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)k);
    free((void *)argv);
    return nv;
}

static napi_value StoreKVReadFloat(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);

    double res = ngenxx_store_kv_read_integer((void *)conn, k);
    napi_value nv = double2NapiValue(env, res);

    free((void *)k);
    free((void *)argv);
    return nv;
}

static napi_value StoreKVWriteFloat(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 3);

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);
    double v = napiValue2double(env, argv[2]);

    bool res = ngenxx_store_kv_write_integer((void *)conn, k, v);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)k);
    free((void *)argv);
    return nv;
}

static napi_value StoreKVContains(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);

    bool res = ngenxx_store_kv_contains((void *)conn, k);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)k);
    free((void *)argv);
    return nv;
}

static napi_value StoreKVClear(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    long conn = napiValue2long(env, argv[0]);
    ngenxx_store_kv_clear((void *)conn);

    free((void *)argv);
    return int2NapiValue(env, napi_ok);
}

static napi_value StoreKVClose(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    long conn = napiValue2long(env, argv[0]);
    ngenxx_store_kv_close((void *)conn);

    free((void *)argv);
    return int2NapiValue(env, napi_ok);
}

#pragma mark DeviceInfo

static napi_value DeviceType(napi_env env, napi_callback_info info) {
    int dt = ngenxx_device_type();
    napi_value v = int2NapiValue(env, dt);
    return v;
}

static napi_value DeviceName(napi_env env, napi_callback_info info) {
    const char *cDN = ngenxx_device_name();
    napi_value v = chars2NapiValue(env, cDN);
    free((void *)cDN);
    return v;
}

static napi_value DeviceManufacturer(napi_env env, napi_callback_info info) {
    const char *cDM = ngenxx_device_manufacturer();
    napi_value v = chars2NapiValue(env, cDM);
    free((void *)cDM);
    return v;
}

static napi_value DeviceOsVersion(napi_env env, napi_callback_info info) {
    const char *cOV = ngenxx_device_name();
    napi_value v = chars2NapiValue(env, cOV);
    free((void *)cOV);
    return v;
}

static napi_value DeviceCpuArch(napi_env env, napi_callback_info info) {
    int dca = ngenxx_device_cpu_arch();
    napi_value v = int2NapiValue(env, dca);
    return v;
}

#pragma mark JsonDecoder

static napi_value JsonDecoderInit(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    const char *cJson = napiValue2chars(env, argv[0]);
    
    long res = (long)ngenxx_json_decoder_init(cJson);
    napi_value v = long2NapiValue(env, res);

    free((void *)cJson);
    free((void *)argv);
    return v;
}

static napi_value JsonDecoderIsArray(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    bool res = ngenxx_json_decoder_is_array((void *)decoder, (void *)node);
    napi_value v = bool2NapiValue(env, res);
    
    free((void *)argv);
    return v;
}

static napi_value JsonDecoderIsObject(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    bool res = ngenxx_json_decoder_is_object((void *)decoder, (void *)node);
    napi_value v = bool2NapiValue(env, res);
    
    free((void *)argv);
    return v;
}

static napi_value JsonDecoderReadNode(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 3);

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    const char *cK = napiValue2chars(env, argv[2]);
    
    long res = (long)ngenxx_json_decoder_read_node((void *)decoder, (void *)node, cK);
    napi_value v = long2NapiValue(env, res);

    free((void *)cK);
    free((void *)argv);
    return v;
}

static napi_value JsonDecoderReadChild(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    long res = (long)ngenxx_json_decoder_read_child((void *)decoder, (void *)node);
    napi_value v = long2NapiValue(env, res);
    
    free((void *)argv);
    return v;
}

static napi_value JsonDecoderReadNext(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    long res = (long)ngenxx_json_decoder_read_next((void *)decoder, (void *)node);
    napi_value v = long2NapiValue(env, res);
    
    free((void *)argv);
    return v;
}

static napi_value JsonDecoderReadString(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    const char *cRes = ngenxx_json_decoder_read_string((void *)decoder, (void *)node);
    napi_value v = chars2NapiValue(env, cRes);
    
    free((void *)cRes);
    free((void *)argv);
    return v;
}

static napi_value JsonDecoderReadNumber(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    double res = ngenxx_json_decoder_read_number((void *)decoder, (void *)node);
    napi_value v = double2NapiValue(env, res);
    
    free((void *)argv);
    return v;
}

static napi_value JsonDecoderRelease(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    long decoder = napiValue2long(env, argv[0]);
    
    ngenxx_json_decoder_release((void *)decoder);
    
    free((void *)argv);
    return int2NapiValue(env, napi_ok);
}

#pragma mark Crypto

static napi_value CryptoAesEncrypt(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    uint32_t inLen = napiValueLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    uint32_t keyLen = napiValueLen(env, argv[1]);
    const byte *keyBytes = napiValue2byteArray(env, argv[1], keyLen);
    
    size outLen;
    const byte *outBytes = ngenxx_crypto_aes_encrypt(inBytes, inLen, keyBytes, keyLen, &outLen);
    napi_value v = byteArray2NapiValue(env, outBytes, outLen);
    
    free((void *)outBytes);
    free((void *)keyBytes);
    free((void *)inBytes);
    free((void *)argv);
    return v;
}

static napi_value CryptoAesDecrypt(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    uint32_t inLen = napiValueLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    uint32_t keyLen = napiValueLen(env, argv[1]);
    const byte *keyBytes = napiValue2byteArray(env, argv[1], keyLen);
    
    size outLen;
    const byte *outBytes = ngenxx_crypto_aes_decrypt(inBytes, inLen, keyBytes, keyLen, &outLen);
    napi_value v = byteArray2NapiValue(env, outBytes, outLen);
    
    free((void *)outBytes);
    free((void *)keyBytes);
    free((void *)inBytes);
    free((void *)argv);
    return v;
}

static napi_value CryptoHashMd5(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    uint32_t inLen = napiValueLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    
    size outLen;
    const byte *outBytes = ngenxx_crypto_hash_md5(inBytes, inLen, &outLen);
    napi_value v = byteArray2NapiValue(env, outBytes, outLen);
    
    free((void *)outBytes);
    free((void *)inBytes);
    free((void *)argv);
    return v;
}

static napi_value CryptoHashSha256(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    uint32_t inLen = napiValueLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    
    size outLen;
    const byte *outBytes = ngenxx_crypto_hash_sha256(inBytes, inLen, &outLen);
    napi_value v = byteArray2NapiValue(env, outBytes, outLen);
    
    free((void *)outBytes);
    free((void *)inBytes);
    free((void *)argv);
    return v;
}

static napi_value CryptoBase64Encode(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    uint32_t inLen = napiValueLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    
    size outLen;
    const byte *outBytes = ngenxx_crypto_base64_encode(inBytes, inLen, &outLen);
    napi_value v = byteArray2NapiValue(env, outBytes, outLen);
    
    free((void *)outBytes);
    free((void *)inBytes);
    free((void *)argv);
    return v;
}

static napi_value CryptoBase64Decode(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    uint32_t inLen = napiValueLen(env, argv[0]);
    const byte *inBytes = napiValue2byteArray(env, argv[0], inLen);
    
    size outLen;
    const byte *outBytes = ngenxx_crypto_base64_decode(inBytes, inLen, &outLen);
    napi_value v = byteArray2NapiValue(env, outBytes, outLen);
    
    free((void *)outBytes);
    free((void *)inBytes);
    free((void *)argv);
    return v;
}

#pragma mark Lua

static napi_value LLoadF(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    const char *file = napiValue2chars(env, argv[0]);

    bool res = ngenxx_L_loadF(file);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)file);
    free((void *)argv);
    return nv;
}

static napi_value LLoadS(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 1);

    const char *script = napiValue2chars(env, argv[0]);

    bool res = ngenxx_L_loadS(script);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)script);
    free((void *)argv);
    return nv;
}

static napi_value LCall(napi_env env, napi_callback_info info) {
    napi_value *argv = readParams(env, info, 2);

    const char *func = napiValue2chars(env, argv[0]);
    const char *params = napiValue2chars(env, argv[1]);

    const char *res = ngenxx_L_call(func, params);
    napi_value nv = chars2NapiValue(env, res);

    free((void *)res);
    free((void *)params);
    free((void *)func);
    free((void *)argv);
    return nv;
}

#pragma mark Register Module

EXTERN_C_START
static napi_value RegisterFuncs(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"getVersion", nullptr, GetVersion, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"init", nullptr, Init, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"release", nullptr, Release, nullptr, nullptr, nullptr, napi_default, nullptr},

        {"logSetLevel", nullptr, LogSetLevel, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"logSetCallback", nullptr, LogSetCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"logPrint", nullptr, LogPrint, nullptr, nullptr, nullptr, napi_default, nullptr},

        {"netHttpRequest", nullptr, NetHttpRequest, nullptr, nullptr, nullptr, napi_default, nullptr},

        {"lLoadF", nullptr, LLoadF, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"lLoadS", nullptr, LLoadS, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"lCall", nullptr, LCall, nullptr, nullptr, nullptr, napi_default, nullptr},

        {"storeSQLiteOpen", nullptr, StoreSQLiteOpen, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"storeSQLiteExecute", nullptr, StoreSQLiteExecute, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"storeSQLiteQueryDo", nullptr, StoreSQLiteQueryDo, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"storeSQLiteQueryReadRow", nullptr, StoreSQLiteQueryReadRow, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"storeSQLiteQueryReadColumnText", nullptr, StoreSQLiteQueryReadColumnText, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"storeSQLiteQueryReadColumnInteger", nullptr, StoreSQLiteQueryReadColumnInteger, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"storeSQLiteQueryReadColumnFloat", nullptr, StoreSQLiteQueryReadColumnFloat, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"storeSQLiteQueryDrop", nullptr, StoreSQLiteQueryDrop, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"storeSQLiteClose", nullptr, StoreSQLiteClose, nullptr, nullptr, nullptr, napi_default, nullptr},

        {"storeKVOpen", nullptr, StoreKVOpen, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"storeKVReadString", nullptr, StoreKVReadString, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"storeKVWriteString", nullptr, StoreKVWriteString, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"storeKVReadInteger", nullptr, StoreKVReadInteger, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"storeKVWriteInteger", nullptr, StoreKVWriteInteger, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"storeKVReadFloat", nullptr, StoreKVReadFloat, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"storeKVWriteFloat", nullptr, StoreKVWriteFloat, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"storeKVContains", nullptr, StoreKVContains, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"storeKVClear", nullptr, StoreKVClear, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"storeKVClose", nullptr, StoreKVClose, nullptr, nullptr, nullptr, napi_default, nullptr},

        {"deviceType", nullptr, DeviceType, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deviceName", nullptr, DeviceName, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deviceManufacturer", nullptr, DeviceManufacturer, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deviceOsVersion", nullptr, DeviceOsVersion, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"deviceCpuArch", nullptr, DeviceCpuArch, nullptr, nullptr, nullptr, napi_default, nullptr},

        {"jsonDecoderInit", nullptr, JsonDecoderInit, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"jsonDecoderIsArray", nullptr, JsonDecoderIsArray, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"jsonDecoderIsObject", nullptr, JsonDecoderIsObject, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"jsonDecoderReadNode", nullptr, JsonDecoderReadNode, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"jsonDecoderReadChild", nullptr, JsonDecoderReadChild, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"jsonDecoderReadNext", nullptr, JsonDecoderReadNext, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"jsonDecoderReadString", nullptr, JsonDecoderReadString, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"jsonDecoderReadNumber", nullptr, JsonDecoderReadNumber, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"jsonDecoderRelease", nullptr, JsonDecoderRelease, nullptr, nullptr, nullptr, napi_default, nullptr},

        {"cryptoAesEncrypt", nullptr, CryptoAesEncrypt, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"cryptoAesDecrypt", nullptr, CryptoAesDecrypt, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"cryptoHashMd5", nullptr, CryptoHashMd5, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"cryptoHashSha256", nullptr, CryptoHashSha256, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"cryptoBase64Encode", nullptr, CryptoBase64Encode, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"cryptoBase64Decode", nullptr, CryptoBase64Decode, nullptr, nullptr, nullptr, napi_default,
         nullptr},
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module ngenxxModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = RegisterFuncs,
    .nm_modname = "ngenxx",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterNGenXXModule(void) { napi_module_register(&ngenxxModule); }