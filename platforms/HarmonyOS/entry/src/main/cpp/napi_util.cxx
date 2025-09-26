#include "napi_util.hxx"

#include "../../../../../../build.HarmonyOS/output/include/DynXX/CXX/Memory.hxx"

#include <cstring>

// static Utils

size_t napiValueArrayLen(napi_env env, napi_value nv) {
    bool is_array;
    auto status = napi_is_array(env, nv, &is_array);
    if (status != napi_ok || !is_array) {
        return 0;
    }
    uint32_t len;
    status = napi_get_array_length(env, nv, &len);
    CHECK_NAPI_STATUS_RETURN_NUM(env, status, "napi_get_array_length() failed");
    return len;
}

const char *napiValueToChars(napi_env env, napi_value nv) {
    size_t len;
    auto status = napi_get_value_string_utf8(env, nv, nullptr, 0, &len);
    CHECK_NAPI_STATUS_RETURN_PTR(env, status, "napi_get_value_string_utf8() failed");
    auto cStr = mallocX<char>(len);
    status = napi_get_value_string_utf8(env, nv, cStr, len + 1, &len);
    CHECK_NAPI_STATUS_RETURN_PTR(env, status, "napi_get_value_string_utf8() failed");
    return cStr;
}

bool napiValueToBool(napi_env env, napi_value nv) {
    bool b = false;
    auto status = napi_get_value_bool(env, nv, &b);
    CHECK_NAPI_STATUS_RETURN_NUM(env, status, "napi_get_value_bool() failed");
    return b;
}

int napiValueToInt(napi_env env, napi_value nv) {
    int i;
    auto status = napi_get_value_int32(env, nv, &i);
    CHECK_NAPI_STATUS_RETURN_NUM(env, status, "napi_get_value_int32() failed");
    return i;
}

long napiValueToLong(napi_env env, napi_value nv) {
    long l;
    auto status = napi_get_value_int64(env, nv, &l);
    CHECK_NAPI_STATUS_RETURN_NUM(env, status, "napi_get_value_int64() failed");
    return l;
}

double napiValueToDouble(napi_env env, napi_value nv) {
    double d;
    auto status = napi_get_value_double(env, nv, &d);
    CHECK_NAPI_STATUS_RETURN_NUM(env, status, "napi_get_value_double() failed");
    return d;
}

std::tuple<const byte *, size_t> napiValueToByteArray(napi_env env, napi_value nv) {
    auto len = napiValueArrayLen(env, nv);
    if (len == 0) {
        return {};
    }
    auto byteArray = mallocX<byte>(len);
    for (decltype(len) i = 0; i < len; i++) {
        napi_value vByte;
        auto status = napi_get_element(env, nv, i, &vByte);
        CHECK_NAPI_STATUS_RETURN_TUPLE(env, status, "napi_get_element() failed");
        byteArray[i] = napiValueToInt(env, vByte);
    }
    return {byteArray, len};
}

std::tuple<const char **, size_t> napiValueToCharsArray(napi_env env, napi_value nv) {
    auto len = napiValueArrayLen(env, nv);
    if (len == 0) {
        return {};
    }
    const auto charsArray = mallocX<const char *>(len);
    for (decltype(len) i = 0; i < len; i++) {
        napi_value vChars;
        auto status = napi_get_element(env, nv, i, &vChars);
        CHECK_NAPI_STATUS_RETURN_TUPLE(env, status, "napi_get_element() failed");
        charsArray[i] = napiValueToChars(env, vChars);
    }
    return {charsArray, len};
}

