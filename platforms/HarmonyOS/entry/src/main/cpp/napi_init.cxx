#include "napi/native_api.h"

#include <cstring>
#include <cstdlib>

#include <napi_util.hxx>

#include "../../../../../../build.HarmonyOS/output/include/NGenXX.h"

static napi_value NapiNGenXXGetVersion(napi_env env, napi_callback_info info)
{
    auto c = ngenxx_get_version();
    auto v = chars2NapiValue(env, c);
    freeX(c);
    return v;
}

static napi_value NapiNGenXXInit(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto root = napiValue2chars(env, args.v[0]);

    auto b = ngenxx_init(root);
    auto v = bool2NapiValue(env, b);

    freeX(root);
    return v;
}

static napi_value NapiNGenXXRelease(napi_env env, napi_callback_info info)
{
    ngenxx_release();

    return int2NapiValue(env, napi_ok);
}

#pragma mark Log Callback

typedef struct
{
    napi_async_work tsWork;
    napi_threadsafe_function tsWorkFunc;
    int logLevel;
    const char *logContent;
} TSLogWorkData;

static napi_env sNapiEnv;
static napi_ref sTsLogCallbackRef;

static void OnLogWorkCallTS(napi_env env, napi_value ts_callback, void *context, void *data)
{
    if (env == nullptr || ts_callback == nullptr || data == nullptr) 
    {
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

static void OnLogWorkExecute(napi_env env, void *data)
{
    auto tSLogWorkData = static_cast<TSLogWorkData *>(data);

    auto status = napi_acquire_threadsafe_function(tSLogWorkData->tsWorkFunc);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_acquire_threadsafe_function() failed");

    status = napi_call_threadsafe_function(tSLogWorkData->tsWorkFunc, tSLogWorkData, napi_tsfn_blocking);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_call_threadsafe_function() failed");
}

static void OnLogWorkComplete(napi_env env, napi_status status, void *data)
{
    auto tSLogWorkData = static_cast<TSLogWorkData *>(data);

    status = napi_release_threadsafe_function(tSLogWorkData->tsWorkFunc, napi_tsfn_release);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_release_threadsafe_function() failed");

    status = napi_delete_async_work(env, tSLogWorkData->tsWork);
    CHECK_NAPI_STATUS_RETURN_VOID(env, status, "napi_delete_async_work() failed");
}

static void engineLogCallback(int level, const char *content)
{
    if (sNapiEnv == nullptr || content == nullptr) 
    {
        return;
    }

    auto tSLogWorkData = mallocX<TSLogWorkData>();
    tSLogWorkData->tsWork = nullptr;
    tSLogWorkData->tsWorkFunc = nullptr;
    tSLogWorkData->logLevel = level;
    auto len = std::strlen(content);
    tSLogWorkData->logContent = mallocX<char>(len);
    std::strncpy(const_cast<char *>(tSLogWorkData->logContent), content, len);
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

static napi_value NapiNGenXXLogSetLevel(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto level = napiValue2int(env, args.v[0]);

    ngenxx_log_set_level(level);

    return int2NapiValue(env, napi_ok);
}

static napi_value NapiNGenXXLogSetCallback(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    napi_value vLogCallback = args.c > 0 ? args.v[0] : nullptr;
    int status;
    if (vLogCallback == nullptr)
    {
        sNapiEnv = nullptr;

        status = napi_delete_reference(env, sTsLogCallbackRef);
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_delete_reference() failed");

        ngenxx_log_set_callback(nullptr);
    }
    else
    {
        sNapiEnv = env;

        status = napi_create_reference(env, vLogCallback, 1, &sTsLogCallbackRef);
        CHECK_NAPI_STATUS_RETURN_NAPI_VALUE(env, status, "napi_create_reference() failed");

        ngenxx_log_set_callback(engineLogCallback);
    }

    return int2NapiValue(env, napi_ok);
}

static napi_value NapiNGenXXLogPrint(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto level = napiValue2int(env, args.v[0]);
    auto content = napiValue2chars(env, args.v[1]);

    ngenxx_log_print(level, content);
    freeX(content);

    return int2NapiValue(env, napi_ok);
}

#pragma mark Net

static napi_value NapiNGenXXNetHttpRequest(napi_env env, napi_callback_info info)
{
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
    FILE *cFILE = cFilePath ? std::fopen(cFilePath, "r") : nullptr;
    auto fileLength = args.c > 8 ? napiValue2long(env, args.v[8]) : 0;

    auto lTimeout = args.c > 9 ? napiValue2long(env, args.v[9]) : 15000;

    auto res = ngenxx_net_http_request(cUrl, cParams, iMethod,
                                              header_v, header_c,
                                              form_field_name_v, form_field_mime_v, form_field_data_v, form_field_count,
                                              static_cast<void *>(cFILE), fileLength,
                                              lTimeout);
    auto nv = chars2NapiValue(env, res);

    freeX(res);
    for (decltype(header_c) i = 0; i < header_c; i++)
    {
        freeX(header_v[i]);
    }
    for (decltype(form_field_count) i = 0; i < form_field_count; i++)
    {
        freeX(form_field_name_v[i]);
        freeX(form_field_mime_v[i]);
        freeX(form_field_data_v[i]);
    }
    if (cFILE)
    {
        std::fclose(cFILE);
    }
    if (cParams)
    {
        freeX(cParams);
    }
    if (cUrl)
    {
        freeX(cUrl);
    }
    return nv;
}

#pragma mark Store.SQLite

static napi_value NapiNGenXXStoreSQLiteOpen(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto _id = napiValue2chars(env, args.v[0]);
    auto res = ptr2addr(ngenxx_store_sqlite_open(_id));
    auto nv = long2NapiValue(env, res);

    freeX(_id);
    return nv;
}

static napi_value NapiNGenXXStoreSQLiteExecute(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto sql = napiValue2chars(env, args.v[1]);

    auto res = ngenxx_store_sqlite_execute(addr2ptr(conn), sql);
    auto nv = bool2NapiValue(env, res);

    freeX(sql);
    return nv;
}

static napi_value NapiNGenXXStoreSQLiteQueryDo(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto sql = napiValue2chars(env, args.v[1]);

    auto res = ptr2addr(ngenxx_store_sqlite_query_do(addr2ptr(conn), sql));
    auto nv = long2NapiValue(env, res);

    freeX(sql);
    return nv;
}

static napi_value NapiNGenXXStoreSQLiteQueryReadRow(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto query_result = napiValue2long(env, args.v[0]);

    auto res = ngenxx_store_sqlite_query_read_row(addr2ptr(query_result));
    return bool2NapiValue(env, res);
}

static napi_value NapiNGenXXStoreSQLiteQueryReadColumnText(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto query_result = napiValue2long(env, args.v[0]);
    auto column = napiValue2chars(env, args.v[1]);

    auto res = ngenxx_store_sqlite_query_read_column_text(addr2ptr(query_result), column);
    auto nv = chars2NapiValue(env, res);

    freeX(res);
    freeX(column);
    return nv;
}

static napi_value NapiNGenXXStoreSQLiteQueryReadColumnInteger(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto query_result = napiValue2long(env, args.v[0]);
    auto column = napiValue2chars(env, args.v[1]);

    auto res = ngenxx_store_sqlite_query_read_column_integer(addr2ptr(query_result), column);
    auto nv = long2NapiValue(env, res);

    freeX(column);
    return nv;
}

static napi_value NapiNGenXXStoreSQLiteQueryReadColumnFloat(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto query_result = napiValue2long(env, args.v[0]);
    auto column = napiValue2chars(env, args.v[1]);

    auto res = ngenxx_store_sqlite_query_read_column_float(addr2ptr(query_result), column);
    auto nv = double2NapiValue(env, res);

    freeX(column);
    return nv;
}

static napi_value NapiNGenXXStoreSQLiteQueryDrop(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto query_result = napiValue2long(env, args.v[0]);
    ngenxx_store_sqlite_query_drop(addr2ptr(query_result));

    return int2NapiValue(env, napi_ok);
}

static napi_value NapiNGenXXStoreSQLiteClose(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    ngenxx_store_sqlite_close(addr2ptr(conn));

    return int2NapiValue(env, napi_ok);
}

#pragma mark Store.KV

static napi_value NapiNGenXXStoreKVOpen(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto _id = napiValue2chars(env, args.v[0]);
    auto res = ptr2addr(ngenxx_store_kv_open(_id));
    auto nv = long2NapiValue(env, res);

    freeX(_id);
    return nv;
}

static napi_value NapiNGenXXStoreKVReadString(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);

    auto res = ngenxx_store_kv_read_string(addr2ptr(conn), k);
    auto nv = chars2NapiValue(env, res);

    freeX(res);
    freeX(k);
    return nv;
}

static napi_value NapiNGenXXStoreKVWriteString(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);
    auto v = napiValue2chars(env, args.v[2]);

    auto res = ngenxx_store_kv_write_string(addr2ptr(conn), k, v);
    auto nv = bool2NapiValue(env, res);

    freeX(v);
    freeX(k);
    return nv;
}

static napi_value NapiNGenXXStoreKVReadInteger(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);

    auto res = ngenxx_store_kv_read_integer(addr2ptr(conn), k);
    auto nv = long2NapiValue(env, res);

    freeX(k);
    return nv;
}

