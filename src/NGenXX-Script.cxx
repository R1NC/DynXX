#include "NGenXX-Script.hxx"

#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <functional>
#if defined(USE_STD_RANGES)
#include <ranges>
#endif

#include <NGenXX.hxx>
#include <NGenXXNetHttp.h>
#include "json/JsonDecoder.hxx"

std::string bytes2json(const Bytes &bytes)
{
    if (bytes.empty()) [[unlikely]]
    {
        return {};
    }
#if defined(USE_STD_RANGES)
    auto intV = bytes 
        | std::views::transform([](const auto b) { return static_cast<int>(b); })
        | std::ranges::to<std::vector>();
#else
    std::vector<int> intV(bytes.size());
    std::transform(bytes.begin(), bytes.end(), intV.begin(), [](const auto b) { 
        return static_cast<int>(b); 
    });
#endif
    auto cj = cJSON_CreateIntArray(intV.data(), static_cast<int>(intV.size()));
    return cJSON_PrintUnformatted(cj);
}

std::string strArray2json(const std::vector<std::string> &v)
{
    auto cj = cJSON_CreateArray();
    for (const auto &it : v)
    {
        cJSON_AddItemToArray(cj, cJSON_CreateString(it.c_str()));
    }
    return cJSON_PrintUnformatted(cj);
}

double parseNum(const NGenXX::Json::Decoder &decoder, const char *k)
{
    return decoder.readNumber(decoder[k]);
}

std::string parseStr(const NGenXX::Json::Decoder &decoder, const char *k)
{
    return decoder.readString(decoder[k]);
}

template <typename T = void>
T* parsePtr(const NGenXX::Json::Decoder &decoder, const char *k)
{
    address addr = 0;
    auto s = parseStr(decoder, k);
    if (s.empty())
    {
        return nullptr;
    }
    
    try
    {
        addr = std::stoll(s);
    }
    catch (const std::exception &e)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "parsePtr failed s:{}", s);
        return nullptr;
    }

    return addr2ptr<T>(addr);
}

Bytes parseByteArray(const NGenXX::Json::Decoder &decoder, const char *bytesK)
{
    Bytes data;
    auto byte_vNode = decoder[bytesK];
    if (byte_vNode)
    {
        decoder.readChildren(byte_vNode, 
                            [&data, &decoder](size_t idx, const void *const child)
                            {
                                data.emplace_back(decoder.readNumber(child));
                            });
    }
    return data;
}

std::vector<std::string> parseStrArray(const NGenXX::Json::Decoder &decoder, const char *strVK)
{
    std::vector<std::string> v;
    const auto str_vNode = decoder[strVK];
    if (str_vNode)
    {
        decoder.readChildren(str_vNode,
                             [&v, &decoder](size_t idx, const void *const child)
                             {
                                 v.emplace_back(decoder.readString(child));
                             });
    }
    return v;
}

std::string ngenxx_get_versionS(const char *json)
{
    return ngenxxGetVersion();
}

std::string ngenxx_root_pathS(const char *json)
{
    return ngenxxRootPath();
}

#pragma mark Device.DeviceInfo

int ngenxx_device_typeS(const char *json)
{
    return static_cast<int>(ngenxxDeviceType());
}

std::string ngenxx_device_nameS(const char *json)
{
    return ngenxxDeviceName();
}

std::string ngenxx_device_manufacturerS(const char *json)
{
    return ngenxxDeviceManufacturer();
}

std::string ngenxx_device_os_versionS(const char *json)
{
    return ngenxxDeviceOsVersion();
}

int ngenxx_device_cpu_archS(const char *json)
{
    return static_cast<int>(ngenxxDeviceCpuArch());
}

#pragma mark Log

void ngenxx_log_printS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    NGenXX::Json::Decoder decoder(json);
    auto level = parseNum(decoder, "level");
    auto content = parseStr(decoder, "content");
    if (level < 0 || content.empty())
    {
        return;
    }

    ngenxxLogPrint(static_cast<NGenXXLogLevelX>(level), content);
}

