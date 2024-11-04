#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <vector>

#include "../include/NGenXX.hxx"
#include "../include/NGenXXNetHttp.h"
#include "NGenXX-Script.hxx"
#include "json/JsonDecoder.hxx"

const std::string bytes2json(Bytes bytes)
{
    auto [data, len] = bytes;
    int x[len];
    for (int i = 0; i < len; i++)
        x[i] = data[i];
    auto cj = data == NULL || len <= 0 ? cJSON_CreateArray() : cJSON_CreateIntArray(x, len);
    const char *outJson = cJSON_Print(cj);
    return std::string(outJson);
}

const std::string strArray2json(const std::vector<std::string> &v)
{
    auto cj = cJSON_CreateArray();
    for (auto it = v.begin(); it != v.end(); ++it)
    {
        cJSON_AddItemToArray(cj, cJSON_CreateString(it->c_str()));
    }
    const char *outJson = cJSON_Print(cj);
    return std::string(outJson);
}

Bytes parseByteArray(NGenXX::Json::Decoder &decoder, const char *bytesK, const char *lenK)
{
    size_t len = decoder.readNumber(decoder.readNode(NULL, lenK));
    byte data[len];
    if (len > 0)
    {
        void *byte_vNode = decoder.readNode(NULL, bytesK);
        if (byte_vNode)
        {
            decoder.readChildren(byte_vNode,
                                 [len, &data, &decoder](int idx, void *child) -> void
                                 {
                                     if (idx == len)
                                         return;
                                     data[idx] = decoder.readNumber(child);
                                 });
        }
    }
    return {data, len};
}

const std::vector<std::string> parseStrArray(NGenXX::Json::Decoder &decoder, const char *strVK, const size_t count, const size_t maxLen)
{
    std::vector<std::string> v;
    if (count <= 0)
        return v;
    char **strV = reinterpret_cast<char **>(malloc(count * sizeof(char *)));
    void *str_vNode = decoder.readNode(NULL, strVK);
    if (str_vNode)
    {
        decoder.readChildren(str_vNode,
                             [count, maxLen, &v, &decoder](int idx, void *child) -> void
                             {
                                 if (idx == count)
                                     return;
                                 v.push_back(decoder.readString(child));
                             });
    }
    return v;
}

const std::string ngenxx_get_versionS(const char *json)
{
    return ngenxxGetVersion();
}

const std::string ngenxx_root_pathS(const char *json)
{
    return ngenxxRootPath();
}

#pragma mark Device.DeviceInfo

int ngenxx_device_typeS(const char *json)
{
    return static_cast<int>(ngenxxDeviceType());
}

const std::string ngenxx_device_nameS(const char *json)
{
    return ngenxxDeviceName();
}

const std::string ngenxx_device_manufacturerS(const char *json)
{
    return ngenxxDeviceManufacturer();
}

const std::string ngenxx_device_os_versionS(const char *json)
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
        return;
    NGenXX::Json::Decoder decoder(json);
    int level = decoder.readNumber(decoder.readNode(NULL, "level"));
    auto content = decoder.readString(decoder.readNode(NULL, "content"));
    if (level < 0 || content.length() == 0)
        return;

    ngenxxLogPrint(static_cast<NGenXXLogLevelX>(level), content);
}

#pragma mark Net.Http

