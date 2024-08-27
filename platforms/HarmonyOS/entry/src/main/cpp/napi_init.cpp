#include "napi/native_api.h"
#include "../../../../../../build.HarmonyOS/output/include/EngineXX.h"
#include <string.h>
#include <stdlib.h>

#define CHECK_NAPI_STATUS(env, status, errMsg)                                                                         \
    do {                                                                                                               \
        if (status != napi_ok) {                                                                                       \
            napi_throw_error(env, NULL, errMsg);                                                                       \
            return NULL;                                                                                               \
        }                                                                                                              \
    } while (0);

static const char *napiValue2char(napi_env env, napi_value nv);
static const long napiValue2long(napi_env env, napi_value nv);
static napi_value char2NapiValue(napi_env env, const char *c);
static napi_value long2NapiValue(napi_env env, long l);
static napi_value int2NapiValue(napi_env env, int i);

static napi_value GetVersion(napi_env env, napi_callback_info info)
{
    const char *c = enginexx_get_version();
    napi_value v = char2NapiValue(env, c);
    free((void *)c);
    return v;
}

static napi_value HttpReq(napi_env env, napi_callback_info info) 
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CHECK_NAPI_STATUS(env, status, "napi_get_cb_info() failed");

    const char *cUrl = napiValue2char(env, args[0]);
    const char *cParams = napiValue2char(env, args[1]);
    
    const char *cRsp = enginexx_http_req(cUrl, cParams);
    free((void *)cUrl);
    free((void *)cParams);

    napi_value vRsp = char2NapiValue(env, cRsp);
    free((void *)cRsp);
    return vRsp;
}

static napi_value LCreate(napi_env env, napi_callback_info info)
{
    long l = (long) enginexx_L_create();
    napi_value v = long2NapiValue(env, l);
    return v;
}

static napi_value LLoadF(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CHECK_NAPI_STATUS(env, status, "napi_get_cb_info() failed");

    long lstate = napiValue2long(env, args[0]);
    const char *file = napiValue2char(env, args[1]);
    
    long ret = enginexx_L_loadF((void*)lstate, file);
    if (ret != 0) {
        napi_throw_error(env, NULL, "enginexx_L_loadF() failed");
        return NULL;
    }
    free((void *)file);

    return long2NapiValue(env, ret);
}

static napi_value LLoadS(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CHECK_NAPI_STATUS(env, status, "napi_get_cb_info() failed");

    long lstate = napiValue2long(env, args[0]);
    const char *script = napiValue2char(env, args[1]);
    
    long ret = enginexx_L_loadS((void*)lstate, script);
    if (ret != 0) {
        napi_throw_error(env, NULL, "enginexx_L_loadS() failed");
        return NULL;
    }
    free((void *)script);

    return long2NapiValue(env, ret);
}

static napi_value LCall(napi_env env, napi_callback_info info)
{
    size_t argc = 3;
    napi_value args[3] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CHECK_NAPI_STATUS(env, status, "napi_get_cb_info() failed");

    long lstate = napiValue2long(env, args[0]);
    const char *func = napiValue2char(env, args[1]);
    const char *params = napiValue2char(env, args[2]);
    
    const char* cRes = enginexx_L_call((void*)lstate, func, params);
    free((void *)func);
    free((void *)params);

    napi_value vRes = char2NapiValue(env, cRes);
    free((void *)cRes);
    return vRes;
}

static napi_value LDestroy(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};

    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    CHECK_NAPI_STATUS(env, status, "napi_get_cb_info() failed");

    long lstate = napiValue2long(env, args[0]);
    
    enginexx_L_destroy((void*)lstate);

    return int2NapiValue(env, napi_ok);
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) 
{
    napi_property_descriptor desc[] = {
        {"getVersion", nullptr, GetVersion, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"httpReq", nullptr, HttpReq, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"lCreate", nullptr, LCreate, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"lLoadF", nullptr, LLoadF, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"lLoadS", nullptr, LLoadS, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"lCall", nullptr, LCall, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"lDestroy", nullptr, LDestroy, nullptr, nullptr, nullptr, napi_default, nullptr}
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module enginexxModule = 
{
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "enginexx",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) 
void RegisterEnginexxModule(void) 
{ 
    napi_module_register(&enginexxModule); 
}



static const char *napiValue2char(napi_env env, napi_value nv) 
{
    napi_status status;
    size_t len;
    status = napi_get_value_string_utf8(env, nv, NULL, 0, &len);
    CHECK_NAPI_STATUS(env, status, "napi_get_value_string_utf8() get length failed");
    char *cStr = (char *)malloc(len + 1);
    status = napi_get_value_string_utf8(env, nv, cStr, len + 1, &len);
    CHECK_NAPI_STATUS(env, status, "napi_get_value_string_utf8() get content failed");
    return cStr;
}

static const long napiValue2long(napi_env env, napi_value nv)
{
    long l;
    napi_status status = napi_get_value_int64(env, nv, &l);
    CHECK_NAPI_STATUS(env, status, "napi_get_value_int64() get content failed");
    return l;
}

static napi_value char2NapiValue(napi_env env, const char *c) 
{
    napi_value v;
    napi_status status = napi_create_string_utf8(env, c, strlen(c), &v);
    CHECK_NAPI_STATUS(env, status, "napi_create_string_utf8() failed");
    return v;
}

static napi_value long2NapiValue(napi_env env, long l)
{
    napi_value v;
    napi_status status = napi_create_int64(env, l, &v);
    CHECK_NAPI_STATUS(env, status, "napi_create_int64() failed");
    return v;
}

static napi_value int2NapiValue(napi_env env, int i)
{
    napi_value v;
    napi_status status = napi_create_int32(env, i, &v);
    CHECK_NAPI_STATUS(env, status, "napi_create_int32() failed");
    return v;
}