#pragma mark Net.Http

std::string ngenxx_net_http_requestS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto url = parseStr(decoder, "url");
    auto params = parseStr(decoder, "params");
    auto method = parseNum(decoder, "method");

    auto rawBody = parseByteArray(decoder, "rawBodyBytes");

    auto header_v = parseStrArray(decoder, "header_v");

    auto form_field_name_v = parseStrArray(decoder, "form_field_name_v");
    auto form_field_mime_v = parseStrArray(decoder, "form_field_mime_v");
    auto form_field_data_v = parseStrArray(decoder, "form_field_data_v");

    auto cFILE = parsePtr<std::FILE>(decoder, "cFILE");
    auto fileSize = parseNum(decoder, "file_size");

    auto timeout = parseNum(decoder, "timeout");

    auto header_c = header_v.size();
    auto form_field_count = form_field_name_v.size();
    if (method < 0 || url.empty() || header_c > NGENXX_HTTP_HEADER_MAX_COUNT)
    {
        return s;
    }
    if (form_field_count == 0 && (!form_field_name_v.empty() || !form_field_mime_v.empty() || !form_field_data_v.empty())) [[unlikely]]
    {
        return s;
    }
    if (form_field_count > 0 && (form_field_name_v.empty() || form_field_mime_v.empty() || form_field_data_v.empty())) [[unlikely]]
    {
        return s;
    }
    if ((cFILE != nullptr && fileSize == 0) || (cFILE == nullptr && fileSize > 0)) [[unlikely]]
    {
        return s;
    }

    auto t = ngenxxNetHttpRequest(url, static_cast<NGenXXHttpMethodX>(method), params, rawBody,
                                  header_v,
                                  form_field_name_v,
                                  form_field_mime_v,
                                  form_field_data_v,
                                  cFILE,
                                  fileSize,
                                  timeout);
    return t.toJson();
}

bool ngenxx_net_http_downloadS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto url = parseStr(decoder, "url");
    auto file = parseStr(decoder, "file");
    auto timeout = parseNum(decoder, "timeout");

    if (url.empty() || file.empty())
    {
        return false;
    }

    return ngenxxNetHttpDownload(url, file, timeout);
}

#pragma mark Store.SQLite

std::string ngenxx_store_sqlite_openS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto _id = parseStr(decoder, "_id");
    if (_id.empty())
    {
        return s;
    }

    auto db = ngenxxStoreSqliteOpen(_id);
    if (db == nullptr)
    {
        return s;
    }
    return std::to_string(ptr2addr(db));
}

bool ngenxx_store_sqlite_executeS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parsePtr(decoder, "conn");
    auto sql = parseStr(decoder, "sql");
    if (conn == nullptr || sql.empty())
    {
        return false;
    }

    return ngenxxStoreSqliteExecute(conn, sql);
}

std::string ngenxx_store_sqlite_query_doS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parsePtr(decoder, "conn");
    auto sql = parseStr(decoder, "sql");
    if (conn == nullptr || sql.empty())
    {
        return s;
    }

    auto res = ngenxxStoreSqliteQueryDo(conn, sql);
    if (res == nullptr)
    {
        return s;
    }
    return std::to_string(ptr2addr(res));
}

bool ngenxx_store_sqlite_query_read_rowS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto query_result = parsePtr(decoder, "query_result");
    if (query_result == nullptr)
    {
        return false;
    }

    return ngenxxStoreSqliteQueryReadRow(query_result);
}

std::string ngenxx_store_sqlite_query_read_column_textS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto query_result = parsePtr(decoder, "query_result");
    auto column = parseStr(decoder, "column");
    if (query_result == nullptr || column.empty())
    {
        return s;
    }

    return ngenxxStoreSqliteQueryReadColumnText(query_result, column);
}

int64_t ngenxx_store_sqlite_query_read_column_integerS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    NGenXX::Json::Decoder decoder(json);
    auto query_result = parsePtr(decoder, "query_result");
    auto column = parseStr(decoder, "column");
    if (query_result == nullptr || column.empty())
    {
        return 0;
    }

    return ngenxxStoreSqliteQueryReadColumnInteger(query_result, column);
}