const std::string ngenxx_net_http_requestS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);
    auto url = decoder.readString(decoder.readNode(NULL, "url"));
    auto params = decoder.readString(decoder.readNode(NULL, "params"));
    const int method = decoder.readNumber(decoder.readNode(NULL, "method"));

    size_t header_c = decoder.readNumber(decoder.readNode(NULL, "header_c"));
    header_c = std::min(header_c, NGENXX_HTTP_HEADER_MAX_COUNT);
    auto header_v = parseStrArray(decoder, "header_v", header_c, NGENXX_HTTP_HEADER_MAX_LENGTH);

    size_t form_field_count = decoder.readNumber(decoder.readNode(NULL, "form_field_count"));
    form_field_count = std::min(form_field_count, NGENXX_HTTP_FORM_FIELD_MAX_COUNT);
    auto form_field_name_v = parseStrArray(decoder, "form_field_name_v", form_field_count, NGENXX_HTTP_FORM_FIELD_NAME_MAX_LENGTH);
    auto form_field_mime_v = parseStrArray(decoder, "form_field_mime_v", form_field_count, NGENXX_HTTP_FORM_FIELD_MINE_MAX_LENGTH);
    auto form_field_data_v = parseStrArray(decoder, "form_field_data_v", form_field_count, NGENXX_HTTP_FORM_FIELD_DATA_MAX_LENGTH);

    auto cFILE = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "cFILE")));
    const size_t fileSize = decoder.readNumber(decoder.readNode(NULL, "file_size"));

    const size_t timeout = decoder.readNumber(decoder.readNode(NULL, "timeout"));

    if (method < 0 || url.length() == 0 || header_c > NGENXX_HTTP_HEADER_MAX_COUNT)
        return s;
    if (form_field_count <= 0 && (form_field_name_v.size() > 0 || form_field_mime_v.size() > 0 || form_field_data_v.size() > 0))
        return s;
    if (form_field_count > 0 && (form_field_name_v.size() == 0 || form_field_mime_v.size() == 0 || form_field_data_v.size() == 0))
        return s;
    if ((cFILE > 0 && fileSize <= 0) || (cFILE <= 0 && fileSize > 0))
        return s;

    return ngenxxNetHttpRequest(url, static_cast<NGenXXHttpMethodX>(method), params, 
                                header_v,
                                form_field_name_v,
                                form_field_mime_v,
                                form_field_data_v,
                                reinterpret_cast<std::FILE *>(cFILE),
                                fileSize,
                                timeout);
}

#pragma mark Store.SQLite

const address ngenxx_store_sqlite_openS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    auto _id = decoder.readString(decoder.readNode(NULL, "_id"));
    if (_id.length() == 0)
        return 0;

    void *db = ngenxxStoreSqliteOpen(_id);
    return reinterpret_cast<address>(db);
}

bool ngenxx_store_sqlite_executeS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    auto sql = decoder.readString(decoder.readNode(NULL, "sql"));
    if (conn <= 0 || sql.length() == 0)
        return false;

    return ngenxxStoreSqliteExecute(reinterpret_cast<void *>(conn), sql);
}

const address ngenxx_store_sqlite_query_doS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    auto sql = decoder.readString(decoder.readNode(NULL, "sql"));
    if (conn <= 0 || sql.length() == 0)
        return 0;

    void *res = ngenxxStoreSqliteQueryDo(reinterpret_cast<void *>(conn), sql);
    return reinterpret_cast<address>(res);
}

bool ngenxx_store_sqlite_query_read_rowS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    auto query_result = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "query_result")));
    if (query_result <= 0)
        return false;

    return ngenxxStoreSqliteQueryReadRow(reinterpret_cast<void *>(query_result));
}

const std::string ngenxx_store_sqlite_query_read_column_textS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);
    auto query_result = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "query_result")));
    auto column = decoder.readString(decoder.readNode(NULL, "column"));
    if (query_result <= 0 || column.length() == 0)
        return s;

    return ngenxxStoreSqliteQueryReadColumnText(reinterpret_cast<void *>(query_result), column);
}

long long ngenxx_store_sqlite_query_read_column_integerS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    auto query_result = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "query_result")));
    auto column = decoder.readString(decoder.readNode(NULL, "column"));
    if (query_result <= 0 || column.length() == 0)
        return 0;

    return ngenxxStoreSqliteQueryReadColumnInteger(reinterpret_cast<void *>(query_result), column);
}

double ngenxx_store_sqlite_query_read_column_floatS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    auto query_result = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "query_result")));
    auto column = decoder.readString(decoder.readNode(NULL, "column"));
    if (query_result <= 0 || column.length() == 0)
        return 0;

    return ngenxxStoreSqliteQueryReadColumnFloat(reinterpret_cast<void *>(query_result), column);
}

