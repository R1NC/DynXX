#include <cstdio>

#include <napi/native_api.h>

#include "../../../../../../build.HarmonyOS/output/include/NGenXXTypes.hxx"

#define PRINT_NAPI_STATUS_ERR(env, status, errMsg)                                                                     \
    do {                                                                                                               \
        char msg[128];                                                                                                 \
        sprintf(msg, "status=%d desc='%s'", status, errMsg);                                                           \
        napi_throw_error(env, nullptr, msg);                                                                           \
    } while (0);

#define CHECK_NAPI_STATUS_RETURN_TUPLE(env, status, errMsg)                                                            \
    do {                                                                                                               \
        if (status != napi_ok) {                                                                                       \
            PRINT_NAPI_STATUS_ERR(env, status, errMsg);                                                                \
            return {};                                                                                                 \
        }                                                                                                              \
    } while (0);

#define CHECK_NAPI_STATUS_RETURN_NUM(env, status, errMsg)                                                              \
    do {                                                                                                               \
        if (status != napi_ok) {                                                                                       \
            PRINT_NAPI_STATUS_ERR(env, status, errMsg);                                                                \
            return 0;                                                                                                  \
        }                                                                                                              \
    } while (0);

#define CHECK_NAPI_STATUS_RETURN_PTR(env, status, errMsg)                                                              \
    do {                                                                                                               \
        if (status != napi_ok) {                                                                                       \
            PRINT_NAPI_STATUS_ERR(env, status, errMsg);                                                                \
            return nullptr;                                                                                            \
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


class Args 
{
public:
    size_t c{0};
    napi_value *v{nullptr};
    
    Args() = delete;
    Args(napi_env env, napi_callback_info info);
    Args(const Args &) = delete;
    Args &operator=(const Args &) = delete;
    Args(Args &&) = delete;
    Args &operator=(Args &&) = delete;
    
    ~Args();
private:
    napi_env env;
    napi_callback_info info;
};


size_t napiValueArrayLen(napi_env env, napi_value nv);

const char *napiValue2chars(napi_env env, napi_value nv);

bool napiValue2bool(napi_env env, napi_value nv);

int napiValue2int(napi_env env, napi_value nv);

long napiValue2long(napi_env env, napi_value nv);

double napiValue2double(napi_env env, napi_value nv);

const byte *napiValue2byteArray(napi_env env, napi_value nv, size_t len);

const char **napiValue2charsArray(napi_env env, napi_value nv, size_t len);

napi_value chars2NapiValue(napi_env env, const char *c);

napi_value long2NapiValue(napi_env env, long l);

napi_value int2NapiValue(napi_env env, int i);

napi_value bool2NapiValue(napi_env env, bool b);

napi_value double2NapiValue(napi_env env, double d);

napi_value byteArray2NapiValue(napi_env, const byte *byteArray, size_t len);

napi_value charsArray2NapiValue(napi_env, const char **charsArray, size_t len);