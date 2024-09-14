#include "napi/native_api.h"
#include <cstdio>

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

const char *napiValue2char(napi_env env, napi_value nv);

bool napiValue2bool(napi_env env, napi_value nv);

int napiValue2int(napi_env env, napi_value nv);

long napiValue2long(napi_env env, napi_value nv);

double napiValue2double(napi_env env, napi_value nv);

napi_value char2NapiValue(napi_env env, const char *c);

napi_value long2NapiValue(napi_env env, long l);

napi_value int2NapiValue(napi_env env, int i);

napi_value bool2NapiValue(napi_env env, bool b);

napi_value double2NapiValue(napi_env env, double d);