#include "napi/native_api.h"

#include <cstring>

#include <napi_util.hxx>

#include "../../../../../../build.HarmonyOS/output/include/DynXX/C/DynXX.h"
#include "../../../../../../build.HarmonyOS/output/include/DynXX/CXX/Memory.hxx"

namespace {

napi_env sNapiEnv;
napi_ref sTsLogCallbackRef;

napi_value getVersion(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);
    
    auto c = dynxx_get_version();
    auto v = ctx.napiValueFromChars(c);
    freeX(c);
    return v;
}

napi_value init(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto root = ctx.argCharsAt(0);

    auto b = dynxx_init(root);
    auto v = ctx.napiValueFromBool(b);

    freeX(root);
    return v;
}

napi_value release(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);
    
    dynxx_release();

    return ctx.napiValueFromInt(napi_ok);
}

// Log Callback

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
    argv[0] = napiValueFromInt(env, tSLogWorkData->logLevel);
    argv[1] = napiValueFromChars(env, tSLogWorkData->logContent);

    napi_value vGlobal;
    auto status = napi_get_global(env, &vGlobal);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_get_global() failed");

    napi_get_reference_value(env, sTsLogCallbackRef, &ts_callback);
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

    auto vWorkName = napiValueFromChars(sNapiEnv, "NAPI_LOG_CALLBACK_WORK");

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

// Log API

napi_value logSetLevel(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    const auto level = ctx.argIntAt(0);

    dynxx_log_set_level(static_cast<DynXXLogLevel>(level));

    return ctx.napiValueFromInt(napi_ok);
}

napi_value logSetCallback(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    napi_value vLogCallback = ctx.argCount()> 0 ? ctx.argAt(0) : nullptr;
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

    return ctx.napiValueFromInt(napi_ok);
}

napi_value logPrint(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    const auto level = ctx.argIntAt(0);
    auto content = ctx.argCharsAt(1);

    dynxx_log_print(static_cast<DynXXLogLevel>(level), content);
    freeX(content);

    return ctx.napiValueFromInt(napi_ok);
}

// Net

