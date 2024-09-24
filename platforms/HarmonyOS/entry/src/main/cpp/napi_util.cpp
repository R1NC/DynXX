#include "napi_util.h"
#include <stdlib.h>
#include <string.h>

napi_value *readParams(napi_env env, napi_callback_info info, size_t count) {
    size_t argc = count;
    napi_value *argv = (napi_value *)malloc(sizeof(napi_value) * count + 1);
    memset((void *)argv, 0, count + 1);
    int status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_cb_info() get length failed");
    return argv;
}

const unsigned int napiValueLen(napi_env env, napi_value nv) {
    uint32_t len;
    int status = napi_get_array_length(env, nv, &len);
    CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_array_length() get length failed");
    return len;
}

const char *napiValue2chars(napi_env env, napi_value nv) {
    napi_status status;
    size_t len;
    status = napi_get_value_string_utf8(env, nv, NULL, 0, &len);
    CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_value_string_utf8() get length failed");
    char *cStr = (char *)malloc(len + 1);
    status = napi_get_value_string_utf8(env, nv, cStr, len + 1, &len);
    CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_value_string_utf8() get content failed");
    return cStr;
}

bool napiValue2bool(napi_env env, napi_value nv) {
    bool b;
    napi_status status = napi_get_value_bool(env, nv, &b);
    CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_value_bool() get content failed");
    return b;
}

int napiValue2int(napi_env env, napi_value nv) {
    int i;
    napi_status status = napi_get_value_int32(env, nv, &i);
    CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_value_int32() get content failed");
    return i;
}

long napiValue2long(napi_env env, napi_value nv) {
    long l;
    napi_status status = napi_get_value_int64(env, nv, &l);
    CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_value_int64() get content failed");
    return l;
}

double napiValue2double(napi_env env, napi_value nv) {
    double d;
    napi_status status = napi_get_value_double(env, nv, &d);
    CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_value_double() get content failed");
    return d;
}

const unsigned char *napiValue2byteArray(napi_env env, napi_value nv, unsigned long len) {
    unsigned char *byteArray = (unsigned char *)malloc(len * sizeof(const unsigned char) + 1);
    int status;
    for (int i = 0; i < len; i++) {
        napi_value vByte;
        status = napi_get_element(env, nv, i, &vByte);
        CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_element() failed");
        byteArray[i] = napiValue2int(env, vByte);
    }
    return byteArray;
}

const char **napiValue2charsArray(napi_env env, napi_value nv, unsigned long len) {
    const char **charsArray = (const char **)malloc(len * sizeof(char *) + 1);
    int status;
    for (int i = 0; i < len; i++) {
        napi_value vChars;
        status = napi_get_element(env, nv, i, &vChars);
        CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napi_get_element() failed");
        charsArray[i] = napiValue2chars(env, vChars);
    }
    return charsArray;
}

napi_value chars2NapiValue(napi_env env, const char *c) {
    if (c == NULL)
        c = "";
    napi_value v;
    napi_status status = napi_create_string_utf8(env, c, strlen(c), &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_string_utf8() failed");
    return v;
}

napi_value long2NapiValue(napi_env env, long l) {
    napi_value v;
    napi_status status = napi_create_int64(env, l, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_int64() failed");
    return v;
}

napi_value int2NapiValue(napi_env env, int i) {
    napi_value v;
    napi_status status = napi_create_int32(env, i, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_int32() failed");
    return v;
}

napi_value bool2NapiValue(napi_env env, bool b) {
    napi_value v;
    int status = napi_get_boolean(env, b, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_get_boolean() failed");
    return v;
}

napi_value double2NapiValue(napi_env env, double d) {
    napi_value v;
    napi_status status = napi_create_double(env, d, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_double() failed");
    return v;
}

napi_value byteArray2NapiValue(napi_env env, const unsigned char *byteArray, unsigned long len) {
    napi_value v;
    int status = napi_create_array_with_length(env, len, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_array_with_length() failed");
    for (int i = 0; i < len; i++) {
        status = napi_set_element(env, v, i, int2NapiValue(env, byteArray[i]));
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_set_element() failed");
    }
    return v;
}

napi_value charsArray2NapiValue(napi_env env, const char **charsArray, unsigned long len) {
    napi_value v;
    int status = napi_create_array_with_length(env, len, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_array_with_length() failed");
    for (int i = 0; i < len; i++) {
        status = napi_set_element(env, v, i, chars2NapiValue(env, charsArray[i]));
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_set_element() failed");
    }
    return v;
}