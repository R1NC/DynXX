#include "napi_util.h"
#include <stdlib.h>
#include <string.h>

const char *napiValue2char(napi_env env, napi_value nv) {
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
    CHECK_NAPI_STATUS_RETURN_ANY(env, status, "napiValue2double() get content failed");
    return d;
}

napi_value char2NapiValue(napi_env env, const char *c) {
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
    napi_get_boolean(env, b, &v);
    return v;
}

napi_value double2NapiValue(napi_env env, double d) {
    napi_value v;
    napi_status status = napi_create_double(env, d, &v);
    CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_double() failed");
    return v;
}