napi_value netHttpRequest(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto cUrl = ctx.argCharsAt(0);
    const auto iMethod = ctx.argIntAt(1);
    auto cParams = ctx.argCount()> 2 ? ctx.argCharsAt(2) : "";

    auto [header_v, header_c] = ctx.argCharsArrayAt(3);

    auto [form_field_name_v, form_field_count] = ctx.argCharsArrayAt(4);
    auto [form_field_mime_v, form_field_mime_count] = ctx.argCharsArrayAt(5);
    auto [form_field_data_v, form_field_data_count] = ctx.argCharsArrayAt(6);

    auto cFilePath = ctx.argCount()> 7 ? ctx.argCharsAt(7) : nullptr;
    auto cFILE = cFilePath ? std::fopen(cFilePath, "r") : nullptr;
    auto fileLength = ctx.argCount()> 8 ? ctx.argLongAt(8) : 0;

    auto lTimeout = ctx.argCount()> 9 ? ctx.argLongAt(9) : 15000;

    auto res =
        dynxx_net_http_request(cUrl, cParams, static_cast<DynXXHttpMethod>(iMethod), header_v, header_c, form_field_name_v, form_field_mime_v,
                                form_field_data_v, form_field_count, cFILE, fileLength, lTimeout);
    auto nv = ctx.napiValueFromChars(res);

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

// SQLite

napi_value sqliteOpen(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto _id = ctx.argCharsAt(0);
    auto res = dynxx_sqlite_open(_id);
    auto nv = ctx.napiValueFromLong(res);

    freeX(_id);
    return nv;
}

napi_value sqliteExecute(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto conn = ctx.argLongAt(0);
    auto sql = ctx.argCharsAt(1);

    auto res = dynxx_sqlite_execute(conn, sql);
    auto nv = ctx.napiValueFromBool(res);

    freeX(sql);
    return nv;
}

napi_value sqliteQueryDo(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto conn = ctx.argLongAt(0);
    auto sql = ctx.argCharsAt(1);

    auto res = dynxx_sqlite_query_do(conn, sql);
    auto nv = ctx.napiValueFromLong(res);

    freeX(sql);
    return nv;
}

napi_value sqliteQueryReadRow(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto query_result = ctx.argLongAt(0);

    auto res = dynxx_sqlite_query_read_row(query_result);
    return ctx.napiValueFromBool(res);
}

napi_value sqliteQueryReadColumnText(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto query_result = ctx.argLongAt(0);
    auto column = ctx.argCharsAt(1);

    auto res = dynxx_sqlite_query_read_column_text(query_result, column);
    auto nv = ctx.napiValueFromChars(res);

    freeX(res);
    freeX(column);
    return nv;
}

napi_value sqliteQueryReadColumnInteger(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto query_result = ctx.argLongAt(0);
    auto column = ctx.argCharsAt(1);

    auto res = dynxx_sqlite_query_read_column_integer(query_result, column);
    auto nv = ctx.napiValueFromLong(res);

    freeX(column);
    return nv;
}

napi_value sqliteQueryReadColumnFloat(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto query_result = ctx.argLongAt(0);
    auto column = ctx.argCharsAt(1);

    auto res = dynxx_sqlite_query_read_column_float(query_result, column);
    auto nv = ctx.napiValueFromDouble(res);

    freeX(column);
    return nv;
}

napi_value sqliteQueryDrop(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto query_result = ctx.argLongAt(0);
    dynxx_sqlite_query_drop(query_result);

    return ctx.napiValueFromInt(napi_ok);
}

napi_value sqliteClose(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto conn = ctx.argLongAt(0);
    dynxx_sqlite_close(conn);

    return ctx.napiValueFromInt(napi_ok);
}

// KV

napi_value kvOpen(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto _id = ctx.argCharsAt(0);
    auto res = dynxx_kv_open(_id);
    auto nv = ctx.napiValueFromLong(res);

    freeX(_id);
    return nv;
}

napi_value kvReadString(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto conn = ctx.argLongAt(0);
    auto k = ctx.argCharsAt(1);

    auto res = dynxx_kv_read_string(conn, k);
    auto nv = ctx.napiValueFromChars(res);

    freeX(res);
    freeX(k);
    return nv;
}

napi_value kvWriteString(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto conn = ctx.argLongAt(0);
    auto k = ctx.argCharsAt(1);
    auto v = ctx.argCharsAt(2);

    auto res = dynxx_kv_write_string(conn, k, v);
    auto nv = ctx.napiValueFromBool(res);

    freeX(v);
    freeX(k);
    return nv;
}

napi_value kvReadInteger(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto conn = ctx.argLongAt(0);
    auto k = ctx.argCharsAt(1);

    auto res = dynxx_kv_read_integer(conn, k);
    auto nv = ctx.napiValueFromLong(res);

    freeX(k);
    return nv;
}

napi_value kvWriteInteger(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto conn = ctx.argLongAt(0);
    auto k = ctx.argCharsAt(1);
    auto v = ctx.argLongAt(2);

    auto res = dynxx_kv_write_integer(conn, k, v);
    auto nv = ctx.napiValueFromBool(res);

    freeX(k);
    return nv;
}

napi_value kvReadFloat(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto conn = ctx.argLongAt(0);
    auto k = ctx.argCharsAt(1);

    auto res = dynxx_kv_read_float(conn, k);
    auto nv = ctx.napiValueFromDouble(res);

    freeX(k);
    return nv;
}

napi_value kvWriteFloat(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto conn = ctx.argLongAt(0);
    auto k = ctx.argCharsAt(1);
    auto v = ctx.argDoubleAt(2);

    auto res = dynxx_kv_write_float(conn, k, v);
    auto nv = ctx.napiValueFromBool(res);

    freeX(k);
    return nv;
}

napi_value kvAllKeys(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto conn = ctx.argLongAt(0);

    size_t len;
    auto cRes = dynxx_kv_all_keys(conn, &len);
    auto nv = ctx.napiValueFromCharsArray((const char**)cRes, len);

    for (size_t i = 0; i < len; i++) {
        freeX(cRes[i]);
    }
    freeX(cRes);

    return nv;
}

napi_value kvContains(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto conn = ctx.argLongAt(0);
    auto k = ctx.argCharsAt(1);

    auto res = dynxx_kv_contains(conn, k);
    auto nv = ctx.napiValueFromBool(res);

    freeX(k);
    return nv;
}

napi_value kvRemove(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto conn = ctx.argLongAt(0);
    auto k = ctx.argCharsAt(1);

    auto res = dynxx_kv_remove(conn, k);
    auto nv = ctx.napiValueFromBool(res);

    freeX(k);
    return nv;
}

napi_value kvClear(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto conn = ctx.argLongAt(0);
    dynxx_kv_clear(conn);

    return ctx.napiValueFromInt(napi_ok);
}

napi_value kvClose(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto conn = ctx.argLongAt(0);
    dynxx_kv_close(conn);

    return ctx.napiValueFromInt(napi_ok);
}

// DeviceInfo

napi_value deviceType(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);
    
    auto dt = dynxx_device_type();
    auto v = ctx.napiValueFromInt(dt);
    return v;
}

