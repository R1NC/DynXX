#pragma once

#include <napi/native_api.h>

#include "../../../../../../build.HarmonyOS/output/include/DynXX/CXX/Types.hxx"

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
            return napiValueFromInt(env, napi_cancelled);                                                                 \
        }                                                                                                              \
    } while (0);

#define CHECK_NAPI_STATUS_RETURN_VOID(env, status, errMsg)                                                             \
    do {                                                                                                               \
        if (status != napi_ok) {                                                                                       \
            PRINT_NAPI_STATUS_ERR(env, status, errMsg);                                                                \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0);


size_t napiValueArrayLen(napi_env env, napi_value nv);
const char *napiValueToChars(napi_env env, napi_value nv);
bool napiValueToBool(napi_env env, napi_value nv);
int napiValueToInt(napi_env env, napi_value nv);
long napiValueToLong(napi_env env, napi_value nv);
double napiValueToDouble(napi_env env, napi_value nv);
std::tuple<const byte *, size_t> napiValueToByteArray(napi_env env, napi_value nv);
std::tuple<const char **, size_t> napiValueToCharsArray(napi_env env, napi_value nv);

napi_value napiValueFromChars(napi_env env, const char *c);
napi_value napiValueFromLong(napi_env env, long l);
napi_value napiValueFromInt(napi_env env, int i);
napi_value napiValueFromBool(napi_env env, bool b);
napi_value napiValueFromDouble(napi_env env, double d);
napi_value napiValueFromByteArray(napi_env env, const byte *byteArray, size_t len);
napi_value napiValueFromCharsArray(napi_env env, const char **charsArray, size_t len);

class NapiCallContext {
public:
    NapiCallContext() = delete;
    NapiCallContext(napi_env env, napi_callback_info info);
    NapiCallContext(const NapiCallContext &) = delete;
    NapiCallContext & operator = (const NapiCallContext &) = delete;
    NapiCallContext(NapiCallContext &&) = delete;
    NapiCallContext & operator = (NapiCallContext &&) = delete;
    ~NapiCallContext();

    napi_value argAt(size_t i);
    size_t argCount() const;

    const char *argCharsAt(size_t i);
    bool argBoolAt(size_t i);
    int argIntAt(size_t i);
    long argLongAt(size_t i);
    double argDoubleAt(size_t i);
    std::tuple<const byte *, size_t> argByteArrayAt(size_t i);
    std::tuple<const char **, size_t> argCharsArrayAt(size_t i);

    const char *napiValueToChars(napi_value nv);
    bool napiValueToBool(napi_value nv);
    int napiValueToInt(napi_value nv);
    long napiValueToLong(napi_value nv);
    double napiValueToDouble(napi_value nv);
    std::tuple<const byte *, size_t> napiValueToByteArray(napi_value nv);
    std::tuple<const char **, size_t> napiValueToCharsArray(napi_value nv);

    napi_value napiValueFromChars(const char *c);
    napi_value napiValueFromLong(long l);
    napi_value napiValueFromInt(int i);
    napi_value napiValueFromBool(bool b);
    napi_value napiValueFromDouble(double d);
    napi_value napiValueFromByteArray(const byte *byteArray, size_t len);
    napi_value napiValueFromCharsArray(const char **charsArray, size_t len);

private:
    napi_env env;
    napi_callback_info cbkInfo;
    size_t argc{0};
    napi_value *argv{nullptr};
    std::vector<void*> ptrVec;
};