void ngenxx_store_sqlite_query_dropS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    auto query_result = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "query_result")));
    if (query_result <= 0)
        return;

    ngenxxStoreSqliteQueryDrop(reinterpret_cast<void *>(query_result));
}

void ngenxx_store_sqlite_closeS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    if (conn <= 0)
        return;

    ngenxxStoreSqliteClose(reinterpret_cast<void *>(conn));
}

#pragma mark Store.KV

const address ngenxx_store_kv_openS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    auto _id = decoder.readString(decoder.readNode(NULL, "_id"));
    if (_id.length() == 0)
        return 0;

    void *res = ngenxxStoreKvOpen(_id);
    return reinterpret_cast<address>(res);
}

const std::string ngenxx_store_kv_read_stringS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    auto k = decoder.readString(decoder.readNode(NULL, "k"));
    if (conn <= 0 || k.length() == 0)
        return s;

    return ngenxxStoreKvReadString(reinterpret_cast<void *>(conn), k);
}

bool ngenxx_store_kv_write_stringS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    auto k = decoder.readString(decoder.readNode(NULL, "k"));
    auto v = decoder.readString(decoder.readNode(NULL, "v"));
    if (conn <= 0 || k.length() == 0)
        return false;

    return ngenxxStoreKvWriteString(reinterpret_cast<void *>(conn), k, v);
}

long long ngenxx_store_kv_read_integerS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    auto k = decoder.readString(decoder.readNode(NULL, "k"));
    if (conn <= 0 || k.length() == 0)
        return 0;

    return ngenxxStoreKvReadInteger(reinterpret_cast<void *>(conn), k);
}

bool ngenxx_store_kv_write_integerS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    auto k = decoder.readString(decoder.readNode(NULL, "k"));
    long long v = decoder.readNumber(decoder.readNode(NULL, "v"));
    if (conn <= 0 || k.length() == 0)
        return false;

    return ngenxxStoreKvWriteInteger(reinterpret_cast<void *>(conn), k, v);
}

double ngenxx_store_kv_read_floatS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    auto k = decoder.readString(decoder.readNode(NULL, "k"));
    if (conn <= 0 || k.length() == 0)
        return false;

    return ngenxxStoreKvReadFloat(reinterpret_cast<void *>(conn), k);
}

bool ngenxx_store_kv_write_floatS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    auto k = decoder.readString(decoder.readNode(NULL, "k"));
    double v = decoder.readNumber(decoder.readNode(NULL, "v"));
    if (conn <= 0 || k.length() == 0)
        return false;

    return ngenxxStoreKvWriteFloat(reinterpret_cast<void *>(conn), k, v);
}

const std::string ngenxx_store_kv_all_keysS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    if (conn <= 0)
        return s;

    auto res = ngenxxStoreKvAllKeys(reinterpret_cast<void *>(conn));
    return strArray2json(res);
}

bool ngenxx_store_kv_containsS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    auto k = decoder.readString(decoder.readNode(NULL, "k"));
    if (conn <= 0 || k.length() == 0)
        return false;

    return ngenxxStoreKvContains(reinterpret_cast<void *>(conn), k);
}

void ngenxx_store_kv_removeS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    auto k = decoder.readString(decoder.readNode(NULL, "k"));
    if (conn <= 0 || k.length() == 0)
        return;

    ngenxxStoreKvRemove(reinterpret_cast<void *>(conn), k);
}

void ngenxx_store_kv_clearS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    if (conn <= 0)
        return;

    ngenxxStoreKvClear(reinterpret_cast<void *>(conn));
}

void ngenxx_store_kv_closeS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    if (conn <= 0)
        return;

    ngenxxStoreKvClose(reinterpret_cast<void *>(conn));
}

#pragma mark Coding

