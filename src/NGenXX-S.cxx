#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include "NGenXX-S.hxx"
#include "json/JsonDecoder.hxx"
#include "util/TypeUtil.hxx"
#include "../include/NGenXX.h"
#include "NGenXX-inner.hxx"

const std::string bytes2json(const byte *bytes, const size len)
{
    int x[len];
    for (int i = 0; i < len; i++)
        x[i] = bytes[i];
    auto cj = bytes == NULL || len <= 0 ? cJSON_CreateArray() : cJSON_CreateIntArray(x, len);
    const char *outJson = cJSON_Print(cj);
    return std::string(outJson);
}

const std::string strArray2json(const char **strArray, const unsigned int len)
{
    auto cj = strArray == NULL || len <= 0 ? cJSON_CreateArray() : cJSON_CreateStringArray(strArray, len);
    const char *outJson = cJSON_Print(cj);
    return std::string(outJson);
}

std::vector<byte> parseByteArray(NGenXX::Json::Decoder &decoder, const char *bytesK, const char *lenK)
{
    size len = decoder.readNumber(decoder.readNode(NULL, lenK));
    std::vector<byte> byteV;
    if (len > 0)
    {
        void *byte_vNode = decoder.readNode(NULL, bytesK);
        if (byte_vNode)
        {
            decoder.readChildren(byte_vNode,
                                 [&byteV, &decoder, &len](int idx, void *child) -> void
                                 {
                                     if (idx == len)
                                         return;
                                     byteV.push_back(decoder.readNumber(child));
                                 });
        }
    }
    return byteV;
}

char **parseStrArray(NGenXX::Json::Decoder &decoder, const char *strVK, const size count, const size maxLen)
{
    if (count <= 0)
        return NULL;
    char **strV = reinterpret_cast<char **>(malloc(count * sizeof(char *)));
    void *str_vNode = decoder.readNode(NULL, strVK);
    if (str_vNode)
    {
        decoder.readChildren(str_vNode,
                             [&strV, &decoder, &count, &maxLen](int idx, void *child) -> void
                             {
                                 if (idx == count)
                                     return;
                                 strV[idx] = reinterpret_cast<char *>(malloc(maxLen * sizeof(char) + 1));
                                 strcpy(strV[idx], decoder.readString(child).c_str());
                             });
    }
    return strV;
}

const std::string ngenxx_get_versionS(const char *json)
{
    return ngenxx_get_version();
}

const std::string ngenxx_root_pathS(const char *json)
{
    return ngenxx_root_path();
}

#pragma mark Device.DeviceInfo

int ngenxx_device_typeS(const char *json)
{
    return ngenxx_device_type();
}

const std::string ngenxx_device_nameS(const char *json)
{
    return ngenxx_device_name();
}

const std::string ngenxx_device_manufacturerS(const char *json)
{
    return ngenxx_device_manufacturer();
}

const std::string ngenxx_device_os_versionS(const char *json)
{
    return ngenxx_device_os_version();
}