napi_value deviceName(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);
    
    auto cDN = dynxx_device_name();
    auto v = ctx.napiValueFromChars(cDN);
    freeX(cDN);

    return v;
}

napi_value deviceManufacturer(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);
    
    auto cDM = dynxx_device_manufacturer();
    auto v = ctx.napiValueFromChars(cDM);
    freeX(cDM);
    return v;
}

napi_value deviceOsVersion(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);
    
    auto cOV = dynxx_device_name();
    auto v = ctx.napiValueFromChars(cOV);
    freeX(cOV);
    return v;
}

napi_value deviceCpuArch(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);
    
    auto dca = dynxx_device_cpu_arch();
    auto v = ctx.napiValueFromInt(dca);
    return v;
}

// JsonDecoder

napi_value jsonReadType(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto node = ctx.argCount()> 1 ? ctx.argLongAt(0) : 0;

    auto res = dynxx_json_node_read_type(node);
    return ctx.napiValueFromInt(res);
}

napi_value jsonDecoderInit(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto cJson = ctx.argCharsAt(0);

    auto res = dynxx_json_decoder_init(cJson);
    auto v = ctx.napiValueFromLong(res);

    freeX(cJson);
    return v;
}

napi_value jsonDecoderReadNode(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto decoder = ctx.argLongAt(0);
    auto cK = ctx.argCharsAt(1);
    auto node = ctx.argCount()> 2 ? ctx.argLongAt(2) : 0;

    auto res = dynxx_json_decoder_read_node(decoder, node, cK);
    auto v = ctx.napiValueFromLong(res);

    freeX(cK);
    return v;
}

napi_value jsonDecoderReadChild(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto decoder = ctx.argLongAt(0);
    auto node = ctx.argCount()> 1 ? ctx.argLongAt(1) : 0;

    auto res = dynxx_json_decoder_read_child(decoder, node);
    return ctx.napiValueFromLong(res);
}

napi_value jsonDecoderReadNext(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto decoder = ctx.argLongAt(0);
    auto node = ctx.argCount()> 1 ? ctx.argLongAt(1) : 0;

    auto res = dynxx_json_decoder_read_next(decoder, node);
    return ctx.napiValueFromLong(res);
}

napi_value jsonDecoderReadString(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto decoder = ctx.argLongAt(0);
    auto node = ctx.argCount()> 1 ? ctx.argLongAt(1) : 0;

    auto cRes = dynxx_json_decoder_read_string(decoder, node);
    auto v = ctx.napiValueFromChars(cRes);

    freeX(cRes);
    return v;
}

napi_value jsonDecoderReadInteger(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto decoder = ctx.argLongAt(0);
    auto node = ctx.argCount()> 1 ? ctx.argLongAt(1) : 0;

    auto res = dynxx_json_decoder_read_integer(decoder, node);
    return ctx.napiValueFromLong(res);
}

napi_value jsonDecoderReadFloat(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto decoder = ctx.argLongAt(0);
    auto node = ctx.argCount()> 1 ? ctx.argLongAt(1) : 0;

    auto res = dynxx_json_decoder_read_float(decoder, node);
    return ctx.napiValueFromDouble(res);
}

napi_value jsonDecoderRelease(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto decoder = ctx.argLongAt(0);

    dynxx_json_decoder_release(decoder);

    return ctx.napiValueFromInt(napi_ok);
}

// Coding