double ngenxx_store_sqlite_query_read_column_floatS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    NGenXX::Json::Decoder decoder(json);
    auto query_result = parsePtr(decoder, "query_result");
    auto column = parseStr(decoder, "column");
    if (query_result == nullptr || column.empty())
    {
        return 0;
    }

    return ngenxxStoreSqliteQueryReadColumnFloat(query_result, column);
}

void ngenxx_store_sqlite_query_dropS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    NGenXX::Json::Decoder decoder(json);
    auto query_result = parsePtr(decoder, "query_result");
    if (query_result == nullptr)
    {
        return;
    }

    ngenxxStoreSqliteQueryDrop(query_result);
}

void ngenxx_store_sqlite_closeS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parsePtr(decoder, "conn");
    if (conn == nullptr)
    {
        return;
    }

    ngenxxStoreSqliteClose(conn);
}

#pragma mark Store.KV

std::string ngenxx_store_kv_openS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto _id = parseStr(decoder, "_id");
    if (_id.empty())
    {
        return s;
    }

    auto res = ngenxxStoreKvOpen(_id);
    if (res == nullptr)
    {
        return s;
    }
    return std::to_string(ptr2addr(res));
}

std::string ngenxx_store_kv_read_stringS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parsePtr(decoder, "conn");
    auto k = parseStr(decoder, "k");
    if (conn == nullptr || k.empty())
    {
        return s;
    }

    return ngenxxStoreKvReadString(conn, k);
}

bool ngenxx_store_kv_write_stringS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parsePtr(decoder, "conn");
    auto k = parseStr(decoder, "k");
    auto v = parseStr(decoder, "v");
    if (conn == nullptr || k.empty())
    {
        return false;
    }

    return ngenxxStoreKvWriteString(conn, k, v);
}

int64_t ngenxx_store_kv_read_integerS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parsePtr(decoder, "conn");
    auto k = parseStr(decoder, "k");
    if (conn == nullptr || k.empty())
    {
        return 0;
    }

    return ngenxxStoreKvReadInteger(conn, k);
}

bool ngenxx_store_kv_write_integerS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parsePtr(decoder, "conn");
    auto k = parseStr(decoder, "k");
    auto v = parseNum(decoder, "v");
    if (conn == nullptr || k.empty())
    {
        return false;
    }

    return ngenxxStoreKvWriteInteger(conn, k, v);
}

double ngenxx_store_kv_read_floatS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parsePtr(decoder, "conn");
    auto k = parseStr(decoder, "k");
    if (conn == nullptr || k.empty())
    {
        return false;
    }

    return ngenxxStoreKvReadFloat(conn, k);
}

bool ngenxx_store_kv_write_floatS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parsePtr(decoder, "conn");
    auto k = parseStr(decoder, "k");
    auto v = parseNum(decoder, "v");
    if (conn == nullptr || k.empty())
    {
        return false;
    }

    return ngenxxStoreKvWriteFloat(conn, k, v);
}

std::string ngenxx_store_kv_all_keysS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parsePtr(decoder, "conn");
    if (conn == nullptr)
    {
        return s;
    }

    auto res = ngenxxStoreKvAllKeys(conn);
    return strArray2json(res);
}

bool ngenxx_store_kv_containsS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parsePtr(decoder, "conn");
    auto k = parseStr(decoder, "k");
    if (conn == nullptr || k.empty())
    {
        return false;
    }

    return ngenxxStoreKvContains(conn, k);
}

bool ngenxx_store_kv_removeS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parsePtr(decoder, "conn");
    auto k = parseStr(decoder, "k");
    if (conn == nullptr || k.empty())
    {
        return false;
    }

    return ngenxxStoreKvRemove(conn, k);
}

void ngenxx_store_kv_clearS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parsePtr(decoder, "conn");
    if (conn == nullptr)
    {
        return;
    }

    ngenxxStoreKvClear(conn);
}

