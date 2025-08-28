#include "napi/native_api.h"

#include <cstring>

#include <napi_util.hxx>

#include "../../../../../../build.HarmonyOS/output/include/DynXX/C/DynXX.h"
#include "../../../../../../build.HarmonyOS/output/include/DynXX/CXX/Types.hxx"

namespace {

napi_env sNapiEnv;
napi_ref sTsLogCallbackRef;

napi_value getVersion(napi_env env, napi_callback_info info) {
    auto c = dynxx_get_version();
    auto v = chars2NapiValue(env, c);
    freeX(c);
    return v;
}

napi_value init(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto root = napiValue2chars(env, args.v[0]);

    auto b = dynxx_init(root);
    auto v = bool2NapiValue(env, b);

    freeX(root);
    return v;
}

napi_value release(napi_env env, napi_callback_info info) {
    dynxx_release();

    return int2NapiValue(env, napi_ok);
}

#pragma mark Log Callback

typedef struct {
    napi_async_work tsWork;
    napi_threadsafe_function tsWorkFunc;
    int logLevel;
    const char *logContent;
} TSLogWorkData;

void OnLogWorkCallTS(napi_env env, napi_value ts_callback, void *context, void *data) {
    if (env == nullptr || ts_callback == nullptr || data == nullptr) {
        return;
    }

    auto tSLogWorkData = static_cast<TSLogWorkData *>(data);

    size_t argc = 2;
    napi_value argv[2];
    argv[0] = int2NapiValue(env, tSLogWorkData->logLevel);
    argv[1] = chars2NapiValue(env, tSLogWorkData->logContent);

    napi_value vGlobal;
    auto status = napi_get_global(env, &vGlobal);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_get_global() failed");

    napi_get_reference_value(sNapiEnv, sTsLogCallbackRef, &ts_callback);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_get_reference_value() failed");

    status = napi_call_function(env, vGlobal, ts_callback, argc, argv, nullptr);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_call_function() failed");

    freeX(tSLogWorkData->logContent);
    freeX(tSLogWorkData);
}

void OnLogWorkExecute(napi_env env, void *data) {
    auto tSLogWorkData = static_cast<TSLogWorkData *>(data);

    auto status = napi_acquire_threadsafe_function(tSLogWorkData->tsWorkFunc);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_acquire_threadsafe_function() failed");

    status = napi_call_threadsafe_function(tSLogWorkData->tsWorkFunc, tSLogWorkData, napi_tsfn_blocking);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_call_threadsafe_function() failed");
}

void OnLogWorkComplete(napi_env env, napi_status status, void *data) {
    const auto tSLogWorkData = static_cast<TSLogWorkData *>(data);

    status = napi_release_threadsafe_function(tSLogWorkData->tsWorkFunc, napi_tsfn_release);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_release_threadsafe_function() failed");

    status = napi_delete_async_work(env, tSLogWorkData->tsWork);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_delete_async_work() failed");
}

void engineLogCallback(int level, const char *content) {
    if (sNapiEnv == nullptr || content == nullptr) {
        return;
    }

    auto tSLogWorkData = mallocX<TSLogWorkData>();
    tSLogWorkData->tsWork = nullptr;
    tSLogWorkData->tsWorkFunc = nullptr;
    tSLogWorkData->logLevel = level;
    tSLogWorkData->logContent = dupStr(content);
    freeX(content);

    auto vWorkName = chars2NapiValue(sNapiEnv, "NAPI_LOG_CALLBACK_WORK");

    napi_value vTsCallback;
    auto status = napi_get_reference_value(sNapiEnv, sTsLogCallbackRef, &vTsCallback);
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_get_reference_value() failed");

    status = napi_create_threadsafe_function(sNapiEnv, vTsCallback, nullptr, vWorkName, 0, 1, nullptr, nullptr, nullptr,
                                             OnLogWorkCallTS, &(tSLogWorkData->tsWorkFunc));
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_create_threadsafe_function() failed");

    status = napi_create_async_work(sNapiEnv, nullptr, vWorkName, OnLogWorkExecute, OnLogWorkComplete, tSLogWorkData,
                                    &(tSLogWorkData->tsWork));
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_create_async_work() failed");

    status = napi_queue_async_work(sNapiEnv, tSLogWorkData->tsWork);
    CHECK_NAPI_STATUS_RETURN_VOID(sNapiEnv, status, "napi_queue_async_work() failed");
}

#pragma mark Log API

napi_value logSetLevel(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto level = napiValue2int(env, args.v[0]);

    dynxx_log_set_level(level);

    return int2NapiValue(env, napi_ok);
}

napi_value logSetCallback(napi_env env, napi_callback_info info) {
    Args args(env, info);

    napi_value vLogCallback = args.c > 0 ? args.v[0] : nullptr;
    int status;
    if (vLogCallback == nullptr) {
        sNapiEnv = nullptr;

        status = napi_delete_reference(env, sTsLogCallbackRef);
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_delete_reference() failed");

        dynxx_log_set_callback(nullptr);
    } else {
        sNapiEnv = env;

        status = napi_create_reference(env, vLogCallback, 1, &sTsLogCallbackRef);
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_reference() failed");

        dynxx_log_set_callback(engineLogCallback);
    }

    return int2NapiValue(env, napi_ok);
}

napi_value logPrint(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto level = napiValue2int(env, args.v[0]);
    auto content = napiValue2chars(env, args.v[1]);

    dynxx_log_print(level, content);
    freeX(content);

    return int2NapiValue(env, napi_ok);
}

#pragma mark Net

napi_value netHttpRequest(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto cUrl = napiValue2chars(env, args.v[0]);
    auto iMethod = napiValue2int(env, args.v[1]);
    auto cParams = args.c > 2 ? napiValue2chars(env, args.v[2]) : "";

    auto header_c = args.c > 3 ? napiValueArrayLen(env, args.v[3]) : 0;
    auto header_v = args.c > 3 ? napiValue2charsArray(env, args.v[3], header_c) : nullptr;

    auto form_field_count = args.c > 4 ? napiValueArrayLen(env, args.v[4]) : 0;
    auto form_field_name_v = args.c > 4 ? napiValue2charsArray(env, args.v[4], form_field_count) : nullptr;
    auto form_field_mime_v = args.c > 5 ? napiValue2charsArray(env, args.v[5], form_field_count) : nullptr;
    auto form_field_data_v = args.c > 6 ? napiValue2charsArray(env, args.v[6], form_field_count) : nullptr;

    auto cFilePath = args.c > 7 ? napiValue2chars(env, args.v[7]) : nullptr;
    auto cFILE = cFilePath ? std::fopen(cFilePath, "r") : nullptr;
    auto fileLength = args.c > 8 ? napiValue2long(env, args.v[8]) : 0;

    auto lTimeout = args.c > 9 ? napiValue2long(env, args.v[9]) : 15000;

    auto res =
        dynxx_net_http_request(cUrl, cParams, iMethod, header_v, header_c, form_field_name_v, form_field_mime_v,
                                form_field_data_v, form_field_count, static_cast<void *>(cFILE), fileLength, lTimeout);
    auto nv = chars2NapiValue(env, res);

    freeX(res);
    for (decltype(header_c) i = 0; i < header_c; i++) {
        freeX(header_v[i]);
    }
    for (decltype(form_field_count) i = 0; i < form_field_count; i++) {
        freeX(form_field_name_v[i]);
        freeX(form_field_mime_v[i]);
        freeX(form_field_data_v[i]);
    }
    if (cFILE) {
        std::fclose(cFILE);
    }
    if (cParams) {
        freeX(cParams);
    }
    if (cUrl) {
        freeX(cUrl);
    }
    return nv;
}

#pragma mark Store.SQLite

napi_value sqliteOpen(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto _id = napiValue2chars(env, args.v[0]);
    auto res = ptr2addr(dynxx_sqlite_open(_id));
    auto nv = long2NapiValue(env, res);

    freeX(_id);
    return nv;
}

napi_value sqliteExecute(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto sql = napiValue2chars(env, args.v[1]);

    auto res = dynxx_sqlite_execute(addr2ptr(conn), sql);
    auto nv = bool2NapiValue(env, res);

    freeX(sql);
    return nv;
}

napi_value sqliteQueryDo(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto sql = napiValue2chars(env, args.v[1]);

    auto res = ptr2addr(dynxx_sqlite_query_do(addr2ptr(conn), sql));
    auto nv = long2NapiValue(env, res);

    freeX(sql);
    return nv;
}

napi_value sqliteQueryReadRow(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto query_result = napiValue2long(env, args.v[0]);

    auto res = dynxx_sqlite_query_read_row(addr2ptr(query_result));
    return bool2NapiValue(env, res);
}

napi_value sqliteQueryReadColumnText(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto query_result = napiValue2long(env, args.v[0]);
    auto column = napiValue2chars(env, args.v[1]);

    auto res = dynxx_sqlite_query_read_column_text(addr2ptr(query_result), column);
    auto nv = chars2NapiValue(env, res);

    freeX(res);
    freeX(column);
    return nv;
}

napi_value sqliteQueryReadColumnInteger(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto query_result = napiValue2long(env, args.v[0]);
    auto column = napiValue2chars(env, args.v[1]);

    auto res = dynxx_sqlite_query_read_column_integer(addr2ptr(query_result), column);
    auto nv = long2NapiValue(env, res);

    freeX(column);
    return nv;
}

napi_value sqliteQueryReadColumnFloat(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto query_result = napiValue2long(env, args.v[0]);
    auto column = napiValue2chars(env, args.v[1]);

    auto res = dynxx_sqlite_query_read_column_float(addr2ptr(query_result), column);
    auto nv = double2NapiValue(env, res);

    freeX(column);
    return nv;
}

napi_value sqliteQueryDrop(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto query_result = napiValue2long(env, args.v[0]);
    dynxx_sqlite_query_drop(addr2ptr(query_result));

    return int2NapiValue(env, napi_ok);
}

napi_value sqliteClose(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    dynxx_sqlite_close(addr2ptr(conn));

    return int2NapiValue(env, napi_ok);
}

#pragma mark Store.KV

napi_value kvOpen(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto _id = napiValue2chars(env, args.v[0]);
    auto res = ptr2addr(dynxx_kv_open(_id));
    auto nv = long2NapiValue(env, res);

    freeX(_id);
    return nv;
}

napi_value kvReadString(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);