static napi_value NapiNGenXXStoreKVWriteInteger(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);
    auto v = napiValue2long(env, args.v[2]);

    auto res = ngenxx_store_kv_write_integer(addr2ptr(conn), k, v);
    auto nv = bool2NapiValue(env, res);

    freeX(k);
    return nv;
}

static napi_value NapiNGenXXStoreKVReadFloat(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);

    auto res = ngenxx_store_kv_read_integer(addr2ptr(conn), k);
    auto nv = double2NapiValue(env, res);

    freeX(k);
    return nv;
}

static napi_value NapiNGenXXStoreKVWriteFloat(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);
    auto v = napiValue2double(env, args.v[2]);

    auto res = ngenxx_store_kv_write_integer(addr2ptr(conn), k, v);
    auto nv = bool2NapiValue(env, res);

    freeX(k);
    return nv;
}

static napi_value NapiNGenXXStoreKVContains(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto  conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);

    auto res = ngenxx_store_kv_contains(addr2ptr(conn), k);
    auto nv = bool2NapiValue(env, res);

    freeX(k);
    return nv;
}

static napi_value NapiNGenXXStoreKVRemove(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    auto k = napiValue2chars(env, args.v[1]);

    auto res = ngenxx_store_kv_remove(addr2ptr(conn), k);
    auto nv = bool2NapiValue(env, res);

    freeX(k);
    return nv;
}