napi_value napiValueFromChars(napi_env env, const char *c) {
    if (c == nullptr) {
        return napiValueFromInt(env, napi_invalid_arg);
    }
    napi_value v;
    auto status = napi_create_string_utf8(env, c, std::strlen(c), &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_string_utf8() failed");
    freeX(c);
    return v;
}

napi_value napiValueFromLong(napi_env env, long l) {
    napi_value v;
    auto status = napi_create_int64(env, l, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_int64() failed");
    return v;
}

napi_value napiValueFromInt(napi_env env, int i) {
    napi_value v;
    auto status = napi_create_int32(env, i, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_int32() failed");
    return v;
}

napi_value napiValueFromBool(napi_env env, bool b) {
    napi_value v;
    auto status = napi_get_boolean(env, b, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_boolean() failed");
    return v;
}

napi_value napiValueFromDouble(napi_env env, double d) {
    napi_value v;
    auto status = napi_create_double(env, d, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_double() failed");
    return v;
}

napi_value napiValueFromByteArray(napi_env env, const byte *byteArray, size_t len) {
    if (byteArray == nullptr || len == 0) {
        return napiValueFromInt(env, napi_invalid_arg);
    }
    napi_value v;
    auto status = napi_create_array_with_length(env, len, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_array_with_length() failed");
    for (decltype(len) i = 0; i < len; i++) {
        status = napi_set_element(env, v, i, napiValueFromInt(env, byteArray[i]));
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_set_element() failed");
    }
    freeX(byteArray);
    return v;
}

napi_value napiValueFromCharsArray(napi_env env, const char **charsArray, size_t len) {
    if (charsArray == nullptr || len == 0) {
        return napiValueFromInt(env, napi_invalid_arg);
    }
    napi_value v;
    auto status = napi_create_array_with_length(env, len, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_array_with_length() failed");
    for (decltype(len) i = 0; i < len; i++) {
        status = napi_set_element(env, v, i, napiValueFromChars(env, charsArray[i]));
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_set_element() failed");
        freeX(charsArray[i]);
    }
    freeX(charsArray);
    return v;
}

// NapiCallContext

NapiCallContext::NapiCallContext(napi_env env, napi_callback_info cbkInfo) : env(env), cbkInfo(cbkInfo) {
    auto status = napi_get_cb_info(this->env, cbkInfo, &this->argc, nullptr, nullptr, nullptr);
    this->argv = mallocX<napi_value>(this->argc);
    status = napi_get_cb_info(this->env, cbkInfo, &this->argc, this->argv, nullptr, nullptr);
}

NapiCallContext::~NapiCallContext() {
    if (this->argv != nullptr) {
        for (auto ptr : this->ptrVec) {
            freeX(ptr);
        }
        freeX(this->argv);
        this->argv = nullptr;
    }
}

napi_value NapiCallContext::argAt(size_t i) {
    if (i >= this->argCount()) [[unlikely]] {
        return this->napiValueFromInt(napi_invalid_arg);
    }
    return this->argv[i];
}

const char *NapiCallContext::argCharsAt(size_t i) {
    return this->napiValueToChars(this->argAt(i));
}

bool NapiCallContext::argBoolAt(size_t i) {
    return this->napiValueToBool(this->argAt(i));
}

int NapiCallContext::argIntAt(size_t i) {
    return this->napiValueToInt(this->argAt(i));
}

long NapiCallContext::argLongAt(size_t i) {
    return this->napiValueToLong(this->argAt(i));
}

double NapiCallContext::argDoubleAt(size_t i) {
    return this->napiValueToDouble(this->argAt(i));
}

std::tuple<const byte *, size_t> NapiCallContext::argByteArrayAt(size_t i) {
    return this->napiValueToByteArray(this->argAt(i));
}

std::tuple<const char **, size_t> NapiCallContext::argCharsArrayAt(size_t i) {
    return this->napiValueToCharsArray(this->argAt(i));
}

size_t NapiCallContext::argCount() const {
    return this->argc;
}

const char *NapiCallContext::napiValueToChars(napi_value nv) {
    auto ptr = ::napiValueToChars(this->env, nv);
    this->ptrVec.push_back((void*)ptr);
    return ptr;
}

bool NapiCallContext::napiValueToBool(napi_value nv) {
    return ::napiValueToBool(this->env, nv);
}

int NapiCallContext::napiValueToInt(napi_value nv) {
    return ::napiValueToInt(this->env, nv);
}

long NapiCallContext::napiValueToLong(napi_value nv) {
    return ::napiValueToLong(this->env, nv);
}

double NapiCallContext::napiValueToDouble(napi_value nv) {
    return ::napiValueToDouble(this->env, nv);
}

std::tuple<const byte *, size_t> NapiCallContext::napiValueToByteArray(napi_value nv) {
    auto [ptr, len] = ::napiValueToByteArray(this->env, nv);
    this->ptrVec.push_back((void*)ptr);
    return {ptr, len};
}

std::tuple<const char **, size_t> NapiCallContext::napiValueToCharsArray(napi_value nv) {
    auto [ptrV, len] = ::napiValueToCharsArray(this->env, nv);
    for (decltype(len) i = 0; i < len; i++) {
        this->ptrVec.push_back((void*)ptrV[i]);
    }
    this->ptrVec.push_back((void*)ptrV);
    return {ptrV, len};
}

napi_value NapiCallContext::napiValueFromChars(const char *c) {
    return ::napiValueFromChars(this->env, c);
}

napi_value NapiCallContext::napiValueFromLong(long l) {
    return ::napiValueFromLong(this->env, l);
}

napi_value NapiCallContext::napiValueFromInt(int i) {
    return ::napiValueFromInt(this->env, i);
}

napi_value NapiCallContext::napiValueFromBool(bool b) {
    return ::napiValueFromBool(this->env, b);
}

napi_value NapiCallContext::napiValueFromDouble(double d) {
    return ::napiValueFromDouble(this->env, d);
}

napi_value NapiCallContext::napiValueFromByteArray(const byte *byteArray, size_t len) {
    return ::napiValueFromByteArray(this->env, byteArray, len);
}

napi_value NapiCallContext::napiValueFromCharsArray(const char **charsArray, size_t len) {
    return ::napiValueFromCharsArray(this->env, charsArray, len);
}