    auto res = dynxx_kv_read_string(addr2ptr(conn), k);
    auto nv = chars2NapiValue(env, res);

    freeX(res);
    freeX(k);
    return nv;
}

napi_value kvWriteString(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);
    auto v = napiValue2chars(env, args.v[2]);

    auto res = dynxx_kv_write_string(addr2ptr(conn), k, v);
    auto nv = bool2NapiValue(env, res);

    freeX(v);
    freeX(k);
    return nv;
}

napi_value kvReadInteger(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);

    auto res = dynxx_kv_read_integer(addr2ptr(conn), k);
    auto nv = long2NapiValue(env, res);

    freeX(k);
    return nv;
}

napi_value kvWriteInteger(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);
    auto v = napiValue2long(env, args.v[2]);

    auto res = dynxx_kv_write_integer(addr2ptr(conn), k, v);
    auto nv = bool2NapiValue(env, res);

    freeX(k);
    return nv;
}

napi_value kvReadFloat(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);

    auto res = dynxx_kv_read_integer(addr2ptr(conn), k);
    auto nv = double2NapiValue(env, res);

    freeX(k);
    return nv;
}

napi_value kvWriteFloat(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);
    auto v = napiValue2double(env, args.v[2]);

    auto res = dynxx_kv_write_integer(addr2ptr(conn), k, v);
    auto nv = bool2NapiValue(env, res);

    freeX(k);
    return nv;
}