static napi_value NapiNGenXXStoreKVClear(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    ngenxx_store_kv_clear(addr2ptr(conn));

    return int2NapiValue(env, napi_ok);
}

static napi_value NapiNGenXXStoreKVClose(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto conn = napiValue2long(env, args.v[0]);
    ngenxx_store_kv_close(addr2ptr(conn));

    return int2NapiValue(env, napi_ok);
}

#pragma mark DeviceInfo

static napi_value NapiNGenXXDeviceType(napi_env env, napi_callback_info info)
{
    auto dt = ngenxx_device_type();
    auto v = int2NapiValue(env, dt);
    return v;
}

static napi_value NapiNGenXXDeviceName(napi_env env, napi_callback_info info)
{
    auto cDN = ngenxx_device_name();
    auto v = chars2NapiValue(env, cDN);
    freeX(cDN);
    
    return v;
}

static napi_value NapiNGenXXDeviceManufacturer(napi_env env, napi_callback_info info)
{
    auto cDM = ngenxx_device_manufacturer();
    auto v = chars2NapiValue(env, cDM);
    freeX(cDM);
    return v;
}

static napi_value NapiNGenXXDeviceOsVersion(napi_env env, napi_callback_info info)
{
    auto cOV = ngenxx_device_name();
    auto v = chars2NapiValue(env, cOV);
    freeX(cOV);
    return v;
}

static napi_value NapiNGenXXDeviceCpuArch(napi_env env, napi_callback_info info)
{
    auto dca = ngenxx_device_cpu_arch();
    auto v = int2NapiValue(env, dca);
    return v;
}

#pragma mark JsonDecoder

static napi_value NapiNGenXXJsonDecoderInit(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto cJson = napiValue2chars(env, args.v[0]);

    auto res = ptr2addr(ngenxx_json_decoder_init(cJson));
    auto v = long2NapiValue(env, res);

    freeX(cJson);
    return v;
}

static napi_value NapiNGenXXJsonDecoderIsArray(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto decoder = napiValue2long(env, args.v[0]);
    auto node = args.c > 1 ? napiValue2long(env, args.v[1]) : 0;

    auto res = ngenxx_json_decoder_is_array(addr2ptr(decoder), addr2ptr(node));
    return bool2NapiValue(env, res);
}

