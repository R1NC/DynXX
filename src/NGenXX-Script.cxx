#include "NGenXX-Script.hxx"

#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <functional>

#include <NGenXX.hxx>
#include <NGenXXNetHttp.h>
#include "json/JsonDecoder.hxx"

std::string bytes2json(const Bytes &bytes)
{
    std::vector<int> intV(bytes.size());
    std::transform(bytes.begin(), bytes.end(), intV.begin(), [](const auto& b) { 
        return static_cast<int>(b); 
    });
    auto cj = bytes.empty() ? cJSON_CreateArray() : cJSON_CreateIntArray(intV.data(), static_cast<int>(intV.size()));
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

double parseNum(NGenXX::Json::Decoder &decoder, const char *k)
{
    return decoder.readNumber(decoder[k]);
}

std::string parseStr(NGenXX::Json::Decoder &decoder, const char *k)
{
    return decoder.readString(decoder[k]);
}

address parseAddress(NGenXX::Json::Decoder &decoder, const char *k)
{
    address a = 0;
    auto s = parseStr(decoder, k);
    if (s.empty() || s == "0")
    {
        return a;
    }
    try
    {
        a = std::stoll(s);
    }
    catch (const std::exception &e)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "parseAddress failed s:{}", s);
    }
    return a;
}

Bytes parseByteArray(NGenXX::Json::Decoder &decoder, const char *bytesK)
{
    Bytes data;
    auto byte_vNode = decoder[bytesK];
    if (byte_vNode)
    {
        decoder.readChildren(byte_vNode, 
                            [&data, &decoder](const size_t idx, const void *const child) -> void
                            {
                                data.push_back(decoder.readNumber(child));
                            });
    }
    return data;
}

