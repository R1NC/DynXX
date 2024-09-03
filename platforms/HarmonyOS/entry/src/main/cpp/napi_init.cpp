#include "napi/native_api.h"
#include "../../../../../../build.HarmonyOS/output/include/NGenXX.h"
#include <cstdio>
#include <string.h>
#include <stdlib.h>

#define PRINT_NAPI_STATUS_ERR(env, status, errMsg)                                                                     \
    do {                                                                                                               \
        char msg[128];                                                                                                 \
        sprintf(msg, "status=%d desc='%s'", status, errMsg);                                                           \
        napi_throw_error(env, NULL, msg);                                                                              \
    } while (0);

#define CHECK_NAPI_STATUS_RETURN_ANY(env, status, errMsg)                                                              \
    do {                                                                                                               \
        if (status != napi_ok) {                                                                                       \
            PRINT_NAPI_STATUS_ERR(env, status, errMsg);                                                                \
            return NULL;                                                                                               \
        }                                                                                                              \
    } while (0);

#define CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, errMsg)                                                       \
    do {                                                                                                               \
        if (status != napi_ok) {                                                                                       \
            PRINT_NAPI_STATUS_ERR(env, status, errMsg);                                                                \
            return int2NapiValue(env, napi_cancelled);                                                                 \
        }                                                                                                              \
    } while (0);

#define CHECK_NAPI_STATUS_RETURN_VOID(env, status, errMsg)                                                             \
    do {                                                                                                               \
        if (status != napi_ok) {                                                                                       \
            PRINT_NAPI_STATUS_ERR(env, status, errMsg);                                                                \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0);

static const char *napiValue2char(napi_env env, napi_value nv);
static const int napiValue2bool(napi_env env, napi_value nv);
static const int napiValue2int(napi_env env, napi_value nv);
static const long napiValue2long(napi_env env, napi_value nv);
static napi_value char2NapiValue(napi_env env, const char *c);
static napi_value long2NapiValue(napi_env env, long l);
static napi_value int2NapiValue(napi_env env, int i);
static napi_value bool2NapiValue(napi_env env, bool b);

static napi_value GetVersion(napi_env env, napi_callback_info info) {
    const char *c = ngenxx_get_version();
    napi_value v = char2NapiValue(env, c);
    free((void *)c);
    return v;
}

static napi_value Init(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    int useLua = napiValue2bool(env, argv[0]);

    long l = (long)ngenxx_init(useLua);
    napi_value v = long2NapiValue(env, l);
    return v;
}

static napi_value Release(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long handle = napiValue2long(env, argv[0]);

    ngenxx_release((void *)handle);

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
    argv[1] = char2NapiValue(env, tSLogWorkData->logContent);

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

    napi_value vWorkName = char2NapiValue(sNapiEnv, "NAPI_LOG_CALLBACK_WORK");

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
    napi_value argv[2] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    int level = napiValue2int(env, argv[0]);
    const char *content = napiValue2char(env, argv[1]);

    ngenxx_log_print(level, content);
    free((void *)content);

    return int2NapiValue(env, napi_ok);
}

#pragma mark Net

static napi_value NetHttpReq(napi_env env, napi_callback_info info) {
    size_t argc = 5;
    napi_value args[5] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    const char *cUrl = napiValue2char(env, argv[0]);
    const char *cParams = napiValue2char(env, argv[1]);
    int iMethod = napiValue2int(env, args[2]);
    
    uint32_t headers_c;
    status = napi_get_array_length(env, args[3], &headers_c);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_array_length() failed");
    char **headers_v = (char **)malloc(headers_c * sizeof(char*));
    for (int i = 0; i < headers_c; i++) {
        napi_value vHeader;
        status = napi_get_element(env, args[3], i, &vHeader);
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_element() failed");
        headers_v[i] = (char *)napiValue2char(env, vHeader);
    }
    
    long lTimeout = napiValue2long(env, args[4]);

    const char *cRsp = ngenxx_net_http_request(cUrl, cParams, iMethod, headers_v, headers_c, lTimeout);

    free((void *)cUrl);
    free((void *)cParams);
    for (int i = 0; i < headers_c; i++) {
        free((void *)headers_v[i]);
    }
    free((void *)headers_v);

    napi_value vRsp = char2NapiValue(env, cRsp);
    free((void *)cRsp);
    return vRsp;
}

#pragma mark Log


#pragma mark Lua

static napi_value LLoadF(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long handle = napiValue2long(env, argv[0]);
    const char *file = napiValue2char(env, argv[1]);

    bool ret = ngenxx_L_loadF((void *)handle, file);
    free((void *)file);

    return bool2NapiValue(env, ret);
}

static napi_value LLoadS(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv[2] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long handle = napiValue2long(env, argv[0]);
    const char *script = napiValue2char(env, argv[1]);

    bool ret = ngenxx_L_loadS((void *)handle, script);
    free((void *)script);

    return bool2NapiValue(env, ret);
}

static napi_value LCall(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value argv[3] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long handle = napiValue2long(env, argv[0]);
    const char *func = napiValue2char(env, argv[1]);
    const char *params = napiValue2char(env, argv[2]);

    const char *cRes = ngenxx_L_call((void *)handle, func, params);
    free((void *)func);
    free((void *)params);

    napi_value vRes = char2NapiValue(env, cRes);
    free((void *)cRes);
    return vRes;
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

        {"netHttpReq", nullptr, NetHttpReq, nullptr, nullptr, nullptr, napi_default, nullptr},
        
        {"lLoadF", nullptr, LLoadF, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"lLoadS", nullptr, LLoadS, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"lCall", nullptr, LCall, nullptr, nullptr, nullptr, napi_default, nullptr},
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

#pragma mark NAPI Utils

static const char *napiValue2char(napi_env env, napi_value nv) {
    napi_status status;
    size_t len;
    status = napi_get_value_string_utf8(env, nv, NULL, 0, &len);
    CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_value_string_utf8() get length failed");
    char *cStr = (char *)malloc(len + 1);
    status = napi_get_value_string_utf8(env, nv, cStr, len + 1, &len);
    CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_value_string_utf8() get content failed");
    return cStr;
}

static const int napiValue2bool(napi_env env, napi_value nv) {
    bool i;
    napi_status status = napi_get_value_bool(env, nv, &i);
    CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_value_bool() get content failed");
    return i;
}

static const int napiValue2int(napi_env env, napi_value nv) {
    int i;
    napi_status status = napi_get_value_int32(env, nv, &i);
    CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_value_int32() get content failed");
    return i;
}

static const long napiValue2long(napi_env env, napi_value nv) {
    long l;
    napi_status status = napi_get_value_int64(env, nv, &l);
    CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_value_int64() get content failed");
    return l;
}

static napi_value char2NapiValue(napi_env env, const char *c) {
    if (c == NULL) c = "";
    napi_value v;
    napi_status status = napi_create_string_utf8(env, c, strlen(c), &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_string_utf8() failed");
    return v;
}

static napi_value long2NapiValue(napi_env env, long l) {
    napi_value v;
    napi_status status = napi_create_int64(env, l, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_int64() failed");
    return v;
}

static napi_value int2NapiValue(napi_env env, int i) {
    napi_value v;
    napi_status status = napi_create_int32(env, i, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_int32() failed");
    return v;
}

static napi_value bool2NapiValue(napi_env env, bool b) {
    napi_value v;
    napi_get_boolean(env, b, &v);
    return v;
}