static napi_value NapiNGenXXJsonDecoderIsObject(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto decoder = napiValue2long(env, args.v[0]);
    auto node = args.c > 1 ? napiValue2long(env, args.v[1]) : 0;

    auto res = ngenxx_json_decoder_is_object(addr2ptr(decoder), addr2ptr(node));
    return bool2NapiValue(env, res);
}

static napi_value NapiNGenXXJsonDecoderReadNode(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto decoder = napiValue2long(env, args.v[0]);
    auto cK = napiValue2chars(env, args.v[1]);
    auto node = args.c > 2 ? napiValue2long(env, args.v[2]) : 0;

    auto res = ptr2addr(ngenxx_json_decoder_read_node(addr2ptr(decoder), addr2ptr(node), cK));
    auto v = long2NapiValue(env, res);

    freeX(cK);
    return v;
}

static napi_value NapiNGenXXJsonDecoderReadChild(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto decoder = napiValue2long(env, args.v[0]);
    auto node = args.c > 1 ? napiValue2long(env, args.v[1]) : 0;

    auto res = ptr2addr(ngenxx_json_decoder_read_child(addr2ptr(decoder), addr2ptr(node)));
    return long2NapiValue(env, res);
}

static napi_value NapiNGenXXJsonDecoderReadNext(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto decoder = napiValue2long(env, args.v[0]);
    auto node = args.c > 1 ? napiValue2long(env, args.v[1]) : 0;

    auto res = ptr2addr(ngenxx_json_decoder_read_next(addr2ptr(decoder), addr2ptr(node)));
    return long2NapiValue(env, res);
}

static napi_value NapiNGenXXJsonDecoderReadString(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto decoder = napiValue2long(env, args.v[0]);
    auto node = args.c > 1 ? napiValue2long(env, args.v[1]) : 0;

    auto cRes = ngenxx_json_decoder_read_string(addr2ptr(decoder), addr2ptr(node));
    auto v = chars2NapiValue(env, cRes);

    freeX(cRes);
    return v;
}

static napi_value NapiNGenXXJsonDecoderReadNumber(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto decoder = napiValue2long(env, args.v[0]);
    auto node = args.c > 1 ? napiValue2long(env, args.v[1]) : 0;

    auto res = ngenxx_json_decoder_read_number(addr2ptr(decoder), addr2ptr(node));
    return double2NapiValue(env, res);
}

static napi_value NapiNGenXXJsonDecoderRelease(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto decoder = napiValue2long(env, args.v[0]);

    ngenxx_json_decoder_release(addr2ptr(decoder));

    return int2NapiValue(env, napi_ok);
}

#pragma mark Coding

static napi_value NapiNGenXXCodingHexBytes2str(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);

    auto cRes = ngenxx_coding_hex_bytes2str(inBytes, inLen);
    auto v = chars2NapiValue(env, cRes);

    freeX(cRes);
    freeX(inBytes);
    return v;
}

static napi_value NapiNGenXXCodingHexStr2Bytes(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto cStr = napiValue2chars(env, args.v[0]);

    size_t outLen;
    auto cRes = ngenxx_coding_hex_str2bytes(cStr, &outLen);
    auto v = byteArray2NapiValue(env, cRes, outLen);

    freeX(cRes);
    freeX(cStr);
    return v;
}

#pragma mark Crypto

static napi_value NapiNGenXXCryptoRand(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto len = napiValue2int(env, args.v[0]);
    byte out[len];
    std::memset(out, 0, len);

    ngenxx_crypto_rand(len, out);
    auto v = byteArray2NapiValue(env, out, len);

    return v;
}

static napi_value NapiNGenXXCryptoAesEncrypt(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);
    auto keyLen = napiValueArrayLen(env, args.v[1]);
    auto keyBytes = napiValue2byteArray(env, args.v[1], keyLen);

    size_t outLen;
    auto outBytes = ngenxx_crypto_aes_encrypt(inBytes, inLen, keyBytes, keyLen, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    freeX(keyBytes);
    freeX(inBytes);
    return v;
}

static napi_value NapiNGenXXCryptoAesDecrypt(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);
    auto keyLen = napiValueArrayLen(env, args.v[1]);
    auto keyBytes = napiValue2byteArray(env, args.v[1], keyLen);

    size_t outLen;
    auto outBytes = ngenxx_crypto_aes_decrypt(inBytes, inLen, keyBytes, keyLen, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    freeX(keyBytes);
    freeX(inBytes);
    return v;
}

