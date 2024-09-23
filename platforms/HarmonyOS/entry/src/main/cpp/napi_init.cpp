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
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    const char *root = napiValue2chars(env, argv[0]);

    bool b = ngenxx_init(root);
    napi_value v = bool2NapiValue(env, b);

    free((void *)root);
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
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    int level = napiValue2int(env, argv[0]);

    ngenxx_log_set_level(level);

    return int2NapiValue(env, napi_ok);
}

static napi_value LogSetCallback(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    napi_value vLogCallback = argv[0];
    if (vLogCallback == NULL) {
        sNapiEnv = NULL;

        napi_delete_reference(env, sTsLogCallbackRef);
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_delete_reference() failed");

        ngenxx_log_set_callback(NULL);
    } else {
        sNapiEnv = env;

        napi_create_reference(env, vLogCallback, 1, &sTsLogCallbackRef);
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_reference() failed");

        ngenxx_log_set_callback(engineLogCallback);
    }

    return int2NapiValue(env, napi_ok);
}

static napi_value LogPrint(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    int level = napiValue2int(env, argv[0]);
    const char *content = napiValue2chars(env, argv[1]);

    ngenxx_log_print(level, content);
    free((void *)content);

    return int2NapiValue(env, napi_ok);
}

#pragma mark Net

static napi_value NetHttpRequest(napi_env env, napi_callback_info info) {
    size_t argc = 5;
    napi_value argv[5] = {nullptr, nullptr, nullptr, nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    const char *cUrl = napiValue2chars(env, argv[0]);
    const char *cParams = napiValue2chars(env, argv[1]);
    int iMethod = napiValue2int(env, argv[2]);

    uint32_t headers_c;
    status = napi_get_array_length(env, argv[3], &headers_c);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_array_length() failed");
    char **headers_v = (char **)malloc(headers_c * sizeof(char *));
    for (int i = 0; i < headers_c; i++) {
        napi_value vHeader;
        status = napi_get_element(env, argv[3], i, &vHeader);
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_element() failed");
        headers_v[i] = (char *)napiValue2chars(env, vHeader);
    }

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
    return nv;
}

#pragma mark Store.SQLite

static napi_value StoreSQLiteOpen(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    const char *_id = napiValue2chars(env, argv[0]);
    long res = (long)ngenxx_store_sqlite_open(_id);
    napi_value nv = long2NapiValue(env, res);

    free((void *)_id);
    return nv;
}

static napi_value StoreSQLiteExecute(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long conn = napiValue2long(env, argv[0]);
    const char *sql = napiValue2chars(env, argv[1]);

    bool res = ngenxx_store_sqlite_execute((void *)conn, sql);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)sql);
    return nv;
}

static napi_value StoreSQLiteQueryDo(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long conn = napiValue2long(env, argv[0]);
    const char *sql = napiValue2chars(env, argv[1]);

    long res = (long)ngenxx_store_sqlite_query_do((void *)conn, sql);
    napi_value nv = long2NapiValue(env, res);

    free((void *)sql);
    return nv;
}

static napi_value StoreSQLiteQueryReadRow(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long query_result = napiValue2long(env, argv[0]);

    bool res = ngenxx_store_sqlite_query_read_row((void *)query_result);
    napi_value nv = bool2NapiValue(env, res);

    return nv;
}

static napi_value StoreSQLiteQueryReadColumnText(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long query_result = napiValue2long(env, argv[0]);
    const char *column = napiValue2chars(env, argv[1]);

    const char *res = ngenxx_store_sqlite_query_read_column_text((void *)query_result, column);
    napi_value nv = chars2NapiValue(env, res);

    free((void *)res);
    free((void *)column);
    return nv;
}

static napi_value StoreSQLiteQueryReadColumnInteger(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long query_result = napiValue2long(env, argv[0]);
    const char *column = napiValue2chars(env, argv[1]);

    long res = ngenxx_store_sqlite_query_read_column_integer((void *)query_result, column);
    napi_value nv = long2NapiValue(env, res);

    free((void *)column);
    return nv;
}

static napi_value StoreSQLiteQueryReadColumnFloat(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long query_result = napiValue2long(env, argv[0]);
    const char *column = napiValue2chars(env, argv[1]);

    double res = ngenxx_store_sqlite_query_read_column_float((void *)query_result, column);
    napi_value nv = double2NapiValue(env, res);

    free((void *)column);
    return nv;
}

static napi_value StoreSQLiteQueryDrop(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long query_result = napiValue2long(env, argv[0]);
    ngenxx_store_sqlite_query_drop((void *)query_result);

    return int2NapiValue(env, napi_ok);
}

static napi_value StoreSQLiteClose(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long conn = napiValue2long(env, argv[0]);
    ngenxx_store_sqlite_close((void *)conn);

    return int2NapiValue(env, napi_ok);
}

#pragma mark Store.KV

static napi_value StoreKVOpen(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    const char *_id = napiValue2chars(env, argv[0]);
    long res = (long)ngenxx_store_kv_open(_id);
    napi_value nv = long2NapiValue(env, res);

    free((void *)_id);
    return nv;
}

static napi_value StoreKVReadString(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);

    const char *res = ngenxx_store_kv_read_string((void *)conn, k);
    napi_value nv = chars2NapiValue(env, res);

    free((void *)res);
    free((void *)k);
    return nv;
}

