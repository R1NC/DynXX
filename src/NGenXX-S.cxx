#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include "NGenXX-S.hxx"
#include "json/JsonDecoder.hxx"
#include "util/TypeUtil.hxx"
#include "../include/NGenXX.h"
#include "NGenXX-inner.hxx"

const char *bytes2json(const byte *bytes, const size len)
{
    int x[len];
    for (int i = 0; i < len; i++)
        x[i] = bytes[i];
    auto cj = bytes == NULL || len <= 0 ? cJSON_CreateArray() : cJSON_CreateIntArray(x, len);
    const char *outJson = cJSON_Print(cj);
    char *s = (char *)malloc(strlen(outJson));
    strcpy(s, outJson);
    return s;
}

NGenXX::Bytes parseByteArray(NGenXX::Json::Decoder &decoder, const char *bytesK, const char *lenK)
{
    size len = decoder.readNumber(decoder.readNode(NULL, lenK));
    byte *byteV = NULL;
    if (len > 0)
    {
        byteV = (byte *)malloc(len * sizeof(byte));
        void *byte_vNode = decoder.readNode(NULL, bytesK);
        if (byte_vNode)
        {
            decoder.readChildren(byte_vNode,
                                 [&byteV, &decoder, &len](int idx, void *child) -> void
                                 {
                                     if (idx == len)
                                         return;
                                     byteV[idx] = decoder.readNumber(child);
                                 });
        }
    }
    return {byteV, len};
}

char **parseStrArray(NGenXX::Json::Decoder &decoder, const char *strVK, const size count, const size maxLen)
{
    char **strV = NULL;
    if (count > 0)
    {
        strV = (char **)malloc(count * sizeof(char *));
        void *str_vNode = decoder.readNode(NULL, strVK);
        if (str_vNode)
        {
            decoder.readChildren(str_vNode,
                                 [&strV, &decoder, &count, &maxLen](int idx, void *child) -> void
                                 {
                                     if (idx == count)
                                         return;
                                     strV[idx] = (char *)malloc(maxLen * sizeof(char) + 1);
                                     strcpy(strV[idx], decoder.readString(child).c_str());
                                 });
        }
    }
    return strV;
}

const char *ngenxx_get_versionS(const char *json)
{
    return ngenxx_get_version();
}

#pragma mark Device.DeviceInfo

int ngenxx_device_typeS(const char *json)
{
    return ngenxx_device_type();
}

const char *ngenxx_device_nameS(const char *json)
{
    return ngenxx_device_name();
}

const char *ngenxx_device_manufacturerS(const char *json)
{
    return ngenxx_device_manufacturer();
}

const char *ngenxx_device_os_versionS(const char *json)
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
    const char *content = str2charp(decoder.readString(decoder.readNode(NULL, "content")));
    if (level < 0 || content == NULL)
        return;

    ngenxx_log_print(level, content);
}

#pragma mark Net.Http

