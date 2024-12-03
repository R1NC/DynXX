#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__cplusplus)

#include "../include/NGenXX.hxx"
#include "../include/NGenXXTypes.hxx"
#include "../include/NGenXXStoreKV.h"
#include "NGenXX-inner.hxx"
#include "util/TypeUtil.hxx"
#endif

EXPORT_AUTO
const char *ngenxx_get_version(void)
{
    return copyStr(ngenxxGetVersion());
}

EXPORT_AUTO
const char *ngenxx_root_path()
{
    return copyStr(ngenxxRootPath());
}

EXPORT
bool ngenxx_init(const char *root)
{
    return ngenxxInit(root ?: "");
}

EXPORT
void ngenxx_release()
{
    ngenxxRelease();
}

#pragma mark Device.DeviceInfo

EXPORT_AUTO
int ngenxx_device_type()
{
    return static_cast<int>(ngenxxDeviceType());
}

EXPORT_AUTO
const char *ngenxx_device_name()
{
    return copyStr(ngenxxDeviceName());
}

EXPORT_AUTO
const char *ngenxx_device_manufacturer()
{
    return copyStr(ngenxxDeviceManufacturer());
}

EXPORT_AUTO
const char *ngenxx_device_os_version()
{
    return copyStr(ngenxxDeviceOsVersion());
}

EXPORT_AUTO
int ngenxx_device_cpu_arch()
{
    return static_cast<int>(ngenxxDeviceCpuArch());
}

#pragma mark Log

EXPORT_AUTO
void ngenxx_log_set_level(const int level)
{
    ngenxxLogSetLevel(static_cast<NGenXXLogLevelX>(level));
}

EXPORT_AUTO
void ngenxx_log_set_callback(void (*const callback)(const int level, const char *content))
{
    ngenxxLogSetCallback(callback);
}

EXPORT_AUTO
void ngenxx_log_print(const int level, const char *content)
{
    ngenxxLogPrint(static_cast<NGenXXLogLevelX>(level), content ?: "");
}

#pragma mark Coding

EXPORT_AUTO
const byte *ngenxx_coding_hex_str2bytes(const char *str, size_t *outLen)
{
    auto t = ngenxxCodingHexStr2bytes(str ?: "");
    if (outLen)
    {
        *outLen = t.size();
    }
    return copyBytes(t);
}

EXPORT_AUTO
const char *ngenxx_coding_hex_bytes2str(const byte *inBytes, const size_t inLen)
{
    auto s = ngenxxCodingHexBytes2str(wrapBytes(inBytes, inLen));
    return copyStr(s);
}

EXPORT_AUTO
const char *ngenxx_coding_bytes2str(const byte *inBytes, const size_t inLen)
{
    auto s = ngenxxCodingBytes2str(wrapBytes(inBytes, inLen));
    return copyStr(s);
}

EXPORT_AUTO
const byte *ngenxx_coding_str2bytes(const char *str, size_t *outLen)
{
    auto t = ngenxxCodingStr2bytes(str ?: "");
    if (outLen)
    {
        *outLen = t.size();
    }
    return copyBytes(t);
}

EXPORT_AUTO
const char *ngenxx_coding_case_upper(const char *str)
{
    auto s = ngenxxCodingCaseUpper(str);
    return copyStr(s);
}

EXPORT_AUTO
const char *ngenxx_coding_case_lower(const char *str)
{
    auto s = ngenxxCodingCaseLower(str);
    return copyStr(s);
}

#pragma mark Crypto

EXPORT_AUTO
bool ngenxx_crypto_rand(const size_t len, byte *bytes)
{
    return ngenxxCryptoRand(len, bytes);
}