napi_value kvContains(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);

    auto res = dynxx_kv_contains(addr2ptr(conn), k);
    auto nv = bool2NapiValue(env, res);

    freeX(k);
    return nv;
}

napi_value kvRemove(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);

    auto res = dynxx_kv_remove(addr2ptr(conn), k);
    auto nv = bool2NapiValue(env, res);

    freeX(k);
    return nv;
}

napi_value kvClear(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    dynxx_kv_clear(addr2ptr(conn));

    return int2NapiValue(env, napi_ok);
}

napi_value kvClose(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    dynxx_kv_close(addr2ptr(conn));

    return int2NapiValue(env, napi_ok);
}

#pragma mark DeviceInfo

napi_value deviceType(napi_env env, napi_callback_info info) {
    auto dt = dynxx_device_type();
    auto v = int2NapiValue(env, dt);
    return v;
}

napi_value deviceName(napi_env env, napi_callback_info info) {
    auto cDN = dynxx_device_name();
    auto v = chars2NapiValue(env, cDN);
    freeX(cDN);

    return v;
}

napi_value deviceManufacturer(napi_env env, napi_callback_info info) {
    auto cDM = dynxx_device_manufacturer();
    auto v = chars2NapiValue(env, cDM);
    freeX(cDM);
    return v;
}

