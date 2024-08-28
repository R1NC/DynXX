#include "napi/native_api.h"
#include "../../../../../../build.HarmonyOS/output/include/EngineXX.h"
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
static const int napiValue2int(napi_env env, napi_value nv);
static const long napiValue2long(napi_env env, napi_value nv);
static napi_value char2NapiValue(napi_env env, const char *c);
static napi_value long2NapiValue(napi_env env, long l);
static napi_value int2NapiValue(napi_env env, int i);

static napi_value GetVersion(napi_env env, napi_callback_info info) {
    const char *c = enginexx_get_version();
    napi_value v = char2NapiValue(env, c);
    free((void *)c);
    return v;
}

#pragma mark Log Callback

typedef struct {
    int level;
    const char *content;
} LogData;

typedef struct {
    napi_async_work work;
    napi_threadsafe_function func;
    LogData *data;
} LogWork;

static napi_env sNapiEnv;
static napi_value sTsLogCallback;
static napi_ref sTsLogCallbackRef;

static void callTsLogFunc(napi_env env, napi_value ts_callback, LogData *logData) {
    size_t argc = 2;
    napi_value argv[2];
    argv[0] = int2NapiValue(env, logData->level);
    argv[1] = char2NapiValue(env, logData->content);

    napi_value vGlobal;
    napi_status status = napi_get_global(env, &vGlobal);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_get_global() failed");

    status = napi_call_function(env, vGlobal, ts_callback, argc, argv, NULL);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_call_function() failed");
}

static void OnLogWorkCallTS(napi_env env, napi_value ts_callback, void *context, void *data) {
    if (env == NULL || ts_callback == NULL || data == NULL)
        return;

    LogData *logData = (LogData *)data;

    napi_get_reference_value(sNapiEnv, sTsLogCallbackRef, &ts_callback);

    callTsLogFunc(env, ts_callback, logData);

    free(data);
}

static void OnLogWorkExecute(napi_env env, void *data) {
    LogWork *logWork = (LogWork *)data;

    napi_status status = napi_acquire_threadsafe_function(logWork->func);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_acquire_threadsafe_function() failed");

    status = napi_call_threadsafe_function(logWork->func, logWork->data, napi_tsfn_blocking);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_call_threadsafe_function() failed");
}

static void OnLogWorkComplete(napi_env env, napi_status status, void *data) {
    LogWork *logWork = (LogWork *)data;

    status = napi_release_threadsafe_function(logWork->func, napi_tsfn_release);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_release_threadsafe_function() failed");

    status = napi_delete_async_work(env, logWork->work);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_delete_async_work() failed");

    logWork->work = NULL;
    logWork->func = NULL;

    // free(data);
}

static void InnerLogCallback(int level, const char *content) {
    if (sNapiEnv == NULL || sTsLogCallback == NULL || content == NULL)
        return;

    LogData *logData = (LogData *)malloc(sizeof(LogData *));
    logData->level = level;
    logData->content = content;

    LogWork *logWork = (LogWork *)malloc(sizeof(LogWork *));
    logWork->func = NULL;
    logWork->work = NULL;
    logWork->data = logData;
    napi_value vWorkName = char2NapiValue(sNapiEnv, "NAPI_LOG_CALLBACK_WORK");

    napi_status status = napi_create_threadsafe_function(sNapiEnv, sTsLogCallback, NULL, vWorkName, 0, 1, NULL, NULL,
                                                         NULL, OnLogWorkCallTS, &(logWork->func));
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_create_threadsafe_function() failed");

    status = napi_create_async_work(sNapiEnv, NULL, vWorkName, OnLogWorkExecute, OnLogWorkComplete, logWork,
                                    &(logWork->work));
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_create_async_work() failed");

    status = napi_queue_async_work(sNapiEnv, logWork->work);
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_queue_async_work() failed");
}

#pragma mark Log API