const std::string ngenxx_coding_hex_bytes2strS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.second == 0)
        return s;

    return ngenxxCodingHexBytes2str(in);
}

const std::string ngenxx_coding_hex_str2bytesS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto str = decoder.readString(decoder.readNode(NULL, "str"));

    if (str.size() == 0)
        return s;

    auto bytes = ngenxxCodingHexStr2bytes(str);
    return bytes2json(bytes);
}

const std::string ngenxx_coding_bytes2strS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.second == 0)
        return s;

    return ngenxxCodingBytes2str(in);
}

const std::string ngenxx_coding_str2bytesS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto str = decoder.readString(decoder.readNode(NULL, "str"));

    if (str.size() == 0)
        return s;

    auto bytes = ngenxxCodingStr2bytes(str);
    return bytes2json(bytes);
}

#pragma mark Crypto

const std::string ngenxx_crypto_randS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);
    size_t outLen = decoder.readNumber(decoder.readNode(NULL, "len"));
    if (outLen <= 0)
        return s;
    byte outBytes[outLen];

    ngenxxCryptoRand(outLen, outBytes);
    return bytes2json({outBytes, outLen});
}

const std::string ngenxx_crypto_aes_encryptS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.second == 0)
        return s;

    auto key = parseByteArray(decoder, "keyBytes", "keyLen");
    if (key.second == 0)
        return s;

    auto outBytes = ngenxxCryptoAesEncrypt(in, key);
    return bytes2json(outBytes);
}

const std::string ngenxx_crypto_aes_decryptS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.second == 0)
        return s;

    auto key = parseByteArray(decoder, "keyBytes", "keyLen");
    if (key.second == 0)
        return s;

    auto outBytes = ngenxxCryptoAesDecrypt(in, key);
    return bytes2json(outBytes);
}

const std::string ngenxx_crypto_aes_gcm_encryptS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.second == 0)
        return s;

    auto key = parseByteArray(decoder, "keyBytes", "keyLen");
    if (key.second == 0)
        return s;

    auto iv = parseByteArray(decoder, "initVectorBytes", "initVectorLen");
    if (iv.second == 0)
        return s;

    auto aad = parseByteArray(decoder, "aadBytes", "aadLen");

    size_t tagBits = decoder.readNumber(decoder.readNode(NULL, "tagBits"));

    auto outBytes = ngenxxCryptoAesGcmEncrypt(in, key, iv, tagBits, aad);
    return bytes2json(outBytes);
}

const std::string ngenxx_crypto_aes_gcm_decryptS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.second == 0)
        return s;

    auto key = parseByteArray(decoder, "keyBytes", "keyLen");
    if (key.second == 0)
        return s;

    auto iv = parseByteArray(decoder, "initVectorBytes", "initVectorLen");
    if (iv.second == 0)
        return s;

    auto aad = parseByteArray(decoder, "aadBytes", "aadLen");

    size_t tagBits = decoder.readNumber(decoder.readNode(NULL, "tagBits"));

    auto outBytes = ngenxxCryptoAesGcmDecrypt(in, key, iv, tagBits, aad);
    return bytes2json(outBytes);
}

const std::string ngenxx_crypto_hash_md5S(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.second == 0)
        return s;

    auto outBytes = ngenxxCryptoHashMd5(in);
    return bytes2json(outBytes);
}

const std::string ngenxx_crypto_hash_sha256S(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.second == 0)
        return s;

    auto outBytes = ngenxxCryptoHashSha256(in);
    return bytes2json(outBytes);
}

const std::string ngenxx_crypto_base64_encodeS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.second == 0)
        return s;

    auto outBytes = ngenxxCryptoBase64Encode(in);
    return bytes2json(outBytes);
}

const std::string ngenxx_crypto_base64_decodeS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.second == 0)
        return s;

    auto outBytes = ngenxxCryptoBase64Decode(in);
    return bytes2json(outBytes);
}

#pragma mark Zip