napi_value codingHexBytes2str(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto [inBytes, inLen] = ctx.argByteArrayAt(0);

    auto cRes = dynxx_coding_hex_bytes2str(inBytes, inLen);
    auto v = ctx.napiValueFromChars(cRes);

    freeX(cRes);
    freeX(inBytes);
    return v;
}

napi_value codingHexStr2Bytes(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto cStr = ctx.argCharsAt(0);

    size_t outLen;
    auto cRes = dynxx_coding_hex_str2bytes(cStr, &outLen);
    auto v = ctx.napiValueFromByteArray(cRes, outLen);

    freeX(cRes);
    freeX(cStr);
    return v;
}

// Crypto

napi_value cryptoRand(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto len = ctx.argIntAt(0);

    auto outBytes = dynxx_crypto_rand(len);
    auto v = ctx.napiValueFromByteArray(outBytes, len);
    freeX(outBytes);

    return v;
}

napi_value cryptoAesEncrypt(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto [inBytes, inLen] = ctx.argByteArrayAt(0);
    auto [keyBytes, keyLen] = ctx.argByteArrayAt(1);

    size_t outLen;
    auto outBytes = dynxx_crypto_aes_encrypt(inBytes, inLen, keyBytes, keyLen, &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

    freeX(outBytes);
    freeX(keyBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoAesDecrypt(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto [inBytes, inLen] = ctx.argByteArrayAt(0);
    auto [keyBytes, keyLen] = ctx.argByteArrayAt(1);

    size_t outLen;
    auto outBytes = dynxx_crypto_aes_decrypt(inBytes, inLen, keyBytes, keyLen, &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

    freeX(outBytes);
    freeX(keyBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoAesGcmEncrypt(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto [inBytes, inLen] = ctx.argByteArrayAt(0);
    auto [keyBytes, keyLen] = ctx.argByteArrayAt(1);
    auto [initVectorBytes, initVectorLen] = ctx.argByteArrayAt(2);
    auto tagBits = ctx.argIntAt(3);
    auto [aadBytes, aadLen] = ctx.argByteArrayAt(4);

    size_t outLen;
    auto outBytes = dynxx_crypto_aes_gcm_encrypt(inBytes, inLen, keyBytes, keyLen, initVectorBytes, initVectorLen,
                                                  aadBytes, aadLen, tagBits, &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

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
    NapiCallContext ctx(env, info);

    auto [inBytes, inLen] = ctx.argByteArrayAt(0);
    auto [keyBytes, keyLen] = ctx.argByteArrayAt(1);
    auto [initVectorBytes, initVectorLen] = ctx.argByteArrayAt(2);
    auto tagBits = ctx.argIntAt(3);
    auto [aadBytes, aadLen] = ctx.argByteArrayAt(4);

    size_t outLen;
    auto outBytes = dynxx_crypto_aes_gcm_decrypt(inBytes, inLen, keyBytes, keyLen, initVectorBytes, initVectorLen,
                                                  aadBytes, aadLen, tagBits, &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

    freeX(outBytes);
    if (aadBytes) {
        freeX(aadBytes);
    }
    freeX(initVectorBytes);
    freeX(keyBytes);
    freeX(inBytes);
    return v;
}

// Crypto RSA

napi_value cryptoRsaGenKey(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto base64 = ctx.argCharsAt(0);
    auto is_public = ctx.argBoolAt(1);

    auto cRes = dynxx_crypto_rsa_gen_key(base64, is_public);
    auto v = ctx.napiValueFromChars(cRes);

    freeX(cRes);
    freeX(base64);
    return v;
}

napi_value cryptoRsaEncrypt(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto [inBytes, inLen] = ctx.argByteArrayAt(0);
    auto [keyBytes, keyLen] = ctx.argByteArrayAt(1);
    auto padding = ctx.argIntAt(2);

    size_t outLen;
    auto outBytes = dynxx_crypto_rsa_encrypt(inBytes, inLen, keyBytes, keyLen, static_cast<DynXXCryptoRSAPadding>(padding), &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

    freeX(outBytes);
    freeX(keyBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoRsaDecrypt(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto [inBytes, inLen] = ctx.argByteArrayAt(0);
    auto [keyBytes, keyLen] = ctx.argByteArrayAt(1);
    auto padding = ctx.argIntAt(2);

    size_t outLen;
    auto outBytes = dynxx_crypto_rsa_decrypt(inBytes, inLen, keyBytes, keyLen, static_cast<DynXXCryptoRSAPadding>(padding), &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

    freeX(outBytes);
    freeX(keyBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoHashMd5(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto [inBytes, inLen] = ctx.argByteArrayAt(0);

    size_t outLen;
    auto outBytes = dynxx_crypto_hash_md5(inBytes, inLen, &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoHashSha1(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto [inBytes, inLen] = ctx.argByteArrayAt(0);

    size_t outLen;
    auto outBytes = dynxx_crypto_hash_sha1(inBytes, inLen, &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoHashSha256(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto [inBytes, inLen] = ctx.argByteArrayAt(0);

    size_t outLen;
    auto outBytes = dynxx_crypto_hash_sha256(inBytes, inLen, &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoBase64Encode(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto [inBytes, inLen] = ctx.argByteArrayAt(0);
    auto noNewLines = ctx.argBoolAt(1);

    size_t outLen;
    auto outBytes = dynxx_crypto_base64_encode(inBytes, inLen, noNewLines, &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

napi_value cryptoBase64Decode(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto [inBytes, inLen] = ctx.argByteArrayAt(0);
    auto noNewLines = ctx.argBoolAt(1);

    size_t outLen;
    auto outBytes = dynxx_crypto_base64_decode(inBytes, inLen, noNewLines, &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

// zip

napi_value zZipInit(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto mode = ctx.argIntAt(0);
    auto bufferSize = ctx.argLongAt(1);
    auto format = ctx.argIntAt(2);

    auto zip = dynxx_z_zip_init(static_cast<DynXXZipCompressMode>(mode), bufferSize, static_cast<DynXXZFormat>(format));
    return ctx.napiValueFromInt(zip);
}

napi_value zZipInput(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto zip = ctx.argLongAt(0);
    auto [inBytes, inLen] = ctx.argByteArrayAt(1);
    auto inFinish = ctx.argBoolAt(2);

    auto res = dynxx_z_zip_input(zip, inBytes, inLen, inFinish);
    return ctx.napiValueFromInt(res);
}

napi_value zZipProcessDo(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto zip = ctx.argLongAt(0);

    size_t outLen;
    auto outBytes = dynxx_z_zip_process_do(zip, &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

    freeX(outBytes);
    return v;
}

napi_value zZipProcessFinished(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto zip = ctx.argLongAt(0);

    auto res = dynxx_z_zip_process_finished(zip);
    return ctx.napiValueFromBool(res);
}

napi_value zZipRelease(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto zip = ctx.argLongAt(0);

    dynxx_z_zip_release(zip);

    return ctx.napiValueFromInt(napi_ok);
}

napi_value zUnZipInit(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto bufferSize = ctx.argLongAt(0);
    auto format = ctx.argIntAt(1);

    auto zip = dynxx_z_unzip_init(bufferSize, static_cast<DynXXZFormat>(format));
    return ctx.napiValueFromInt(zip);
}

napi_value zUnZipInput(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto zip = ctx.argLongAt(0);
    auto [inBytes, inLen] = ctx.argByteArrayAt(1);
    auto inFinish = ctx.argBoolAt(2);

    auto res = dynxx_z_unzip_input(zip, inBytes, inLen, inFinish);
    return ctx.napiValueFromInt(res);
}

napi_value zUnZipProcessDo(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto zip = ctx.argLongAt(0);

    size_t outLen;
    auto outBytes = dynxx_z_unzip_process_do(zip, &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

    freeX(outBytes);
    return v;
}

napi_value zUnZipProcessFinished(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto zip = ctx.argLongAt(0);

    auto res = dynxx_z_unzip_process_finished(zip);
    return ctx.napiValueFromBool(res);
}

napi_value zUnZipRelease(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto zip = ctx.argLongAt(0);

    dynxx_z_unzip_release(zip);

    return ctx.napiValueFromInt(napi_ok);
}

napi_value zZipBytes(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto mode = ctx.argIntAt(0);
    auto bufferSize = ctx.argLongAt(1);
    auto format = ctx.argIntAt(2);
    auto [inBytes, inLen] = ctx.argByteArrayAt(3);

    size_t outLen;
    auto outBytes = dynxx_z_bytes_zip(static_cast<DynXXZipCompressMode>(mode), bufferSize, static_cast<DynXXZFormat>(format), inBytes, inLen, &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

napi_value zUnzipBytes(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto bufferSize = ctx.argLongAt(0);
    auto format = ctx.argIntAt(1);
    auto [inBytes, inLen] = ctx.argByteArrayAt(2);

    size_t outLen;
    auto outBytes = dynxx_z_bytes_unzip(bufferSize, static_cast<DynXXZFormat>(format), inBytes, inLen, &outLen);
    auto v = ctx.napiValueFromByteArray(outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

// Lua

napi_value lLoadF(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto file = ctx.argCharsAt(0);

    auto res = dynxx_lua_loadF(file);
    auto nv = ctx.napiValueFromBool(res);

    freeX(file);
    return nv;
}

napi_value lLoadS(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto script = ctx.argCharsAt(0);

    auto res = dynxx_lua_loadS(script);
    auto nv = ctx.napiValueFromBool(res);

    freeX(script);
    return nv;
}

napi_value lCall(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto func = ctx.argCharsAt(0);
    auto params = ctx.argCharsAt(1);

    auto res = dynxx_lua_call(func, params);
    auto nv = ctx.napiValueFromChars(res);

    freeX(res);
    if (params) {
        freeX(params);
    }
    freeX(func);
    return nv;
}

// JS

napi_value jLoadF(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto file = ctx.argCharsAt(0);
    auto isModule = ctx.argBoolAt(1);

    auto res = dynxx_js_loadF(file, isModule);
    auto nv = ctx.napiValueFromBool(res);

    freeX(file);
    return nv;
}

napi_value jLoadS(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto script = ctx.argCharsAt(0);
    auto name = ctx.argCharsAt(1);
    auto isModule = ctx.argBoolAt(2);

    auto res = dynxx_js_loadS(script, name, isModule);
    auto nv = ctx.napiValueFromBool(res);

    freeX(script);
    freeX(name);
    return nv;
}

napi_value jLoadB(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto [inBytes, inLen] = ctx.argByteArrayAt(0);
    auto isModule = ctx.argBoolAt(1);

    auto b = dynxx_js_loadB(inBytes, inLen, isModule);
    auto v = ctx.napiValueFromBool(b);

    freeX(inBytes);
    return v;
}

napi_value jCall(napi_env env, napi_callback_info info) {
    NapiCallContext ctx(env, info);

    auto func = ctx.argCharsAt(0);
    auto params = ctx.argCharsAt(1);
    auto await = ctx.argBoolAt(2);

    auto res = dynxx_js_call(func, params, await);
    auto nv = ctx.napiValueFromChars(res);

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

// Register Module

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
        NAPI(kvAllKeys),
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
        NAPI(jsonDecoderReadInteger),
        NAPI(jsonDecoderReadFloat),
        NAPI(jsonDecoderRelease),

        NAPI(codingHexBytes2str),
        NAPI(codingHexStr2Bytes),

        NAPI(cryptoRand),
        NAPI(cryptoAesEncrypt),
        NAPI(cryptoAesDecrypt),
        NAPI(cryptoAesGcmEncrypt),
        NAPI(cryptoAesGcmDecrypt),
        NAPI(cryptoRsaGenKey),
        NAPI(cryptoRsaEncrypt),
        NAPI(cryptoRsaDecrypt),
        NAPI(cryptoHashMd5),
        NAPI(cryptoHashSha1),
        NAPI(cryptoHashSha256),
        NAPI(cryptoBase64Encode),
        NAPI(cryptoBase64Decode),

        NAPI(zZipInit),
        NAPI(zZipInput),
        NAPI(zZipProcessDo),
        NAPI(zZipProcessFinished),
        NAPI(zZipRelease),
        NAPI(zUnZipInit),
        NAPI(zUnZipInput),
        NAPI(zUnZipProcessDo),
        NAPI(zUnZipProcessFinished),
        NAPI(zUnZipRelease),
        NAPI(zZipBytes),
        NAPI(zUnzipBytes),
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