int ngenxx_device_cpu_archS(const char *json)
{
    return ngenxx_device_cpu_arch();
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

    ngenxx_log_print(level, content.c_str());
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

    size header_c = decoder.readNumber(decoder.readNode(NULL, "header_c"));
    header_c = std::min(header_c, NGENXX_HTTP_HEADER_MAX_COUNT);
    auto header_v = parseStrArray(decoder, "header_v", header_c, NGENXX_HTTP_HEADER_MAX_LENGTH);

    size form_field_count = decoder.readNumber(decoder.readNode(NULL, "form_field_count"));
    form_field_count = std::min(form_field_count, NGENXX_HTTP_FORM_FIELD_MAX_COUNT);
    auto form_field_name_v = parseStrArray(decoder, "form_field_name_v", form_field_count, NGENXX_HTTP_FORM_FIELD_NAME_MAX_LENGTH);
    auto form_field_mime_v = parseStrArray(decoder, "form_field_mime_v", form_field_count, NGENXX_HTTP_FORM_FIELD_MINE_MAX_LENGTH);
    auto form_field_data_v = parseStrArray(decoder, "form_field_data_v", form_field_count, NGENXX_HTTP_FORM_FIELD_DATA_MAX_LENGTH);

    auto cFILE = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "cFILE")));
    const size fileSize = decoder.readNumber(decoder.readNode(NULL, "file_size"));

    const size timeout = decoder.readNumber(decoder.readNode(NULL, "timeout"));

    if (method < 0 || url.length() == 0 || header_c > NGENXX_HTTP_HEADER_MAX_COUNT)
        return s;
    if (form_field_count <= 0 && (form_field_name_v != NULL || form_field_mime_v != NULL || form_field_data_v != NULL))
        return s;
    if (form_field_count > 0 && (form_field_name_v == NULL || form_field_mime_v == NULL || form_field_data_v == NULL))
        return s;
    if ((cFILE > 0 && fileSize <= 0) || (cFILE <= 0 && fileSize > 0))
        return s;

    s = ngenxx_net_http_request(url.c_str(), params.c_str(), method,
                                const_cast<const char**>(header_v), header_c,
                                const_cast<const char**>(form_field_name_v), 
                                const_cast<const char**>(form_field_mime_v), 
                                const_cast<const char**>(form_field_data_v),
                                form_field_count,
                                reinterpret_cast<void *>(cFILE), fileSize,
                                timeout);

    for (int i = 0; i < header_c; i++)
    {
        free(reinterpret_cast<void *>(header_v[i]));
    }
    for (int i = 0; i < form_field_count; i++)
    {
        free(reinterpret_cast<void *>(form_field_name_v[i]));
        free(reinterpret_cast<void *>(form_field_mime_v[i]));
        free(reinterpret_cast<void *>(form_field_data_v[i]));
    }

    return s;
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

    void *db = ngenxx_store_sqlite_open(_id.c_str());
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

    bool res = ngenxx_store_sqlite_execute(reinterpret_cast<void *>(conn), sql.c_str());
    return res;
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

    void *res = ngenxx_store_sqlite_query_do(reinterpret_cast<void *>(conn), sql.c_str());
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

    bool res = ngenxx_store_sqlite_query_read_row(reinterpret_cast<void *>(query_result));

    return res;
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

    s = ngenxx_store_sqlite_query_read_column_text(reinterpret_cast<void *>(query_result), column.c_str());
    return s;
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

    long long res = ngenxx_store_sqlite_query_read_column_integer(reinterpret_cast<void *>(query_result), column.c_str());
    return res;
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

    double res = ngenxx_store_sqlite_query_read_column_float(reinterpret_cast<void *>(query_result), column.c_str());
    return res;
}

void ngenxx_store_sqlite_query_dropS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    auto query_result = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "query_result")));
    if (query_result <= 0)
        return;

    ngenxx_store_sqlite_query_drop(reinterpret_cast<void *>(query_result));
}

void ngenxx_store_sqlite_closeS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    if (conn <= 0)
        return;

    ngenxx_store_sqlite_close(reinterpret_cast<void *>(conn));
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

    void *res = ngenxx_store_kv_open(_id.c_str());
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

    s = ngenxx_store_kv_read_string(reinterpret_cast<void *>(conn), k.c_str());
    return s;
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

    bool res = ngenxx_store_kv_write_string(reinterpret_cast<void *>(conn), k.c_str(), v.c_str());
    return res;
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

    long long res = ngenxx_store_kv_read_integer(reinterpret_cast<void *>(conn), k.c_str());
    return res;
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

    bool res = ngenxx_store_kv_write_integer(reinterpret_cast<void *>(conn), k.c_str(), v);
    return res;
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

    double res = ngenxx_store_kv_read_float(reinterpret_cast<void *>(conn), k.c_str());
    return res;
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

    bool res = ngenxx_store_kv_write_float(reinterpret_cast<void *>(conn), k.c_str(), v);
    return res;
}

const std::string ngenxx_store_kv_all_keysS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<unsigned long long>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    if (conn <= 0)
        return s;

    unsigned int len;
    auto res = ngenxx_store_kv_all_keys(reinterpret_cast<void *>(conn), &len);

    return strArray2json(res, len);
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

    bool res = ngenxx_store_kv_contains(reinterpret_cast<void *>(conn), k.c_str());

    return res;
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

    ngenxx_store_kv_remove(reinterpret_cast<void *>(conn), k.c_str());
}

void ngenxx_store_kv_clearS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    if (conn <= 0)
        return;

    ngenxx_store_kv_clear(reinterpret_cast<void *>(conn));
}

void ngenxx_store_kv_closeS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    auto conn = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "conn")));
    if (conn <= 0)
        return;

    ngenxx_store_kv_close(reinterpret_cast<void *>(conn));
}

#pragma mark Coding

const std::string ngenxx_coding_hex_bytes2strS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.size() == 0)
        return s;

    s = ngenxx_coding_hex_bytes2str(in.data(), in.size());

    return s;
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

    size outLen;
    auto outBytes = ngenxx_coding_hex_str2bytes(str.c_str(), &outLen);
    auto outJson = bytes2json(outBytes, outLen);

    free(reinterpret_cast<void *>(const_cast<byte *>(outBytes)));
    return outJson;
}