void ngenxx_store_kv_closeS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parsePtr(decoder, "conn");
    if (conn == nullptr)
    {
        return;
    }

    ngenxxStoreKvClose(conn);
}

#pragma mark Coding

std::string ngenxx_coding_hex_bytes2strS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return s;
    }

    return ngenxxCodingHexBytes2str(in);
}

std::string ngenxx_coding_hex_str2bytesS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);

    auto str = parseStr(decoder, "str");

    if (str.size() == 0)
    {
        return s;
    }

    auto bytes = ngenxxCodingHexStr2bytes(str);
    return bytes2json(bytes);
}

std::string ngenxx_coding_bytes2strS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return s;
    }

    return ngenxxCodingBytes2str(in);
}

std::string ngenxx_coding_str2bytesS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);

    auto str = parseStr(decoder, "str");

    if (str.size() == 0)
    {
        return s;
    }

    auto bytes = ngenxxCodingStr2bytes(str);
    return bytes2json(bytes);
}

std::string ngenxx_coding_case_upperS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);

    auto str = parseStr(decoder, "str");

    if (str.size() == 0)
    {
        return s;
    }
    return ngenxxCodingCaseUpper(str);
}

std::string ngenxx_coding_case_lowerS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);

    auto str = parseStr(decoder, "str");

    if (str.size() == 0)
    {
        return s;
    }
    return ngenxxCodingCaseLower(str);
}

#pragma mark Crypto

std::string ngenxx_crypto_randS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    size_t outLen = parseNum(decoder, "len");
    if (outLen == 0)
    {
        return s;
    }
    byte outBytes[outLen];

    ngenxxCryptoRand(outLen, outBytes);
    return bytes2json(wrapBytes(outBytes, outLen));
}

std::string ngenxx_crypto_aes_encryptS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return s;
    }

    auto key = parseByteArray(decoder, "keyBytes");
    if (key.empty())
    {
        return s;
    }

    auto outBytes = ngenxxCryptoAesEncrypt(in, key);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_aes_decryptS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return s;
    }

    auto key = parseByteArray(decoder, "keyBytes");
    if (key.empty())
    {
        return s;
    }

    auto outBytes = ngenxxCryptoAesDecrypt(in, key);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_aes_gcm_encryptS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return s;
    }

    auto key = parseByteArray(decoder, "keyBytes");
    if (key.empty())
    {
        return s;
    }

    auto iv = parseByteArray(decoder, "initVectorBytes");
    if (iv.empty())
    {
        return s;
    }

    auto aad = parseByteArray(decoder, "aadBytes");

    auto tagBits = parseNum(decoder, "tagBits");

    auto outBytes = ngenxxCryptoAesGcmEncrypt(in, key, iv, tagBits, aad);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_aes_gcm_decryptS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return s;
    }

    auto key = parseByteArray(decoder, "keyBytes");
    if (key.empty())
    {
        return s;
    }

    auto iv = parseByteArray(decoder, "initVectorBytes");
    if (iv.empty())
    {
        return s;
    }

    auto aad = parseByteArray(decoder, "aadBytes");

    auto tagBits = parseNum(decoder, "tagBits");

    auto outBytes = ngenxxCryptoAesGcmDecrypt(in, key, iv, tagBits, aad);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_hash_md5S(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return s;
    }

    auto outBytes = ngenxxCryptoHashMd5(in);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_hash_sha256S(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return s;
    }

    auto outBytes = ngenxxCryptoHashSha256(in);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_base64_encodeS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return s;
    }

    auto outBytes = ngenxxCryptoBase64Encode(in);
    return bytes2json(outBytes);
}

std::string ngenxx_crypto_base64_decodeS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return s;
    }

    auto outBytes = ngenxxCryptoBase64Decode(in);
    return bytes2json(outBytes);
}

#pragma mark Zip