const char *ngenxx_net_http_requestS(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);
    const char *url = str2charp(decoder.readString(decoder.readNode(NULL, "url")));
    const char *params = str2charp(decoder.readString(decoder.readNode(NULL, "params")));
    const int method = decoder.readNumber(decoder.readNode(NULL, "method"));

    size header_c = decoder.readNumber(decoder.readNode(NULL, "header_c"));
    header_c = std::min(header_c, NGENXX_HTTP_HEADER_MAX_COUNT);
    char **header_v = parseStrArray(decoder, "header_v", header_c, NGENXX_HTTP_HEADER_MAX_LENGTH);

    size form_field_count = decoder.readNumber(decoder.readNode(NULL, "form_field_count"));
    form_field_count = std::min(form_field_count, NGENXX_HTTP_FORM_FIELD_MAX_COUNT);
    char **form_field_name_v = parseStrArray(decoder, "form_field_name_v", form_field_count, NGENXX_HTTP_FORM_FIELD_NAME_MAX_LENGTH);
    char **form_field_mime_v = parseStrArray(decoder, "form_field_mime_v", form_field_count, NGENXX_HTTP_FORM_FIELD_MINE_MAX_LENGTH);
    char **form_field_data_v = parseStrArray(decoder, "form_field_data_v", form_field_count, NGENXX_HTTP_FORM_FIELD_DATA_MAX_LENGTH);

    const unsigned long cFILE = decoder.readNumber(decoder.readNode(NULL, "cFILE"));
    const size fileSize = decoder.readNumber(decoder.readNode(NULL, "file_size"));

    const size timeout = decoder.readNumber(decoder.readNode(NULL, "timeout"));

    if (method < 0 || url == NULL || header_c > NGENXX_HTTP_HEADER_MAX_COUNT)
        return NULL;
    if (form_field_count <= 0 && (form_field_name_v != NULL || form_field_mime_v != NULL || form_field_data_v != NULL))
        return NULL;
    if (form_field_count > 0 && (form_field_name_v == NULL || form_field_mime_v == NULL || form_field_data_v == NULL))
        return NULL;
    if ((cFILE > 0 && fileSize <= 0) || (cFILE <= 0 && fileSize > 0))
        return NULL;

    const char *res = ngenxx_net_http_request(url, params, method,
                                              (const char **)header_v, (const size)header_c,
                                              (const char **)form_field_name_v, (const char **)form_field_mime_v, (const char **)form_field_data_v, (const size)form_field_count,
                                              (void *)cFILE, fileSize,
                                              timeout);

    free((void *)url);
    free((void *)params);
    for (int i = 0; i < header_c; i++)
    {
        free((void *)header_v[i]);
    }
    for (int i = 0; i < form_field_count; i++)
    {
        free((void *)form_field_name_v[i]);
        free((void *)form_field_mime_v[i]);
        free((void *)form_field_data_v[i]);
    }

    return res;
}

#pragma mark Store.SQLite

void *ngenxx_store_sqlite_openS(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);
    const char *_id = str2charp(decoder.readString(decoder.readNode(NULL, "_id")));
    if (_id == NULL)
        return NULL;

    void *db = ngenxx_store_sqlite_open(_id);

    free((void *)_id);
    return db;
}

bool ngenxx_store_sqlite_executeS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *sql = str2charp(decoder.readString(decoder.readNode(NULL, "sql")));
    if (conn <= 0 || sql == NULL)
        return false;

    bool res = ngenxx_store_sqlite_execute((void *)conn, sql);

    free((void *)sql);
    return res;
}

void *ngenxx_store_sqlite_query_doS(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *sql = str2charp(decoder.readString(decoder.readNode(NULL, "sql")));
    if (conn <= 0 || sql == NULL)
        return NULL;

    void *res = ngenxx_store_sqlite_query_do((void *)conn, sql);

    free((void *)sql);
    return res;
}

bool ngenxx_store_sqlite_query_read_rowS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    long query_result = decoder.readNumber(decoder.readNode(NULL, "query_result"));
    if (query_result <= 0)
        return false;

    bool res = ngenxx_store_sqlite_query_read_row((void *)query_result);

    return res;
}

const char *ngenxx_store_sqlite_query_read_column_textS(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);
    long query_result = decoder.readNumber(decoder.readNode(NULL, "query_result"));
    const char *column = str2charp(decoder.readString(decoder.readNode(NULL, "column")));
    if (query_result <= 0 || column == NULL)
        return NULL;

    const char *res = ngenxx_store_sqlite_query_read_column_text((void *)query_result, column);

    free((void *)column);
    return res;
}

long long ngenxx_store_sqlite_query_read_column_integerS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    long query_result = decoder.readNumber(decoder.readNode(NULL, "query_result"));
    const char *column = str2charp(decoder.readString(decoder.readNode(NULL, "column")));
    if (query_result <= 0 || column == NULL)
        return 0;

    long long res = ngenxx_store_sqlite_query_read_column_integer((void *)query_result, column);

    free((void *)column);
    return res;
}