napi_value deviceOsVersion(napi_env env, napi_callback_info info) {
    auto cOV = dynxx_device_name();
    auto v = chars2NapiValue(env, cOV);
    freeX(cOV);
    return v;
}

napi_value deviceCpuArch(napi_env env, napi_callback_info info) {
    auto dca = dynxx_device_cpu_arch();
    auto v = int2NapiValue(env, dca);
    return v;
}

#pragma mark JsonDecoder

napi_value jsonReadType(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto node = args.c > 1 ? napiValue2long(env, args.v[0]) : 0;

    auto res = dynxx_json_read_type(addr2ptr(node));
    return int2NapiValue(env, res);
}

napi_value jsonDecoderInit(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto cJson = napiValue2chars(env, args.v[0]);

    auto res = ptr2addr(dynxx_json_decoder_init(cJson));
    auto v = long2NapiValue(env, res);

    freeX(cJson);
    return v;
}

napi_value jsonDecoderReadNode(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto decoder = napiValue2long(env, args.v[0]);
    auto cK = napiValue2chars(env, args.v[1]);
    auto node = args.c > 2 ? napiValue2long(env, args.v[2]) : 0;

    auto res = ptr2addr(dynxx_json_decoder_read_node(addr2ptr(decoder), addr2ptr(node), cK));
    auto v = long2NapiValue(env, res);

    freeX(cK);
    return v;
}

napi_value jsonDecoderReadChild(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto decoder = napiValue2long(env, args.v[0]);
    auto node = args.c > 1 ? napiValue2long(env, args.v[1]) : 0;

    auto res = ptr2addr(dynxx_json_decoder_read_child(addr2ptr(decoder), addr2ptr(node)));
    return long2NapiValue(env, res);
}

napi_value jsonDecoderReadNext(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto decoder = napiValue2long(env, args.v[0]);
    auto node = args.c > 1 ? napiValue2long(env, args.v[1]) : 0;

    auto res = ptr2addr(dynxx_json_decoder_read_next(addr2ptr(decoder), addr2ptr(node)));
    return long2NapiValue(env, res);
}

napi_value jsonDecoderReadString(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto decoder = napiValue2long(env, args.v[0]);
    auto node = args.c > 1 ? napiValue2long(env, args.v[1]) : 0;

    auto cRes = dynxx_json_decoder_read_string(addr2ptr(decoder), addr2ptr(node));
    auto v = chars2NapiValue(env, cRes);

    freeX(cRes);
    return v;
}

napi_value jsonDecoderReadNumber(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto decoder = napiValue2long(env, args.v[0]);
    auto node = args.c > 1 ? napiValue2long(env, args.v[1]) : 0;

    auto res = dynxx_json_decoder_read_number(addr2ptr(decoder), addr2ptr(node));
    return double2NapiValue(env, res);
}

napi_value jsonDecoderRelease(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto decoder = napiValue2long(env, args.v[0]);

    dynxx_json_decoder_release(addr2ptr(decoder));

    return int2NapiValue(env, napi_ok);
}

#pragma mark Coding

napi_value codingHexBytes2str(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);

    auto cRes = dynxx_coding_hex_bytes2str(inBytes, inLen);
    auto v = chars2NapiValue(env, cRes);

    freeX(cRes);
    freeX(inBytes);
    return v;
}

napi_value codingHexStr2Bytes(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto cStr = napiValue2chars(env, args.v[0]);

    size_t outLen;
    auto cRes = dynxx_coding_hex_str2bytes(cStr, &outLen);
    auto v = byteArray2NapiValue(env, cRes, outLen);

    freeX(cRes);
    freeX(cStr);
    return v;
}

#pragma mark Crypto

napi_value cryptoRand(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto len = napiValue2int(env, args.v[0]);

    auto outBytes = dynxx_crypto_rand(len);
    auto v = byteArray2NapiValue(env, outBytes, len);
    freeX(outBytes);

    return v;
}