#pragma mark Crypto

const std::string ngenxx_crypto_randS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);
    size outLen = decoder.readNumber(decoder.readNode(NULL, "len"));
    if (outLen <= 0)
        return s;
    byte outBytes[outLen];

    ngenxx_crypto_rand(outLen, outBytes);
    auto outJson = bytes2json(outBytes, outLen);

    return outJson;
}

const std::string ngenxx_crypto_aes_encryptS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.size() == 0)
        return s;

    auto key = parseByteArray(decoder, "keyBytes", "keyLen");
    if (key.size() == 0)
        return s;

    size outLen;
    auto outBytes = ngenxx_crypto_aes_encrypt(in.data(), in.size(), key.data(), key.size(), &outLen);
    auto outJson = bytes2json(outBytes, outLen);

    free(reinterpret_cast<void *>(const_cast<byte *>(outBytes)));
    return outJson;
}

const std::string ngenxx_crypto_aes_decryptS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.size() == 0)
        return s;

    auto key = parseByteArray(decoder, "keyBytes", "keyLen");
    if (key.size() == 0)
        return s;

    size outLen;
    auto outBytes = ngenxx_crypto_aes_decrypt(in.data(), in.size(), key.data(), key.size(), &outLen);
    auto outJson = bytes2json(outBytes, outLen);

    free(reinterpret_cast<void *>(const_cast<byte *>(outBytes)));
    return outJson;
}

const std::string ngenxx_crypto_aes_gcm_encryptS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.size() == 0)
        return s;

    auto key = parseByteArray(decoder, "keyBytes", "keyLen");
    if (key.size() == 0)
        return s;

    auto iv = parseByteArray(decoder, "initVectorBytes", "initVectorLen");
    if (iv.size() == 0)
        return s;

    auto aad = parseByteArray(decoder, "aadBytes", "aadLen");

    size tagBits = decoder.readNumber(decoder.readNode(NULL, "tagBits"));

    size outLen;
    auto outBytes = ngenxx_crypto_aes_gcm_encrypt(in.data(), in.size(),
                                                  key.data(), key.size(),
                                                  iv.data(), iv.size(),
                                                  aad.data(), aad.size(),
                                                  tagBits,
                                                  &outLen);
    auto outJson = bytes2json(outBytes, outLen);

    free(reinterpret_cast<void *>(const_cast<byte *>(outBytes)));
    return outJson;
}

const std::string ngenxx_crypto_aes_gcm_decryptS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.size() == 0)
        return s;

    auto key = parseByteArray(decoder, "keyBytes", "keyLen");
    if (key.size() == 0)
        return s;

    auto iv = parseByteArray(decoder, "initVectorBytes", "initVectorLen");
    if (iv.size() == 0)
        return s;

    auto aad = parseByteArray(decoder, "aadBytes", "aadLen");

    size tagBits = decoder.readNumber(decoder.readNode(NULL, "tagBits"));

    size outLen;
    auto outBytes = ngenxx_crypto_aes_gcm_decrypt(in.data(), in.size(),
                                                  key.data(), key.size(),
                                                  iv.data(), iv.size(),
                                                  aad.data(), aad.size(),
                                                  tagBits,
                                                  &outLen);
    auto outJson = bytes2json(outBytes, outLen);

    free(reinterpret_cast<void *>(const_cast<byte *>(outBytes)));
    return outJson;
}

const std::string ngenxx_crypto_hash_md5S(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.size() == 0)
        return s;

    size outLen;
    auto outBytes = ngenxx_crypto_hash_md5(in.data(), in.size(), &outLen);
    auto outJson = bytes2json(outBytes, outLen);

    free(reinterpret_cast<void *>(const_cast<byte *>(outBytes)));
    return outJson;
}

const std::string ngenxx_crypto_hash_sha256S(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.size() == 0)
        return s;

    size outLen;
    auto outBytes = ngenxx_crypto_hash_sha256(in.data(), in.size(), &outLen);
    auto outJson = bytes2json(outBytes, outLen);

    free(reinterpret_cast<void *>(const_cast<byte *>(outBytes)));
    return outJson;
}

const std::string ngenxx_crypto_base64_encodeS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.size() == 0)
        return s;

    size outLen;
    auto outBytes = ngenxx_crypto_base64_encode(in.data(), in.size(), &outLen);
    auto outJson = bytes2json(outBytes, outLen);

    free(reinterpret_cast<void *>(const_cast<byte *>(outBytes)));
    return outJson;
}