double ngenxx_store_sqlite_query_read_column_floatS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    long query_result = decoder.readNumber(decoder.readNode(NULL, "query_result"));
    const char *column = str2charp(decoder.readString(decoder.readNode(NULL, "column")));
    if (query_result <= 0 || column == NULL)
        return 0;

    double res = ngenxx_store_sqlite_query_read_column_float((void *)query_result, column);

    free((void *)column);
    return res;
}

void ngenxx_store_sqlite_query_dropS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    long query_result = decoder.readNumber(decoder.readNode(NULL, "query_result"));
    if (query_result <= 0)
        return;

    ngenxx_store_sqlite_query_drop((void *)query_result);
}

void ngenxx_store_sqlite_closeS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    if (conn <= 0)
        return;

    ngenxx_store_sqlite_close((void *)conn);
}

#pragma mark Store.KV

void *ngenxx_store_kv_openS(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);
    const char *_id = str2charp(decoder.readString(decoder.readNode(NULL, "_id")));
    if (_id == NULL)
        return NULL;

    void *res = ngenxx_store_kv_open(_id);

    free((void *)_id);
    return res;
}

const char *ngenxx_store_kv_read_stringS(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *k = str2charp(decoder.readString(decoder.readNode(NULL, "k")));
    if (conn <= 0 || k == NULL)
        return NULL;

    const char *res = ngenxx_store_kv_read_string((void *)conn, k);

    free((void *)k);
    return res;
}

bool ngenxx_store_kv_write_stringS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *k = str2charp(decoder.readString(decoder.readNode(NULL, "k")));
    const char *v = str2charp(decoder.readString(decoder.readNode(NULL, "v")));
    if (conn <= 0 || k == NULL)
        return false;

    bool res = ngenxx_store_kv_write_string((void *)conn, k, v);

    free((void *)k);
    free((void *)v);
    return res;
}

long long ngenxx_store_kv_read_integerS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *k = str2charp(decoder.readString(decoder.readNode(NULL, "k")));
    if (conn <= 0 || k == NULL)
        return 0;

    long long res = ngenxx_store_kv_read_integer((void *)conn, k);

    free((void *)k);
    return res;
}

bool ngenxx_store_kv_write_integerS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *k = str2charp(decoder.readString(decoder.readNode(NULL, "k")));
    long long v = decoder.readNumber(decoder.readNode(NULL, "v"));
    if (conn <= 0 || k == NULL)
        return false;

    bool res = ngenxx_store_kv_write_integer((void *)conn, k, v);

    free((void *)k);
    return res;
}

double ngenxx_store_kv_read_floatS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *k = str2charp(decoder.readString(decoder.readNode(NULL, "k")));
    if (conn <= 0 || k == NULL)
        return false;

    double res = ngenxx_store_kv_read_float((void *)conn, k);

    free((void *)k);
    return res;
}

double ngenxx_store_kv_write_floatS(const char *json)
{
    if (json == NULL)
        return 0;
    NGenXX::Json::Decoder decoder(json);
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *k = str2charp(decoder.readString(decoder.readNode(NULL, "k")));
    double v = decoder.readNumber(decoder.readNode(NULL, "v"));
    if (conn <= 0 || k == NULL)
        return 0;

    bool res = ngenxx_store_kv_write_float((void *)conn, k, v);

    free((void *)k);
    return res;
}

bool ngenxx_store_kv_containsS(const char *json)
{
    if (json == NULL)
        return false;
    NGenXX::Json::Decoder decoder(json);
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    const char *k = str2charp(decoder.readString(decoder.readNode(NULL, "k")));
    if (conn <= 0 || k == NULL)
        return false;

    bool res = ngenxx_store_kv_contains((void *)conn, k);

    free((void *)k);
    return res;
}

void ngenxx_store_kv_clearS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    if (conn <= 0)
        return;

    ngenxx_store_kv_clear((void *)conn);
}

void ngenxx_store_kv_closeS(const char *json)
{
    if (json == NULL)
        return;
    NGenXX::Json::Decoder decoder(json);
    long conn = decoder.readNumber(decoder.readNode(NULL, "conn"));
    if (conn <= 0)
        return;

    ngenxx_store_kv_close((void *)conn);
}