static napi_value NapiNGenXXCryptoAesGcmEncrypt(napi_env env, napi_callback_info info)
{
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
    auto outBytes = ngenxx_crypto_aes_gcm_encrypt(inBytes, inLen, keyBytes, keyLen, initVectorBytes, initVectorLen, aadBytes, aadLen, tagBits, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    if (aadBytes) 
    {
        freeX(aadBytes);
    }
    freeX(initVectorBytes);
    freeX(keyBytes);
    freeX(inBytes);
    return v;
}

static napi_value NapiNGenXXCryptoAesGcmDecrypt(napi_env env, napi_callback_info info)
{
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
    auto outBytes = ngenxx_crypto_aes_gcm_decrypt(inBytes, inLen, keyBytes, keyLen, initVectorBytes, initVectorLen, aadBytes, aadLen, tagBits, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    if (aadBytes) 
    {
        freeX(aadBytes);
    }
    freeX(initVectorBytes);
    freeX(keyBytes);
    freeX(inBytes);
    return v;
}

static napi_value NapiNGenXXCryptoHashMd5(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);

    size_t outLen;
    auto outBytes = ngenxx_crypto_hash_md5(inBytes, inLen, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

static napi_value NapiNGenXXCryptoHashSha256(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);

    size_t outLen;
    auto outBytes = ngenxx_crypto_hash_sha256(inBytes, inLen, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

static napi_value NapiNGenXXCryptoBase64Encode(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);

    size_t outLen;
    auto outBytes = ngenxx_crypto_base64_encode(inBytes, inLen, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

static napi_value NapiNGenXXCryptoBase64Decode(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);

    size_t outLen;
    auto outBytes = ngenxx_crypto_base64_decode(inBytes, inLen, &outLen);
    auto v = byteArray2NapiValue(env, outBytes, outLen);

    freeX(outBytes);
    freeX(inBytes);
    return v;
}

#pragma mark Lua

static napi_value NapiNGenXXLLoadF(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto file = napiValue2chars(env, args.v[0]);

    auto res = ngenxx_lua_loadF(file);
    auto nv = bool2NapiValue(env, res);

    freeX(file);
    return nv;
}

static napi_value NapiNGenXXLLoadS(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto script = napiValue2chars(env, args.v[0]);

    auto res = ngenxx_lua_loadS(script);
    auto nv = bool2NapiValue(env, res);

    freeX(script);
    return nv;
}

static napi_value NapiNGenXXLCall(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto func = napiValue2chars(env, args.v[0]);
    auto params = napiValue2chars(env, args.v[1]);

    auto res = ngenxx_lua_call(func, params);
    auto nv = chars2NapiValue(env, res);

    freeX(res);
    if (params) 
    {
        freeX(params);
    }
    freeX(func);
    return nv;
}

#pragma mark JS

static napi_value NapiNGenXXJLoadF(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto file = napiValue2chars(env, args.v[0]);
    auto isModule = napiValue2bool(env, args.v[1]);

    auto res = ngenxx_js_loadF(file, isModule);
    auto nv = bool2NapiValue(env, res);

    freeX(file);
    return nv;
}

static napi_value NapiNGenXXJLoadS(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto script = napiValue2chars(env, args.v[0]);
    auto name = napiValue2chars(env, args.v[1]);
    auto isModule = napiValue2bool(env, args.v[2]);

    auto res = ngenxx_js_loadS(script, name, isModule);
    auto nv = bool2NapiValue(env, res);

    freeX(script);
    freeX(name);
    return nv;
}

static napi_value NapiNGenXXJLoadB(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto inLen = napiValueArrayLen(env, args.v[0]);
    auto inBytes = napiValue2byteArray(env, args.v[0], inLen);
    auto isModule = napiValue2bool(env, args.v[1]);

    size_t outLen;
    auto b = ngenxx_js_loadB(inBytes, inLen, isModule);
    auto v = bool2NapiValue(env, b);

    freeX(inBytes);
    return v;
}

static napi_value NapiNGenXXJCall(napi_env env, napi_callback_info info)
{
    Args args(env, info);

    auto func = napiValue2chars(env, args.v[0]);
    auto params = napiValue2chars(env, args.v[1]);
    auto await = napiValue2bool(env, args.v[2]);

    auto res = ngenxx_js_call(func, params, await);
    auto nv = chars2NapiValue(env, res);

    freeX(res);
    if (params) 
    {
        freeX(params);
    }
    freeX(func);
    return nv;
}

#pragma mark Register Module

EXTERN_C_START
static napi_value NapiNGenXXRegisterFuncs(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = 
    {
        {"getVersion", NULL, NapiNGenXXGetVersion, NULL, NULL, NULL, napi_default, NULL},
        {"init", NULL, NapiNGenXXInit, NULL, NULL, NULL, napi_default, NULL},
        {"release", NULL, NapiNGenXXRelease, NULL, NULL, NULL, napi_default, NULL},

        {"logSetLevel", NULL, NapiNGenXXLogSetLevel, NULL, NULL, NULL, napi_default, NULL},
        {"logSetCallback", NULL, NapiNGenXXLogSetCallback, NULL, NULL, NULL, napi_default, NULL},
        {"logPrint", NULL, NapiNGenXXLogPrint, NULL, NULL, NULL, napi_default, NULL},

        {"netHttpRequest", NULL, NapiNGenXXNetHttpRequest, NULL, NULL, NULL, napi_default, NULL},

        {"lLoadF", NULL, NapiNGenXXLLoadF, NULL, NULL, NULL, napi_default, NULL},
        {"lLoadS", NULL, NapiNGenXXLLoadS, NULL, NULL, NULL, napi_default, NULL},
        {"lCall", NULL, NapiNGenXXLCall, NULL, NULL, NULL, napi_default, NULL},

        {"jLoadF", NULL, NapiNGenXXJLoadF, NULL, NULL, NULL, napi_default, NULL},
        {"jLoadS", NULL, NapiNGenXXJLoadS, NULL, NULL, NULL, napi_default, NULL},
        {"jLoadB", NULL, NapiNGenXXJLoadB, NULL, NULL, NULL, napi_default, NULL},
        {"jCall", NULL, NapiNGenXXJCall, NULL, NULL, NULL, napi_default, NULL},

        {"storeSQLiteOpen", NULL, NapiNGenXXStoreSQLiteOpen, NULL, NULL, NULL, napi_default, NULL},
        {"storeSQLiteExecute", NULL, NapiNGenXXStoreSQLiteExecute, NULL, NULL, NULL, napi_default, NULL},
        {"storeSQLiteQueryDo", NULL, NapiNGenXXStoreSQLiteQueryDo, NULL, NULL, NULL, napi_default, NULL},
        {"storeSQLiteQueryReadRow", NULL, NapiNGenXXStoreSQLiteQueryReadRow, NULL, NULL, NULL, napi_default, NULL},
        {"storeSQLiteQueryReadColumnText", NULL, NapiNGenXXStoreSQLiteQueryReadColumnText, NULL, NULL, NULL, napi_default, NULL},
        {"storeSQLiteQueryReadColumnInteger", NULL, NapiNGenXXStoreSQLiteQueryReadColumnInteger, NULL, NULL, NULL, napi_default, NULL},
        {"storeSQLiteQueryReadColumnFloat", NULL, NapiNGenXXStoreSQLiteQueryReadColumnFloat, NULL, NULL, NULL, napi_default, NULL},
        {"storeSQLiteQueryDrop", NULL, NapiNGenXXStoreSQLiteQueryDrop, NULL, NULL, NULL, napi_default, NULL},
        {"storeSQLiteClose", NULL, NapiNGenXXStoreSQLiteClose, NULL, NULL, NULL, napi_default, NULL},

        {"storeKVOpen", NULL, NapiNGenXXStoreKVOpen, NULL, NULL, NULL, napi_default, NULL},
        {"storeKVReadString", NULL, NapiNGenXXStoreKVReadString, NULL, NULL, NULL, napi_default, NULL},
        {"storeKVWriteString", NULL, NapiNGenXXStoreKVWriteString, NULL, NULL, NULL, napi_default, NULL},
        {"storeKVReadInteger", NULL, NapiNGenXXStoreKVReadInteger, NULL, NULL, NULL, napi_default, NULL},
        {"storeKVWriteInteger", NULL, NapiNGenXXStoreKVWriteInteger, NULL, NULL, NULL, napi_default, NULL},
        {"storeKVReadFloat", NULL, NapiNGenXXStoreKVReadFloat, NULL, NULL, NULL, napi_default, NULL},
        {"storeKVWriteFloat", NULL, NapiNGenXXStoreKVWriteFloat, NULL, NULL, NULL, napi_default, NULL},
        {"storeKVContains", NULL, NapiNGenXXStoreKVContains, NULL, NULL, NULL, napi_default, NULL},
        {"storeKVRemove", NULL, NapiNGenXXStoreKVRemove, NULL, NULL, NULL, napi_default, NULL},
        {"storeKVClear", NULL, NapiNGenXXStoreKVClear, NULL, NULL, NULL, napi_default, NULL},
        {"storeKVClose", NULL, NapiNGenXXStoreKVClose, NULL, NULL, NULL, napi_default, NULL},

        {"deviceType", NULL, NapiNGenXXDeviceType, NULL, NULL, NULL, napi_default, NULL},
        {"deviceName", NULL, NapiNGenXXDeviceName, NULL, NULL, NULL, napi_default, NULL},
        {"deviceManufacturer", NULL, NapiNGenXXDeviceManufacturer, NULL, NULL, NULL, napi_default, NULL},
        {"deviceOsVersion", NULL, NapiNGenXXDeviceOsVersion, NULL, NULL, NULL, napi_default, NULL},
        {"deviceCpuArch", NULL, NapiNGenXXDeviceCpuArch, NULL, NULL, NULL, napi_default, NULL},

        {"jsonDecoderInit", NULL, NapiNGenXXJsonDecoderInit, NULL, NULL, NULL, napi_default, NULL},
        {"jsonDecoderIsArray", NULL, NapiNGenXXJsonDecoderIsArray, NULL, NULL, NULL, napi_default, NULL},
        {"jsonDecoderIsObject", NULL, NapiNGenXXJsonDecoderIsObject, NULL, NULL, NULL, napi_default, NULL},
        {"jsonDecoderReadNode", NULL, NapiNGenXXJsonDecoderReadNode, NULL, NULL, NULL, napi_default, NULL},
        {"jsonDecoderReadChild", NULL, NapiNGenXXJsonDecoderReadChild, NULL, NULL, NULL, napi_default, NULL},
        {"jsonDecoderReadNext", NULL, NapiNGenXXJsonDecoderReadNext, NULL, NULL, NULL, napi_default, NULL},
        {"jsonDecoderReadString", NULL, NapiNGenXXJsonDecoderReadString, NULL, NULL, NULL, napi_default, NULL},
        {"jsonDecoderReadNumber", NULL, NapiNGenXXJsonDecoderReadNumber, NULL, NULL, NULL, napi_default, NULL},
        {"jsonDecoderRelease", NULL, NapiNGenXXJsonDecoderRelease, NULL, NULL, NULL, napi_default, NULL},

        {"codingHexBytes2str", NULL, NapiNGenXXCodingHexBytes2str, NULL, NULL, NULL, napi_default, NULL},
        {"codingHexStr2Bytes", NULL, NapiNGenXXCodingHexStr2Bytes, NULL, NULL, NULL, napi_default, NULL},

        {"cryptoRand", NULL, NapiNGenXXCryptoRand, NULL, NULL, NULL, napi_default, NULL},
        {"cryptoAesEncrypt", NULL, NapiNGenXXCryptoAesEncrypt, NULL, NULL, NULL, napi_default, NULL},
        {"cryptoAesDecrypt", NULL, NapiNGenXXCryptoAesDecrypt, NULL, NULL, NULL, napi_default, NULL},
        {"cryptoAesGcmEncrypt", NULL, NapiNGenXXCryptoAesGcmEncrypt, NULL, NULL, NULL, napi_default, NULL},
        {"cryptoAesGcmDecrypt", NULL, NapiNGenXXCryptoAesGcmDecrypt, NULL, NULL, NULL, napi_default, NULL},
        {"cryptoHashMd5", NULL, NapiNGenXXCryptoHashMd5, NULL, NULL, NULL, napi_default, NULL},
        {"cryptoHashSha256", NULL, NapiNGenXXCryptoHashSha256, NULL, NULL, NULL, napi_default, NULL},
        {"cryptoBase64Encode", NULL, NapiNGenXXCryptoBase64Encode, NULL, NULL, NULL, napi_default, NULL},
        {"cryptoBase64Decode", NULL, NapiNGenXXCryptoBase64Decode, NULL, NULL, NULL, napi_default, NULL},
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module ngenxxModule = 
{
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = NULL,
    .nm_register_func = NapiNGenXXRegisterFuncs,
    .nm_modname = "ngenxx",
    .nm_priv = ((void*)(0)),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterNGenXXModule(void) 
{ 
    napi_module_register(&ngenxxModule); 
}