const std::string ngenxx_crypto_base64_decodeS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.size() == 0)
        return s;

    size outLen;
    auto outBytes = ngenxx_crypto_base64_decode(in.data(), in.size(), &outLen);
    auto outJson = bytes2json(outBytes, outLen);

    free(reinterpret_cast<void *>(const_cast<byte *>(outBytes)));
    return outJson;
}

#pragma mark Zip

const address ngenxx_z_zip_initS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    int mode = decoder.readNumber(decoder.readNode(NULL, "mode"));
    size bufferSize = decoder.readNumber(decoder.readNode(NULL, "bufferSize"));
    int format = decoder.readNumber(decoder.readNode(NULL, "format"));

    void *zip = ngenxx_z_zip_init(mode, bufferSize, format);

    return reinterpret_cast<address>(zip);
}

const size ngenxx_z_zip_inputS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    auto zip = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "zip")));
    if (zip <= 0)
        return 0;

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.size() == 0)
        return 0;

    auto inFinish = static_cast<bool>(decoder.readNumber(decoder.readNode(NULL, "inFinish")));

    return ngenxx_z_zip_input(reinterpret_cast<void *>(zip), in.data(), in.size(), inFinish);
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

    size outLen;
    auto outBytes = ngenxx_z_zip_process_do(reinterpret_cast<void *>(zip), &outLen);
    auto outJson = bytes2json(outBytes, outLen);

    free(reinterpret_cast<void *>(const_cast<byte *>(outBytes)));
    return outJson;
}

bool ngenxx_z_zip_process_finishedS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    auto zip = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "zip")));
    if (zip <= 0)
        return false;

    return ngenxx_z_zip_process_finished(reinterpret_cast<void *>(zip));
}

void ngenxx_z_zip_releaseS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    auto zip = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "zip")));
    if (zip <= 0)
        return;

    ngenxx_z_zip_release(reinterpret_cast<void *>(zip));
}

const address ngenxx_z_unzip_initS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    size bufferSize = decoder.readNumber(decoder.readNode(NULL, "bufferSize"));
    int format = decoder.readNumber(decoder.readNode(NULL, "format"));

    void *zip = ngenxx_z_unzip_init(bufferSize, format);

    return reinterpret_cast<address>(zip);
}

const size ngenxx_z_unzip_inputS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    auto unzip = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "unzip")));
    if (unzip <= 0)
        return 0;

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.size() == 0)
        return 0;

    auto inFinish = static_cast<bool>(decoder.readNumber(decoder.readNode(NULL, "inFinish")));

    return ngenxx_z_unzip_input(reinterpret_cast<void *>(unzip), in.data(), in.size(), inFinish);
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

    size outLen;
    auto outBytes = ngenxx_z_unzip_process_do(reinterpret_cast<void *>(unzip), &outLen);
    auto outJson = bytes2json(outBytes, outLen);

    free(reinterpret_cast<void *>(const_cast<byte *>(outBytes)));
    return outJson;
}

bool ngenxx_z_unzip_process_finishedS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    auto unzip = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "unzip")));
    if (unzip <= 0)
        return false;

    return ngenxx_z_unzip_process_finished(reinterpret_cast<void *>(unzip));
}

void ngenxx_z_unzip_releaseS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    auto unzip = static_cast<address>(decoder.readNumber(decoder.readNode(NULL, "unzip")));
    if (unzip <= 0)
        return;

    ngenxx_z_unzip_release(reinterpret_cast<void *>(unzip));
}

const std::string ngenxx_z_bytes_zipS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);
    int mode = decoder.readNumber(decoder.readNode(NULL, "mode"));
    size bufferSize = decoder.readNumber(decoder.readNode(NULL, "bufferSize"));
    int format = decoder.readNumber(decoder.readNode(NULL, "format"));
    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.size() == 0)
        return s;

    size outLen;
    auto outBytes = ngenxx_z_bytes_zip(mode, bufferSize, format, in.data(), in.size(), &outLen);
    auto outJson = bytes2json(outBytes, outLen);

    free(reinterpret_cast<void *>(const_cast<byte *>(outBytes)));
    return outJson;
}

const std::string ngenxx_z_bytes_unzipS(const char *json)
{
    std::string s;
    if (json == NULL)
        return s;
    NGenXX::Json::Decoder decoder(json);
    size bufferSize = decoder.readNumber(decoder.readNode(NULL, "bufferSize"));
    int format = decoder.readNumber(decoder.readNode(NULL, "format"));
    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.size() == 0)
        return s;

    size outLen;
    auto outBytes = ngenxx_z_bytes_unzip(bufferSize, format, in.data(), in.size(), &outLen);
    auto outJson = bytes2json(outBytes, outLen);

    free(reinterpret_cast<void *>(const_cast<byte *>(outBytes)));
    return outJson;
}