#pragma mark Coding

const char *ngenxx_coding_hex_bytes2strS(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.first == NULL || in.second <= 0)
        return NULL;

    const char *res = ngenxx_coding_hex_bytes2str(in.first, in.second);

    free((void *)in.first);
    return res;
}

const char *ngenxx_coding_hex_str2bytesS(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);

    auto str = decoder.readString(decoder.readNode(NULL, "str"));

    if (str.size() == 0)
        return NULL;

    size outLen;
    const byte *outBytes = ngenxx_coding_hex_str2bytes(str.c_str(), &outLen);
    const char *outJson = bytes2json(outBytes, outLen);

    free((void *)outBytes);
    return outJson;
}

#pragma mark Crypto

const char *ngenxx_crypto_randS(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);
    size outLen = decoder.readNumber(decoder.readNode(NULL, "len"));
    if (outLen <= 0)
        return NULL;
    byte outBytes[outLen];

    ngenxx_crypto_rand(outLen, outBytes);
    const char *outJson = bytes2json(outBytes, outLen);

    return outJson;
}

const char *ngenxx_crypto_aes_encryptS(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.first == NULL || in.second <= 0)
        return NULL;

    auto key = parseByteArray(decoder, "keyBytes", "keyLen");
    if (key.first == NULL || key.second <= 0)
        return NULL;

    size outLen;
    const byte *outBytes = ngenxx_crypto_aes_encrypt(in.first, in.second, key.first, key.second, &outLen);
    const char *outJson = bytes2json(outBytes, outLen);

    free((void *)in.first);
    free((void *)key.first);
    free((void *)outBytes);
    return outJson;
}

const char *ngenxx_crypto_aes_decryptS(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.first == NULL || in.second <= 0)
        return NULL;

    auto key = parseByteArray(decoder, "keyBytes", "keyLen");
    if (key.first == NULL || key.second <= 0)
        return NULL;

    size outLen;
    const byte *outBytes = ngenxx_crypto_aes_decrypt(in.first, in.second, key.first, key.second, &outLen);
    const char *outJson = bytes2json(outBytes, outLen);

    free((void *)in.first);
    free((void *)key.first);
    free((void *)outBytes);
    return outJson;
}

const char *ngenxx_crypto_hash_md5S(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.first == NULL || in.second <= 0)
        return NULL;

    size outLen;
    const byte *outBytes = ngenxx_crypto_hash_md5(in.first, in.second, &outLen);
    const char *outJson = bytes2json(outBytes, outLen);

    free((void *)in.first);
    free((void *)outBytes);
    return outJson;
}

const char *ngenxx_crypto_hash_sha256S(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.first == NULL || in.second <= 0)
        return NULL;

    size outLen;
    const byte *outBytes = ngenxx_crypto_hash_sha256(in.first, in.second, &outLen);
    const char *outJson = bytes2json(outBytes, outLen);

    free((void *)in.first);
    free((void *)outBytes);
    return outJson;
}

const char *ngenxx_crypto_base64_encodeS(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.first == NULL || in.second <= 0)
        return NULL;

    size outLen;
    const byte *outBytes = ngenxx_crypto_base64_encode(in.first, in.second, &outLen);
    const char *outJson = bytes2json(outBytes, outLen);

    free((void *)in.first);
    free((void *)outBytes);
    return outJson;
}

const char *ngenxx_crypto_base64_decodeS(const char *json)
{
    if (json == NULL)
        return NULL;
    NGenXX::Json::Decoder decoder(json);

    auto in = parseByteArray(decoder, "inBytes", "inLen");
    if (in.first == NULL || in.second <= 0)
        return NULL;

    size outLen;
    const byte *outBytes = ngenxx_crypto_base64_decode(in.first, in.second, &outLen);
    const char *outJson = bytes2json(outBytes, outLen);

    free((void *)in.first);
    free((void *)outBytes);
    return outJson;
}