EXPORT_AUTO
const byte *ngenxx_crypto_aes_encrypt(const byte *inBytes, const size_t inLen, const byte *keyBytes, const size_t keyLen, size_t *outLen)
{
    auto t = ngenxxCryptoAesEncrypt(wrapBytes(inBytes, inLen), wrapBytes(keyBytes, keyLen));
    if (outLen)
    {
        *outLen = t.size();
    }
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_crypto_aes_decrypt(const byte *inBytes, const size_t inLen, const byte *keyBytes, const size_t keyLen, size_t *outLen)
{
    auto t = ngenxxCryptoAesDecrypt(wrapBytes(inBytes, inLen), wrapBytes(keyBytes, keyLen));
    if (outLen)
    {
        *outLen = t.size();
    }
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_crypto_aes_gcm_encrypt(const byte *inBytes, const size_t inLen,
                                          const byte *keyBytes, const size_t keyLen,
                                          const byte *initVectorBytes, const size_t initVectorLen,
                                          const byte *aadBytes, const size_t aadLen,
                                          const size_t tagBits, size_t *outLen)
{
    auto t = ngenxxCryptoAesGcmEncrypt(wrapBytes(inBytes, inLen), wrapBytes(keyBytes, keyLen), wrapBytes(initVectorBytes, initVectorLen), tagBits,
                                       wrapBytes(aadBytes, aadLen));
    if (outLen)
    {
        *outLen = t.size();
    }
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_crypto_aes_gcm_decrypt(const byte *inBytes, const size_t inLen,
                                          const byte *keyBytes, const size_t keyLen,
                                          const byte *initVectorBytes, const size_t initVectorLen,
                                          const byte *aadBytes, const size_t aadLen,
                                          const size_t tagBits, size_t *outLen)
{
    auto t = ngenxxCryptoAesGcmDecrypt(wrapBytes(inBytes, inLen), wrapBytes(keyBytes, keyLen), wrapBytes(initVectorBytes, initVectorLen), tagBits,
                                       wrapBytes(aadBytes, aadLen));
    if (outLen)
    {
        *outLen = t.size();
    }
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_crypto_hash_md5(const byte *inBytes, const size_t inLen, size_t *outLen)
{
    auto t = ngenxxCryptoHashMd5(wrapBytes(inBytes, inLen));
    if (outLen)
    {
        *outLen = t.size();
    }
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_crypto_hash_sha256(const byte *inBytes, const size_t inLen, size_t *outLen)
{
    auto t = ngenxxCryptoHashSha256(wrapBytes(inBytes, inLen));
    if (outLen)
    {
        *outLen = t.size();
    }
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_crypto_base64_encode(const byte *inBytes, const size_t inLen, size_t *outLen)
{
    auto t = ngenxxCryptoBase64Encode(wrapBytes(inBytes, inLen));
    if (outLen)
    {
        *outLen = t.size();
    }
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_crypto_base64_decode(const byte *inBytes, const size_t inLen, size_t *outLen)
{
    auto t = ngenxxCryptoBase64Decode(wrapBytes(inBytes, inLen));
    if (outLen)
    {
        *outLen = t.size();
    }
    return copyBytes(t);
}

#pragma mark Net.Http

EXPORT_AUTO
const char *ngenxx_net_http_request(const char *url, const char *params, const int method,
                                    const char **header_v, const size_t header_c,
                                    const char **form_field_name_v,
                                    const char **form_field_mime_v,
                                    const char **form_field_data_v,
                                    const size_t form_field_count,
                                    void *const cFILE, const size_t file_size,
                                    const size_t timeout)
{
    std::vector<std::string> vHeaders;
    if (header_v != NULL && header_c > 0)
    {
        vHeaders = std::vector<std::string>(header_v, header_v + header_c);
    }

    std::vector<std::string> vFormFieldName;
    if (form_field_name_v != NULL && form_field_count > 0)
    {
        vFormFieldName = std::vector<std::string>(form_field_name_v, form_field_name_v + form_field_count);
    }

    std::vector<std::string> vFormFieldMime;
    if (form_field_mime_v != NULL && form_field_count > 0)
    {
        vFormFieldMime = std::vector<std::string>(form_field_mime_v, form_field_mime_v + form_field_count);
    }

    std::vector<std::string> vFormFieldData;
    if (form_field_data_v != NULL && form_field_count > 0)
    {
        vFormFieldData = std::vector<std::string>(form_field_data_v, form_field_data_v + form_field_count);
    }

    auto t = ngenxxNetHttpRequest(url ?: "",
                                  static_cast<NGenXXHttpMethodX>(method),
                                  params ?: "",
                                  BytesEmpty,
                                  vHeaders, vFormFieldName, vFormFieldMime, vFormFieldData,
                                  reinterpret_cast<std::FILE *>(cFILE), file_size, timeout);
    return copyStr(t.toJson());
}

#pragma mark Store.SQLite

EXPORT_AUTO
void *const ngenxx_store_sqlite_open(const char *_id)
{
    return ngenxxStoreSqliteOpen(_id ?: "");
}

EXPORT_AUTO
bool ngenxx_store_sqlite_execute(void *const conn, const char *sql)
{
    return ngenxxStoreSqliteExecute(conn, sql ?: "");
}

EXPORT_AUTO
void *const ngenxx_store_sqlite_query_do(void *const conn, const char *sql)
{
    return ngenxxStoreSqliteQueryDo(conn, sql ?: "");
}

EXPORT_AUTO
bool ngenxx_store_sqlite_query_read_row(void *const query_result)
{
    return ngenxxStoreSqliteQueryReadRow(query_result);
}

EXPORT_AUTO
const char *ngenxx_store_sqlite_query_read_column_text(void *const query_result, const char *column)
{
    return copyStr(ngenxxStoreSqliteQueryReadColumnText(query_result, column ?: ""));
}

EXPORT_AUTO
long long ngenxx_store_sqlite_query_read_column_integer(void *const query_result, const char *column)
{
    return ngenxxStoreSqliteQueryReadColumnInteger(query_result, column ?: "");
}

EXPORT_AUTO
double ngenxx_store_sqlite_query_read_column_float(void *const query_result, const char *column)
{
    return ngenxxStoreSqliteQueryReadColumnFloat(query_result, column ?: "");
}

EXPORT_AUTO
void ngenxx_store_sqlite_query_drop(void *const query_result)
{
    ngenxxStoreSqliteQueryDrop(query_result);
}

EXPORT_AUTO
void ngenxx_store_sqlite_close(void *const conn)
{
    ngenxxStoreSqliteClose(conn);
}

#pragma mark Store.KV

EXPORT_AUTO
void *const ngenxx_store_kv_open(const char *_id)
{
    return ngenxxStoreKvOpen(_id ?: "");
}

EXPORT_AUTO
const char *ngenxx_store_kv_read_string(void *const conn, const char *k)
{
    return copyStr(ngenxxStoreKvReadString(conn, k ?: ""));
}

EXPORT_AUTO
bool ngenxx_store_kv_write_string(void *const conn, const char *k, const char *v)
{
    return ngenxxStoreKvWriteString(conn, k ?: "", v ?: "");
}

EXPORT_AUTO
long long ngenxx_store_kv_read_integer(void *const conn, const char *k)
{
    return ngenxxStoreKvReadInteger(conn, k ?: "");
}

EXPORT_AUTO
bool ngenxx_store_kv_write_integer(void *const conn, const char *k, long long v)
{
    return ngenxxStoreKvWriteInteger(conn, k ?: "", v);
}

EXPORT_AUTO
double ngenxx_store_kv_read_float(void *const conn, const char *k)
{
    return ngenxxStoreKvReadFloat(conn, k ?: "");
}

EXPORT_AUTO
bool ngenxx_store_kv_write_float(void *const conn, const char *k, double v)
{
    return ngenxxStoreKvWriteFloat(conn, k ?: "", v);
}

EXPORT_AUTO
const char **ngenxx_store_kv_all_keys(void *const conn, size_t *len)
{
    auto t = ngenxxStoreKvAllKeys(conn);
    if (len)
    {
        *len = t.size();
    }
    return copyStrVector(t, NGENXX_STORE_KV_KEY_MAX_LENGTH);
}

EXPORT_AUTO
bool ngenxx_store_kv_contains(void *const conn, const char *k)
{
    return ngenxxStoreKvContains(conn, k ?: "");
}

EXPORT_AUTO
bool ngenxx_store_kv_remove(void *const conn, const char *k)
{
    return ngenxxStoreKvRemove(conn, k ?: "");
}

EXPORT_AUTO
void ngenxx_store_kv_clear(void *const conn)
{
    ngenxxStoreKvClear(conn);
}

EXPORT_AUTO
void ngenxx_store_kv_close(void *const conn)
{
    ngenxxStoreKvClose(conn);
}

#pragma mark Json.Decoder

EXPORT_AUTO
void *const ngenxx_json_decoder_init(const char *json)
{
    return json ? ngenxxJsonDecoderInit(json ?: "") : NULL;
}

EXPORT_AUTO
bool ngenxx_json_decoder_is_array(void *const decoder, void *const node)
{
    return ngenxxJsonDecoderIsArray(decoder, node);
}

EXPORT_AUTO
bool ngenxx_json_decoder_is_object(void *const decoder, void *const node)
{
    return ngenxxJsonDecoderIsObject(decoder, node);
}

EXPORT_AUTO
void *const ngenxx_json_decoder_read_node(void *const decoder, void *const node, const char *k)
{
    return ngenxxJsonDecoderReadNode(decoder, k ?: "", node);
}

EXPORT_AUTO
const char *ngenxx_json_decoder_read_string(void *const decoder, void *const node)
{
    return copyStr(ngenxxJsonDecoderReadString(decoder, node));
}

EXPORT_AUTO
double ngenxx_json_decoder_read_number(void *const decoder, void *const node)
{
    return ngenxxJsonDecoderReadNumber(decoder, node);
}

EXPORT_AUTO
void *const ngenxx_json_decoder_read_child(void *const decoder, void *const node)
{
    return ngenxxJsonDecoderReadChild(decoder, node);
}

EXPORT_AUTO
void *const ngenxx_json_decoder_read_next(void *const decoder, void *const node)
{
    return ngenxxJsonDecoderReadNext(decoder, node);
}

EXPORT_AUTO
void ngenxx_json_decoder_release(void *const decoder)
{
    ngenxxJsonDecoderRelease(decoder);
}

#pragma mark Zip

EXPORT_AUTO
void *const ngenxx_z_zip_init(const int mode, const size_t bufferSize, const int format)
{
    return ngenxxZZipInit(static_cast<NGenXXZipCompressModeX>(mode), bufferSize, static_cast<NGenXXZFormatX>(format));
}

EXPORT_AUTO
const size_t ngenxx_z_zip_input(void *const zip, const byte *inBytes, const size_t inLen, const bool inFinish)
{
    return ngenxxZZipInput(zip, wrapBytes(inBytes, inLen), inFinish);
}

EXPORT_AUTO
const byte *ngenxx_z_zip_process_do(void *const zip, size_t *outLen)
{
    auto t = ngenxxZZipProcessDo(zip);
    if (outLen)
    {
        *outLen = t.size();
    }
    return copyBytes(t);
}

EXPORT_AUTO
const bool ngenxx_z_zip_process_finished(void *const zip)
{
    return ngenxxZZipProcessFinished(zip);
}

EXPORT_AUTO
void ngenxx_z_zip_release(void *const zip)
{
    ngenxxZZipRelease(zip);
}

EXPORT_AUTO
void *const ngenxx_z_unzip_init(const size_t bufferSize, const int format)
{
    return ngenxxZUnzipInit(bufferSize, static_cast<NGenXXZFormatX>(format));
}

EXPORT_AUTO
const size_t ngenxx_z_unzip_input(void *const unzip, const byte *inBytes, const size_t inLen, const bool inFinish)
{
    return ngenxxZUnzipInput(unzip, wrapBytes(inBytes, inLen), inFinish);
}

EXPORT_AUTO
const byte *ngenxx_z_unzip_process_do(void *const unzip, size_t *outLen)
{
    auto t = ngenxxZUnzipProcessDo(unzip);
    if (outLen)
    {
        *outLen = t.size();
    }
    return copyBytes(t);
}

EXPORT_AUTO
const bool ngenxx_z_unzip_process_finished(void *const unzip)
{
    return ngenxxZUnzipProcessFinished(unzip);
}

EXPORT_AUTO
void ngenxx_z_unzip_release(void *const unzip)
{
    ngenxxZUnzipRelease(unzip);
}

EXPORT_AUTO
bool ngenxx_z_cfile_zip(const int mode, const size_t bufferSize, const int format, void *const cFILEIn, void *const cFILEOut)
{
    return ngenxxZCFileZip(reinterpret_cast<std::FILE *>(cFILEIn), reinterpret_cast<std::FILE *>(cFILEOut),
                           static_cast<NGenXXZipCompressModeX>(mode), bufferSize, static_cast<NGenXXZFormatX>(format));
}

EXPORT_AUTO
bool ngenxx_z_cfile_unzip(const size_t bufferSize, const int format, void *const cFILEIn, void *const cFILEOut)
{
    return ngenxxZCFileUnzip(reinterpret_cast<std::FILE *>(cFILEIn), reinterpret_cast<std::FILE *>(cFILEOut),
                             bufferSize, static_cast<NGenXXZFormatX>(format));
}

EXPORT_AUTO
bool ngenxx_z_cxxstream_zip(const int mode, const size_t bufferSize, const int format, void *const cxxStreamIn, void *const cxxStreamOut)
{
    return ngenxxZCxxStreamZip(reinterpret_cast<std::istream *>(cxxStreamIn), reinterpret_cast<std::ostream *>(cxxStreamOut),
                               static_cast<NGenXXZipCompressModeX>(mode), bufferSize, static_cast<NGenXXZFormatX>(format));
}

EXPORT_AUTO
bool ngenxx_z_cxxstream_unzip(const size_t bufferSize, const int format, void *const cxxStreamIn, void *const cxxStreamOut)
{
    return ngenxxZCxxStreamUnzip(reinterpret_cast<std::istream *>(cxxStreamIn), reinterpret_cast<std::ostream *>(cxxStreamOut),
                                 bufferSize, static_cast<NGenXXZFormatX>(format));
}

EXPORT_AUTO
const byte *ngenxx_z_bytes_zip(const int mode, const size_t bufferSize, const int format, const byte *inBytes, const size_t inLen, size_t *outLen)
{
    auto t = ngenxxZBytesZip(wrapBytes(inBytes, inLen),
                             static_cast<NGenXXZipCompressModeX>(mode), bufferSize, static_cast<NGenXXZFormatX>(format));
    if (outLen)
    {
        *outLen = t.size();
    }
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_z_bytes_unzip(const size_t bufferSize, const int format, const byte *inBytes, const size_t inLen, size_t *outLen)
{
    auto t = ngenxxZBytesUnzip(wrapBytes(inBytes, inLen),
                               bufferSize, static_cast<NGenXXZFormatX>(format));
    if (outLen)
    {
        *outLen = t.size();
    }
    return copyBytes(t);
}