static napi_value StoreKVWriteString(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value argv[3] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);
    const char *v = napiValue2chars(env, argv[2]);

    bool res = ngenxx_store_kv_write_string((void *)conn, k, v);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)v);
    free((void *)k);
    return nv;
}

static napi_value StoreKVReadInteger(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);

    long res = ngenxx_store_kv_read_integer((void *)conn, k);
    napi_value nv = long2NapiValue(env, res);

    free((void *)k);
    return nv;
}

static napi_value StoreKVWriteInteger(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value argv[3] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);
    long v = napiValue2long(env, argv[2]);

    bool res = ngenxx_store_kv_write_integer((void *)conn, k, v);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)k);
    return nv;
}

static napi_value StoreKVReadFloat(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);

    double res = ngenxx_store_kv_read_integer((void *)conn, k);
    napi_value nv = double2NapiValue(env, res);

    free((void *)k);
    return nv;
}

static napi_value StoreKVWriteFloat(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value argv[3] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);
    double v = napiValue2double(env, argv[2]);

    bool res = ngenxx_store_kv_write_integer((void *)conn, k, v);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)k);
    return nv;
}

static napi_value StoreKVContains(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long conn = napiValue2long(env, argv[0]);
    const char *k = napiValue2chars(env, argv[1]);

    bool res = ngenxx_store_kv_contains((void *)conn, k);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)k);
    return nv;
}

static napi_value StoreKVClear(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long conn = napiValue2long(env, argv[0]);
    ngenxx_store_kv_clear((void *)conn);

    return int2NapiValue(env, napi_ok);
}

static napi_value StoreKVClose(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long conn = napiValue2long(env, argv[0]);
    ngenxx_store_kv_close((void *)conn);

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
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    const char *cJson = napiValue2chars(env, argv[0]);
    
    long res = (long)ngenxx_json_decoder_init(cJson);
    napi_value v = long2NapiValue(env, res);

    free((void *)cJson);
    return v;
}

static napi_value JsonDecoderIsArray(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    bool res = ngenxx_json_decoder_is_array((void *)decoder, (void *)node);
    napi_value v = bool2NapiValue(env, res);
    
    return v;
}

static napi_value JsonDecoderIsObject(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    bool res = ngenxx_json_decoder_is_object((void *)decoder, (void *)node);
    napi_value v = bool2NapiValue(env, res);
    
    return v;
}

static napi_value JsonDecoderReadNode(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value argv[3] = {nullptr, nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    const char *cK = napiValue2chars(env, argv[2]);
    
    long res = (long)ngenxx_json_decoder_read_node((void *)decoder, (void *)node, cK);
    napi_value v = long2NapiValue(env, res);

    free((void *)cK);
    return v;
}

static napi_value JsonDecoderReadChild(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    long res = (long)ngenxx_json_decoder_read_child((void *)decoder, (void *)node);
    napi_value v = long2NapiValue(env, res);
    
    return v;
}

static napi_value JsonDecoderReadNext(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    long res = (long)ngenxx_json_decoder_read_next((void *)decoder, (void *)node);
    napi_value v = long2NapiValue(env, res);
    
    return v;
}

static napi_value JsonDecoderReadString(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    const char *cRes = ngenxx_json_decoder_read_string((void *)decoder, (void *)node);
    napi_value v = chars2NapiValue(env, cRes);
    
    free((void *)cRes);
    return v;
}

static napi_value JsonDecoderReadNumber(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long decoder = napiValue2long(env, argv[0]);
    long node = napiValue2long(env, argv[1]);
    
    double res = ngenxx_json_decoder_read_number((void *)decoder, (void *)node);
    napi_value v = double2NapiValue(env, res);
    
    return v;
}

static napi_value JsonDecoderRelease(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long decoder = napiValue2long(env, argv[0]);
    
    ngenxx_json_decoder_release((void *)decoder);
    
    return int2NapiValue(env, napi_ok);
}

#pragma mark Lua

static napi_value LLoadF(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    const char *file = napiValue2chars(env, argv[0]);

    bool res = ngenxx_L_loadF(file);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)file);
    return nv;
}

static napi_value LLoadS(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    const char *script = napiValue2chars(env, argv[0]);

    bool res = ngenxx_L_loadS(script);
    napi_value nv = bool2NapiValue(env, res);

    free((void *)script);
    return nv;
}

static napi_value LCall(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr, nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    const char *func = napiValue2chars(env, argv[0]);
    const char *params = napiValue2chars(env, argv[1]);

    const char *res = ngenxx_L_call(func, params);
    napi_value nv = chars2NapiValue(env, res);

    free((void *)res);
    free((void *)params);
    free((void *)func);
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
        {"jsonDecoderIsArray", nullptr, JsonDecoderIsArray, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"jsonDecoderIsObject", nullptr, JsonDecoderIsObject, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"jsonDecoderReadNode", nullptr, JsonDecoderReadNode, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"jsonDecoderReadChild", nullptr, JsonDecoderReadChild, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"jsonDecoderReadNext", nullptr, JsonDecoderReadNext, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"jsonDecoderReadString", nullptr, JsonDecoderReadString, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"jsonDecoderReadNumber", nullptr, JsonDecoderReadNumber, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"jsonDecoderRelease", nullptr, JsonDecoderRelease, nullptr, nullptr, nullptr, napi_default,
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