static napi_value LogSetLevel(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv_[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv_, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    int level = napiValue2int(env, argv_[0]);

    enginexx_log_set_level(level);

    return int2NapiValue(env, napi_ok);
}

static napi_value LogSetCallback(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");
    
    sTsLogCallback = argv[0];
    if (sTsLogCallback == NULL) {
        sNapiEnv = NULL;
        
        napi_delete_reference(env, sTsLogCallbackRef);
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_delete_reference() failed");
        
        enginexx_log_set_callback(NULL);
    } else {
        sNapiEnv = env;
        
        napi_create_reference(env, sTsLogCallback, 1, &sTsLogCallbackRef);
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_reference() failed");
        
        enginexx_log_set_callback(InnerLogCallback);
    }

    return int2NapiValue(env, napi_ok);
}

static napi_value LogPrint(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv_[2] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv_, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    int level = napiValue2int(env, argv_[0]);
    const char *content = napiValue2char(env, argv_[1]);

    enginexx_log_print(level, content);
    free((void *)content);

    return int2NapiValue(env, napi_ok);
}

#pragma mark Net

static napi_value NetHttpReq(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv_[2] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv_, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    const char *cUrl = napiValue2char(env, argv_[0]);
    const char *cParams = napiValue2char(env, argv_[1]);

    const char *cRsp = enginexx_net_http_req(cUrl, cParams);
    free((void *)cUrl);
    free((void *)cParams);

    napi_value vRsp = char2NapiValue(env, cRsp);
    free((void *)cRsp);
    return vRsp;
}

#pragma mark Log


#pragma mark Lua

static napi_value LCreate(napi_env env, napi_callback_info info) {
    long l = (long)enginexx_L_create();
    napi_value v = long2NapiValue(env, l);
    return v;
}

static napi_value LLoadF(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv_[2] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv_, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long lstate = napiValue2long(env, argv_[0]);
    const char *file = napiValue2char(env, argv_[1]);

    long ret = enginexx_L_loadF((void *)lstate, file);
    if (ret != 0) {
        napi_throw_error(env, NULL, "enginexx_L_loadF() failed");
        return NULL;
    }
    free((void *)file);

    return long2NapiValue(env, ret);
}

static napi_value LLoadS(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value argv_[2] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv_, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long lstate = napiValue2long(env, argv_[0]);
    const char *script = napiValue2char(env, argv_[1]);

    long ret = enginexx_L_loadS((void *)lstate, script);
    if (ret != 0) {
        napi_throw_error(env, NULL, "enginexx_L_loadS() failed");
        return NULL;
    }
    free((void *)script);

    return long2NapiValue(env, ret);
}

static napi_value LCall(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value argv_[3] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv_, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long lstate = napiValue2long(env, argv_[0]);
    const char *func = napiValue2char(env, argv_[1]);
    const char *params = napiValue2char(env, argv_[2]);

    const char *cRes = enginexx_L_call((void *)lstate, func, params);
    free((void *)func);
    free((void *)params);

    napi_value vRes = char2NapiValue(env, cRes);
    free((void *)cRes);
    return vRes;
}

static napi_value LDestroy(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value argv_[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, argv_, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_cb_info() failed");

    long lstate = napiValue2long(env, argv_[0]);

    enginexx_L_destroy((void *)lstate);

    return int2NapiValue(env, napi_ok);
}

#pragma mark Register Module

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        {"getVersion", nullptr, GetVersion, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"logSetLevel", nullptr, LogSetLevel, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"logSetCallback", nullptr, LogSetCallback, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"logPrint", nullptr, LogPrint, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"netHttpReq", nullptr, NetHttpReq, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"lCreate", nullptr, LCreate, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"lLoadF", nullptr, LLoadF, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"lLoadS", nullptr, LLoadS, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"lCall", nullptr, LCall, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"lDestroy", nullptr, LDestroy, nullptr, nullptr, nullptr, napi_default, nullptr}};
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module enginexxModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "enginexx",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEnginexxModule(void) { napi_module_register(&enginexxModule); }

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