std::string ngenxx_z_zip_initS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto mode = parseNum(decoder, "mode");
    auto bufferSize = parseNum(decoder, "bufferSize");
    auto format = parseNum(decoder, "format");

    auto zip = ngenxxZZipInit(static_cast<NGenXXZipCompressModeX>(mode), bufferSize, static_cast<NGenXXZFormatX>(format));
    if (zip == nullptr)
    {
        return s;
    }
    return std::to_string(ptr2addr(zip));
}

size_t ngenxx_z_zip_inputS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    NGenXX::Json::Decoder decoder(json);
    auto zip = parsePtr(decoder, "zip");
    if (zip == nullptr)
    {
        return 0;
    }

    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return 0;
    }

    auto inFinish = static_cast<bool>(parseNum(decoder, "inFinish"));

    return ngenxxZZipInput(zip, in, inFinish);
}

std::string ngenxx_z_zip_process_doS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto zip = parsePtr(decoder, "zip");
    if (zip == nullptr)
    {
        return s;
    }

    auto outBytes = ngenxxZZipProcessDo(zip);
    return bytes2json(outBytes);
}

bool ngenxx_z_zip_process_finishedS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto zip = parsePtr(decoder, "zip");
    if (zip == nullptr)
    {
        return false;
    }

    return ngenxxZZipProcessFinished(zip);
}

void ngenxx_z_zip_releaseS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    NGenXX::Json::Decoder decoder(json);
    auto zip = parsePtr(decoder, "zip");
    if (zip == nullptr)
    {
        return;
    }

    ngenxxZZipRelease(zip);
}

std::string ngenxx_z_unzip_initS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto bufferSize = parseNum(decoder, "bufferSize");
    auto format = parseNum(decoder, "format");

    auto unzip = ngenxxZUnzipInit(bufferSize, static_cast<NGenXXZFormatX>(format));
    if (unzip == nullptr)
    {
        return s;
    }
    return std::to_string(ptr2addr(unzip));
}

size_t ngenxx_z_unzip_inputS(const char *json)
{
    if (json == nullptr)
    {
        return 0;
    }
    NGenXX::Json::Decoder decoder(json);
    auto unzip = parsePtr(decoder, "unzip");
    if (unzip == nullptr)
    {
        return 0;
    }

    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return 0;
    }

    auto inFinish = static_cast<bool>(parseNum(decoder, "inFinish"));

    return ngenxxZUnzipInput(unzip, in, inFinish);
}

std::string ngenxx_z_unzip_process_doS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto unzip = parsePtr(decoder, "unzip");
    if (unzip == nullptr)
    {
        return s;
    }

    auto outBytes = ngenxxZUnzipProcessDo(unzip);
    return bytes2json(outBytes);
}

bool ngenxx_z_unzip_process_finishedS(const char *json)
{
    if (json == nullptr)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto unzip = parsePtr(decoder, "unzip");
    if (unzip == nullptr)
    {
        return false;
    }

    return ngenxxZUnzipProcessFinished(unzip);
}

void ngenxx_z_unzip_releaseS(const char *json)
{
    if (json == nullptr)
    {
        return;
    }
    NGenXX::Json::Decoder decoder(json);
    auto unzip = parsePtr(decoder, "unzip");
    if (unzip == nullptr)
    {
        return;
    }

    ngenxxZUnzipRelease(unzip);
}

std::string ngenxx_z_bytes_zipS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto mode = parseNum(decoder, "mode");
    auto bufferSize = parseNum(decoder, "bufferSize");
    auto format = parseNum(decoder, "format");
    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return s;
    }

    auto outBytes = ngenxxZBytesZip(in, static_cast<NGenXXZipCompressModeX>(mode), bufferSize, static_cast<NGenXXZFormatX>(format));
    return bytes2json(outBytes);
}

std::string ngenxx_z_bytes_unzipS(const char *json)
{
    std::string s;
    if (json == nullptr)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto bufferSize = parseNum(decoder, "bufferSize");
    auto format = parseNum(decoder, "format");
    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return s;
    }

    auto outBytes = ngenxxZBytesUnzip(in, bufferSize, static_cast<NGenXXZFormatX>(format));
    return bytes2json(outBytes);
}