std::vector<std::string> parseStrArray(NGenXX::Json::Decoder &decoder, const char *strVK)
{
    std::vector<std::string> v;
    const auto str_vNode = decoder[strVK];
    if (str_vNode)
    {
        decoder.readChildren(str_vNode,
                             [&v, &decoder](const size_t idx, const void *const child) -> void
                             {
                                 v.push_back(std::move(decoder.readString(child)));
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
    if (json == NULL)
    {
        return;
    }
    NGenXX::Json::Decoder decoder(json);
    auto level = parseNum(decoder, "level");
    auto content = parseStr(decoder, "content");
    if (level < 0 || content.length() == 0)
    {
        return;
    }

    ngenxxLogPrint(static_cast<NGenXXLogLevelX>(level), content);
}

#pragma mark Net.Http

std::string ngenxx_net_http_requestS(const char *json)
{
    std::string s;
    if (json == NULL)
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

    auto cFILE = parseAddress(decoder, "cFILE");
    auto fileSize = parseNum(decoder, "file_size");

    auto timeout = parseNum(decoder, "timeout");

    auto header_c = header_v.size();
    auto form_field_count = form_field_name_v.size();
    if (method < 0 || url.length() == 0 || header_c > NGENXX_HTTP_HEADER_MAX_COUNT)
    {
        return s;
    }
    if (form_field_count == 0 && (form_field_name_v.size() > 0 || form_field_mime_v.size() > 0 || form_field_data_v.size() > 0))
    {
        return s;
    }
    if (form_field_count > 0 && (form_field_name_v.size() == 0 || form_field_mime_v.size() == 0 || form_field_data_v.size() == 0))
    {
        return s;
    }
    if ((cFILE > 0 && fileSize == 0) || (cFILE == 0 && fileSize > 0))
    {
        return s;
    }

    auto t = ngenxxNetHttpRequest(url, static_cast<NGenXXHttpMethodX>(method), params, rawBody,
                                  header_v,
                                  form_field_name_v,
                                  form_field_mime_v,
                                  form_field_data_v,
                                  reinterpret_cast<std::FILE *>(cFILE),
                                  fileSize,
                                  timeout);
    return t.toJson();
}

bool ngenxx_net_http_downloadS(const char *json)
{
    if (json == NULL)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto url = parseStr(decoder, "url");
    auto file = parseStr(decoder, "file");
    auto timeout = parseNum(decoder, "timeout");

    if (url.length() == 0 || file.length() == 0)
    {
        return false;
    }

    return ngenxxNetHttpDownload(url, file, timeout);
}

#pragma mark Store.SQLite

std::string ngenxx_store_sqlite_openS(const char *json)
{
    std::string s;
    if (json == NULL)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto _id = parseStr(decoder, "_id");
    if (_id.length() == 0)
    {
        return s;
    }

    auto db = ngenxxStoreSqliteOpen(_id);
    if (db == nullptr)
    {
        return s;
    }
    return std::to_string(reinterpret_cast<address>(db));
}

bool ngenxx_store_sqlite_executeS(const char *json)
{
    if (json == NULL)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parseAddress(decoder, "conn");
    auto sql = parseStr(decoder, "sql");
    if (conn == 0 || sql.length() == 0)
    {
        return false;
    }

    return ngenxxStoreSqliteExecute(reinterpret_cast<void *>(conn), sql);
}

std::string ngenxx_store_sqlite_query_doS(const char *json)
{
    std::string s;
    if (json == NULL)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parseAddress(decoder, "conn");
    auto sql = parseStr(decoder, "sql");
    if (conn == 0 || sql.length() == 0)
    {
        return s;
    }

    auto res = ngenxxStoreSqliteQueryDo(reinterpret_cast<void *>(conn), sql);
    if (res == nullptr)
    {
        return s;
    }
    return std::to_string(reinterpret_cast<address>(res));
}

bool ngenxx_store_sqlite_query_read_rowS(const char *json)
{
    if (json == NULL)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto query_result = parseAddress(decoder, "query_result");
    if (query_result == 0)
    {
        return false;
    }

    return ngenxxStoreSqliteQueryReadRow(reinterpret_cast<void *>(query_result));
}

std::string ngenxx_store_sqlite_query_read_column_textS(const char *json)
{
    std::string s;
    if (json == NULL)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto query_result = parseAddress(decoder, "query_result");
    auto column = parseStr(decoder, "column");
    if (query_result == 0 || column.length() == 0)
    {
        return s;
    }

    return ngenxxStoreSqliteQueryReadColumnText(reinterpret_cast<void *>(query_result), column);
}

int64_t ngenxx_store_sqlite_query_read_column_integerS(const char *json)
{
    if (json == NULL)
    {
        return 0;
    }
    NGenXX::Json::Decoder decoder(json);
    auto query_result = parseAddress(decoder, "query_result");
    auto column = parseStr(decoder, "column");
    if (query_result == 0 || column.length() == 0)
    {
        return 0;
    }

    return ngenxxStoreSqliteQueryReadColumnInteger(reinterpret_cast<void *>(query_result), column);
}

double ngenxx_store_sqlite_query_read_column_floatS(const char *json)
{
    if (json == NULL)
    {
        return 0;
    }
    NGenXX::Json::Decoder decoder(json);
    auto query_result = parseAddress(decoder, "query_result");
    auto column = parseStr(decoder, "column");
    if (query_result == 0 || column.length() == 0)
    {
        return 0;
    }

    return ngenxxStoreSqliteQueryReadColumnFloat(reinterpret_cast<void *>(query_result), column);
}

void ngenxx_store_sqlite_query_dropS(const char *json)
{
    if (json == NULL)
    {
        return;
    }
    NGenXX::Json::Decoder decoder(json);
    auto query_result = parseAddress(decoder, "query_result");
    if (query_result == 0)
    {
        return;
    }

    ngenxxStoreSqliteQueryDrop(reinterpret_cast<void *>(query_result));
}

void ngenxx_store_sqlite_closeS(const char *json)
{
    if (json == NULL)
    {
        return;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parseAddress(decoder, "conn");
    if (conn == 0)
    {
        return;
    }

    ngenxxStoreSqliteClose(reinterpret_cast<void *>(conn));
}

#pragma mark Store.KV

std::string ngenxx_store_kv_openS(const char *json)
{
    std::string s;
    if (json == NULL)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto _id = parseStr(decoder, "_id");
    if (_id.length() == 0)
    {
        return s;
    }

    auto res = ngenxxStoreKvOpen(_id);
    if (res == nullptr)
    {
        return s;
    }
    return std::to_string(reinterpret_cast<address>(res));
}

std::string ngenxx_store_kv_read_stringS(const char *json)
{
    std::string s;
    if (json == NULL)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parseAddress(decoder, "conn");
    auto k = parseStr(decoder, "k");
    if (conn == 0 || k.length() == 0)
    {
        return s;
    }

    return ngenxxStoreKvReadString(reinterpret_cast<void *>(conn), k);
}

bool ngenxx_store_kv_write_stringS(const char *json)
{
    if (json == NULL)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parseAddress(decoder, "conn");
    auto k = parseStr(decoder, "k");
    auto v = parseStr(decoder, "v");
    if (conn == 0 || k.length() == 0)
    {
        return false;
    }

    return ngenxxStoreKvWriteString(reinterpret_cast<void *>(conn), k, v);
}

int64_t ngenxx_store_kv_read_integerS(const char *json)
{
    if (json == NULL)
    {
        return 0;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parseAddress(decoder, "conn");
    auto k = parseStr(decoder, "k");
    if (conn == 0 || k.length() == 0)
    {
        return 0;
    }

    return ngenxxStoreKvReadInteger(reinterpret_cast<void *>(conn), k);
}

bool ngenxx_store_kv_write_integerS(const char *json)
{
    if (json == NULL)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parseAddress(decoder, "conn");
    auto k = parseStr(decoder, "k");
    auto v = parseNum(decoder, "v");
    if (conn == 0 || k.length() == 0)
    {
        return false;
    }

    return ngenxxStoreKvWriteInteger(reinterpret_cast<void *>(conn), k, v);
}

double ngenxx_store_kv_read_floatS(const char *json)
{
    if (json == NULL)
    {
        return 0;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parseAddress(decoder, "conn");
    auto k = parseStr(decoder, "k");
    if (conn == 0 || k.length() == 0)
    {
        return false;
    }

    return ngenxxStoreKvReadFloat(reinterpret_cast<void *>(conn), k);
}

bool ngenxx_store_kv_write_floatS(const char *json)
{
    if (json == NULL)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parseAddress(decoder, "conn");
    auto k = parseStr(decoder, "k");
    auto v = parseNum(decoder, "v");
    if (conn == 0 || k.length() == 0)
    {
        return false;
    }

    return ngenxxStoreKvWriteFloat(reinterpret_cast<void *>(conn), k, v);
}

std::string ngenxx_store_kv_all_keysS(const char *json)
{
    std::string s;
    if (json == NULL)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parseAddress(decoder, "conn");
    if (conn == 0)
    {
        return s;
    }

    auto res = ngenxxStoreKvAllKeys(reinterpret_cast<void *>(conn));
    return strArray2json(res);
}

bool ngenxx_store_kv_containsS(const char *json)
{
    if (json == NULL)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parseAddress(decoder, "conn");
    auto k = parseStr(decoder, "k");
    if (conn == 0 || k.length() == 0)
    {
        return false;
    }

    return ngenxxStoreKvContains(reinterpret_cast<void *>(conn), k);
}

bool ngenxx_store_kv_removeS(const char *json)
{
    if (json == NULL)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parseAddress(decoder, "conn");
    auto k = parseStr(decoder, "k");
    if (conn == 0 || k.length() == 0)
    {
        return false;
    }

    return ngenxxStoreKvRemove(reinterpret_cast<void *>(conn), k);
}

void ngenxx_store_kv_clearS(const char *json)
{
    if (json == NULL)
    {
        return;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parseAddress(decoder, "conn");
    if (conn == 0)
    {
        return;
    }

    ngenxxStoreKvClear(reinterpret_cast<void *>(conn));
}

void ngenxx_store_kv_closeS(const char *json)
{
    if (json == NULL)
    {
        return;
    }
    NGenXX::Json::Decoder decoder(json);
    auto conn = parseAddress(decoder, "conn");
    if (conn == 0)
    {
        return;
    }

    ngenxxStoreKvClose(reinterpret_cast<void *>(conn));
}

#pragma mark Coding

std::string ngenxx_coding_hex_bytes2strS(const char *json)
{
    std::string s;
    if (json == NULL)
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
    if (json == NULL)
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
    if (json == NULL)
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
    if (json == NULL)
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
    if (json == NULL)
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
    if (json == NULL)
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
    if (json == NULL)
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
    if (json == NULL)
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
    if (json == NULL)
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
    if (json == NULL)
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
    if (json == NULL)
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
    if (json == NULL)
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
    if (json == NULL)
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
    if (json == NULL)
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
    if (json == NULL)
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
    if (json == NULL)
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
    return std::to_string(reinterpret_cast<address>(zip));
}

size_t ngenxx_z_zip_inputS(const char *json)
{
    if (json == NULL)
    {
        return 0;
    }
    NGenXX::Json::Decoder decoder(json);
    auto zip = parseAddress(decoder, "zip");
    if (zip == 0)
    {
        return 0;
    }

    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return 0;
    }

    auto inFinish = static_cast<bool>(parseNum(decoder, "inFinish"));

    return ngenxxZZipInput(reinterpret_cast<void *>(zip), in, inFinish);
}

std::string ngenxx_z_zip_process_doS(const char *json)
{
    std::string s;
    if (json == NULL)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto zip = parseAddress(decoder, "zip");
    if (zip == 0)
    {
        return s;
    }

    auto outBytes = ngenxxZZipProcessDo(reinterpret_cast<void *>(zip));
    return bytes2json(outBytes);
}

bool ngenxx_z_zip_process_finishedS(const char *json)
{
    if (json == NULL)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto zip = parseAddress(decoder, "zip");
    if (zip == 0)
    {
        return false;
    }

    return ngenxxZZipProcessFinished(reinterpret_cast<void *>(zip));
}

void ngenxx_z_zip_releaseS(const char *json)
{
    if (json == NULL)
    {
        return;
    }
    NGenXX::Json::Decoder decoder(json);
    auto zip = parseAddress(decoder, "zip");
    if (zip == 0)
    {
        return;
    }

    ngenxxZZipRelease(reinterpret_cast<void *>(zip));
}

std::string ngenxx_z_unzip_initS(const char *json)
{
    std::string s;
    if (json == NULL)
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
    return std::to_string(reinterpret_cast<address>(unzip));
}

size_t ngenxx_z_unzip_inputS(const char *json)
{
    if (json == NULL)
    {
        return 0;
    }
    NGenXX::Json::Decoder decoder(json);
    auto unzip = parseAddress(decoder, "unzip");
    if (unzip == 0)
    {
        return 0;
    }

    auto in = parseByteArray(decoder, "inBytes");
    if (in.empty())
    {
        return 0;
    }

    auto inFinish = static_cast<bool>(parseNum(decoder, "inFinish"));

    return ngenxxZUnzipInput(reinterpret_cast<void *>(unzip), in, inFinish);
}

std::string ngenxx_z_unzip_process_doS(const char *json)
{
    std::string s;
    if (json == NULL)
    {
        return s;
    }
    NGenXX::Json::Decoder decoder(json);
    auto unzip = parseAddress(decoder, "unzip");
    if (unzip == 0)
    {
        return s;
    }

    auto outBytes = ngenxxZUnzipProcessDo(reinterpret_cast<void *>(unzip));
    return bytes2json(outBytes);
}

bool ngenxx_z_unzip_process_finishedS(const char *json)
{
    if (json == NULL)
    {
        return false;
    }
    NGenXX::Json::Decoder decoder(json);
    auto unzip = parseAddress(decoder, "unzip");
    if (unzip == 0)
    {
        return false;
    }

    return ngenxxZUnzipProcessFinished(reinterpret_cast<void *>(unzip));
}

void ngenxx_z_unzip_releaseS(const char *json)
{
    if (json == NULL)
    {
        return;
    }
    NGenXX::Json::Decoder decoder(json);
    auto unzip = parseAddress(decoder, "unzip");
    if (unzip == 0)
    {
        return;
    }

    ngenxxZUnzipRelease(reinterpret_cast<void *>(unzip));
}

std::string ngenxx_z_bytes_zipS(const char *json)
{
    std::string s;
    if (json == NULL)
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
    if (json == NULL)
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