const address ngenxx_z_zip_initS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    int mode = decoder.readNumber(decoder.readNode(NULL, "mode"));
    size_t bufferSize = decoder.readNumber(decoder.readNode(NULL, "bufferSize"));
    int format = decoder.readNumber(decoder.readNode(NULL, "format"));

    void *zip = ngenxxZZipInit(static_cast<NGenXXZipCompressModeX>(mode), bufferSize, static_cast<NGenXXZFormatX>(format));

    return reinterpret_cast<address>(zip);
}

const size_t ngenxx_z_zip_inputS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    auto zip = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "zip")));
    if (zip <= 0)
        return 0;

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.second == 0)
        return 0;

    auto inFinish = static_cast<bool>(decoder.readNumber(decoder.readNode(NULL, "inFinish")));

    return ngenxxZZipInput(reinterpret_cast<void *>(zip), in, inFinish);
}

const std::string ngenxx_z_zip_process_doS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);
    auto zip = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "zip")));
    if (zip <= 0)
        return s;

    auto outBytes = ngenxxZZipProcessDo(reinterpret_cast<void *>(zip));
    return bytes2json(outBytes);
}

bool ngenxx_z_zip_process_finishedS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    auto zip = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "zip")));
    if (zip <= 0)
        return false;

    return ngenxxZZipProcessFinished(reinterpret_cast<void *>(zip));
}

void ngenxx_z_zip_releaseS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    auto zip = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "zip")));
    if (zip <= 0)
        return;

    ngenxxZZipRelease(reinterpret_cast<void *>(zip));
}

const address ngenxx_z_unzip_initS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    size_t bufferSize = decoder.readNumber(decoder.readNode(NULL, "bufferSize"));
    int format = decoder.readNumber(decoder.readNode(NULL, "format"));

    void *zip = ngenxxZUnzipInit(bufferSize, static_cast<NGenXXZFormatX>(format));
    return reinterpret_cast<address>(zip);
}

const size_t ngenxx_z_unzip_inputS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    auto unzip = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "unzip")));
    if (unzip <= 0)
        return 0;

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.second == 0)
        return 0;

    auto inFinish = static_cast<bool>(decoder.readNumber(decoder.readNode(NULL, "inFinish")));

    return ngenxxZUnzipInput(reinterpret_cast<void *>(unzip), in, inFinish);
}

const std::string ngenxx_z_unzip_process_doS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);
    auto unzip = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "unzip")));
    if (unzip <= 0)
        return s;

    auto outBytes = ngenxxZUnzipProcessDo(reinterpret_cast<void *>(unzip));
    return bytes2json(outBytes);
}

bool ngenxx_z_unzip_process_finishedS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    auto unzip = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "unzip")));
    if (unzip <= 0)
        return false;

    return ngenxxZUnzipProcessFinished(reinterpret_cast<void *>(unzip));
}

void ngenxx_z_unzip_releaseS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    auto unzip = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "unzip")));
    if (unzip <= 0)
        return;

    ngenxxZUnzipRelease(reinterpret_cast<void *>(unzip));
}

const std::string ngenxx_z_bytes_zipS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);
    int mode = decoder.readNumber(decoder.readNode(NULL, "mode"));
    size_t bufferSize = decoder.readNumber(decoder.readNode(NULL, "bufferSize"));
    int format = decoder.readNumber(decoder.readNode(NULL, "format"));
    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.second == 0)
        return s;

    auto outBytes = ngenxxZBytesZip(in, static_cast<NGenXXZipCompressModeX>(mode), bufferSize, static_cast<NGenXXZFormatX>(format));
    return bytes2json(outBytes);
}

const std::string ngenxx_z_bytes_unzipS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);
    size_t bufferSize = decoder.readNumber(decoder.readNode(NULL, "bufferSize"));
    int format = decoder.readNumber(decoder.readNode(NULL, "format"));
    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.second == 0)
        return s;

    auto outBytes = ngenxxZBytesUnzip(in, bufferSize, static_cast<NGenXXZFormatX>(format));
    return bytes2json(outBytes);
}