napi_value cryptoAesEncrypt(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);
    auto keyLen = napiValueArrayLen(env, args.v[1]);
    auto keyBytes = napiValue2byteArray(env, args.v[1], keyLen);

    size_t outLen;
    auto outBytes = dynxx_crypto_aes_encrypt(inBytes, inLen, keyBytes, keyLen, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    freeX(keyBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoAesDecrypt(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);
    auto keyLen = napiValueArrayLen(env, args.v[1]);
    auto keyBytes = napiValue2byteArray(env, args.v[1], keyLen);

    size_t outLen;
    auto outBytes = dynxx_crypto_aes_decrypt(inBytes, inLen, keyBytes, keyLen, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    freeX(keyBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoAesGcmEncrypt(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);
    auto keyLen = napiValueArrayLen(env, args.v[1]);
    auto keyBytes = napiValue2byteArray(env, args.v[1], keyLen);
    auto initVectorLen = napiValueArrayLen(env, args.v[2]);
    auto initVectorBytes = napiValue2byteArray(env, args.v[2], initVectorLen);
    auto tagBits = napiValue2int(env, args.v[3]);
    auto aadLen = args.c > 4 ? napiValueArrayLen(env, args.v[4]) : 0;
    auto aadBytes = args.c > 4 ? napiValue2byteArray(env, args.v[4], aadLen) : nullptr;

    size_t outLen;
    auto outBytes = dynxx_crypto_aes_gcm_encrypt(inBytes, inLen, keyBytes, keyLen, initVectorBytes, initVectorLen,
                                                  aadBytes, aadLen, tagBits, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    if (aadBytes) {
        freeX(aadBytes);
    }
    freeX(initVectorBytes);
    freeX(keyBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoAesGcmDecrypt(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);
    auto keyLen = napiValueArrayLen(env, args.v[1]);
    auto keyBytes = napiValue2byteArray(env, args.v[1], keyLen);
    auto initVectorLen = napiValueArrayLen(env, args.v[2]);
    auto initVectorBytes = napiValue2byteArray(env, args.v[2], initVectorLen);
    auto tagBits = napiValue2int(env, args.v[3]);
    auto aadLen = args.c > 4 ? napiValueArrayLen(env, args.v[4]) : 0;
    auto aadBytes = args.c > 4 ? napiValue2byteArray(env, args.v[4], aadLen) : nullptr;

    size_t outLen;
    auto outBytes = dynxx_crypto_aes_gcm_decrypt(inBytes, inLen, keyBytes, keyLen, initVectorBytes, initVectorLen,
                                                  aadBytes, aadLen, tagBits, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    if (aadBytes) {
        freeX(aadBytes);
    }
    freeX(initVectorBytes);
    freeX(keyBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoHashMd5(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);

    size_t outLen;
    auto outBytes = dynxx_crypto_hash_md5(inBytes, inLen, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoHashSha1(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);

    size_t outLen;
    auto outBytes = dynxx_crypto_hash_sha1(inBytes, inLen, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoHashSha256(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);

    size_t outLen;
    auto outBytes = dynxx_crypto_hash_sha256(inBytes, inLen, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoBase64Encode(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);
    auto noNewLines = napiValue2bool(env, args.v[1]);

    size_t outLen;
    auto outBytes = dynxx_crypto_base64_encode(inBytes, inLen, noNewLines, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoBase64Decode(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);
    auto noNewLines = napiValue2bool(env, args.v[1]);

    size_t outLen;
    auto outBytes = dynxx_crypto_base64_decode(inBytes, inLen, noNewLines, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

#pragma mark Lua

napi_value lLoadF(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto file = napiValue2chars(env, args.v[0]);

    auto res = dynxx_lua_loadF(file);
    auto nv = bool2NapiValue(env, res);

    freeX(file);
    return nv;
}

napi_value lLoadS(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto script = napiValue2chars(env, args.v[0]);

    auto res = dynxx_lua_loadS(script);
    auto nv = bool2NapiValue(env, res);

    freeX(script);
    return nv;
}

napi_value lCall(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto func = napiValue2chars(env, args.v[0]);
    auto params = napiValue2chars(env, args.v[1]);

    auto res = dynxx_lua_call(func, params);
    auto nv = chars2NapiValue(env, res);

    freeX(res);
    if (params) {
        freeX(params);
    }
    freeX(func);
    return nv;
}

#pragma mark JS

napi_value jLoadF(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto file = napiValue2chars(env, args.v[0]);
    auto isModule = napiValue2bool(env, args.v[1]);

    auto res = dynxx_js_loadF(file, isModule);
    auto nv = bool2NapiValue(env, res);

    freeX(file);
    return nv;
}

napi_value jLoadS(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto script = napiValue2chars(env, args.v[0]);
    auto name = napiValue2chars(env, args.v[1]);
    auto isModule = napiValue2bool(env, args.v[2]);

    auto res = dynxx_js_loadS(script, name, isModule);
    auto nv = bool2NapiValue(env, res);

    freeX(script);
    freeX(name);
    return nv;
}

napi_value jLoadB(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);
    auto isModule = napiValue2bool(env, args.v[1]);

    auto b = dynxx_js_loadB(inBytes, inLen, isModule);
    auto v = bool2NapiValue(env, b);

    freeX(inBytes);
    return v;
}

napi_value jCall(napi_env env, napi_callback_info info) {
    Args args(env, info);

    auto func = napiValue2chars(env, args.v[0]);
    auto params = napiValue2chars(env, args.v[1]);
    auto await = napiValue2bool(env, args.v[2]);

    auto res = dynxx_js_call(func, params, await);
    auto nv = chars2NapiValue(env, res);

    freeX(res);
    if (params) {
        freeX(params);
    }
    freeX(func);
    return nv;
}

#define NAPI(f)                                                                                                        \
    { #f, NULL, f, NULL, NULL, NULL, napi_default, NULL }

} // namespace

#pragma mark Register Module

EXTERN_C_START
napi_value NAPI_DynXX_RegisterFuncs(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        NAPI(getVersion),
        NAPI(init),
        NAPI(release),

        NAPI(logSetLevel),
        NAPI(logSetCallback),
        NAPI(logPrint),

        NAPI(netHttpRequest),

        NAPI(lLoadF),
        NAPI(lLoadS),
        NAPI(lCall),

        NAPI(jLoadF),
        NAPI(jLoadS),
        NAPI(jLoadB),
        NAPI(jCall),

        NAPI(sqliteOpen),
        NAPI(sqliteExecute),
        NAPI(sqliteQueryDo),
        NAPI(sqliteQueryReadRow),
        NAPI(sqliteQueryReadColumnText),
        NAPI(sqliteQueryReadColumnInteger),
        NAPI(sqliteQueryReadColumnFloat),
        NAPI(sqliteQueryDrop),
        NAPI(sqliteClose),

        NAPI(kvOpen),
        NAPI(kvReadString),
        NAPI(kvWriteString),
        NAPI(kvReadInteger),
        NAPI(kvWriteInteger),
        NAPI(kvReadFloat),
        NAPI(kvWriteFloat),
        NAPI(kvContains),
        NAPI(kvRemove),
        NAPI(kvClear),
        NAPI(kvClose),

        NAPI(deviceType),
        NAPI(deviceName),
        NAPI(deviceManufacturer),
        NAPI(deviceOsVersion),
        NAPI(deviceCpuArch),

        NAPI(jsonReadType),
        NAPI(jsonDecoderInit),
        NAPI(jsonDecoderReadNode),
        NAPI(jsonDecoderReadChild),
        NAPI(jsonDecoderReadNext),
        NAPI(jsonDecoderReadString),
        NAPI(jsonDecoderReadNumber),
        NAPI(jsonDecoderRelease),

        NAPI(codingHexBytes2str),
        NAPI(codingHexStr2Bytes),

        NAPI(cryptoRand),
        NAPI(cryptoAesEncrypt),
        NAPI(cryptoAesDecrypt),
        NAPI(cryptoAesGcmEncrypt),
        NAPI(cryptoAesGcmDecrypt),
        NAPI(cryptoHashMd5),
        NAPI(cryptoHashSha1),
        NAPI(cryptoHashSha256),
        NAPI(cryptoBase64Encode),
        NAPI(cryptoBase64Decode),
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module dynxxModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = NULL,
    .nm_register_func = NAPI_DynXX_RegisterFuncs,
    .nm_modname = "dynxx",
    .nm_priv = ((void *)(0)),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterDynXXModule(void) { napi_module_register(&dynxxModule); }
