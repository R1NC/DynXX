#if defined(__cplusplus)

#include <NGenXX.hxx>
#include <NGenXXStoreKV.h>
#include <NGenXXTypes.hxx>
#include <NGenXXMacro.hxx>
#include "core/util/TypeUtil.hxx"
#endif

namespace
{
    byte *handleBytes(const Bytes &bytes, size_t *outLen)
    {
        if (outLen) [[likely]]
        {
            *outLen = bytes.size();
        }
        return NGenXX::Core::Util::Type::copyBytes(bytes);
    }
}

EXPORT_AUTO
const char *ngenxx_get_version()
{
    const auto s = ngenxxGetVersion();
    return NGenXX::Core::Util::Type::copyStr(s);
}

EXPORT_AUTO
const char *ngenxx_root_path()
{
    const auto& s = ngenxxRootPath();
    return NGenXX::Core::Util::Type::copyStr(s);
}

EXPORT
bool ngenxx_init(const char *root)
{
    return ngenxxInit(wrapStr(root));
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
    const auto& s = ngenxxDeviceName();
    return NGenXX::Core::Util::Type::copyStr(s);
}

EXPORT_AUTO
const char *ngenxx_device_manufacturer()
{
    const auto& s = ngenxxDeviceManufacturer();
    return NGenXX::Core::Util::Type::copyStr(s);
}

EXPORT_AUTO
const char *ngenxx_device_model()
{
    const auto& s = ngenxxDeviceModel();
    return NGenXX::Core::Util::Type::copyStr(s);
}

EXPORT_AUTO
const char *ngenxx_device_os_version()
{
    const auto& s = ngenxxDeviceOsVersion();
    return NGenXX::Core::Util::Type::copyStr(s);
}

EXPORT_AUTO
int ngenxx_device_cpu_arch()
{
    return static_cast<int>(ngenxxDeviceCpuArch());
}

#pragma mark Log

EXPORT_AUTO
void ngenxx_log_set_level(int level)
{
    ngenxxLogSetLevel(static_cast<NGenXXLogLevelX>(level));
}

EXPORT_AUTO
void ngenxx_log_set_callback(void (*const callback)(int level, const char *content))
{
    ngenxxLogSetCallback(callback);
}

EXPORT_AUTO
void ngenxx_log_print(int level, const char *content)
{
    ngenxxLogPrint(static_cast<NGenXXLogLevelX>(level), wrapStr(content));
}

#pragma mark Coding

EXPORT_AUTO
const byte *ngenxx_coding_hex_str2bytes(const char *str, size_t *outLen)
{
    const auto& bytes = ngenxxCodingHexStr2bytes(wrapStr(str));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const char *ngenxx_coding_hex_bytes2str(const byte *inBytes, size_t inLen)
{
    const auto& s = ngenxxCodingHexBytes2str(wrapBytes(inBytes, inLen));
    return NGenXX::Core::Util::Type::copyStr(s);
}

EXPORT_AUTO
const char *ngenxx_coding_bytes2str(const byte *inBytes, size_t inLen)
{
    const auto& s = ngenxxCodingBytes2str(wrapBytes(inBytes, inLen));
    return NGenXX::Core::Util::Type::copyStr(s);
}

EXPORT_AUTO
const byte *ngenxx_coding_str2bytes(const char *str, size_t *outLen)
{
    const auto& bytes = ngenxxCodingStr2bytes(wrapStr(str));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const char *ngenxx_coding_case_upper(const char *str)
{
    const auto& s = ngenxxCodingCaseUpper(str);
    return NGenXX::Core::Util::Type::copyStr(s);
}

EXPORT_AUTO
const char *ngenxx_coding_case_lower(const char *str)
{
    const auto& s = ngenxxCodingCaseLower(str);
    return NGenXX::Core::Util::Type::copyStr(s);
}

#pragma mark Crypto

EXPORT_AUTO
bool ngenxx_crypto_rand(size_t len, byte *bytes)
{
    return ngenxxCryptoRand(len, bytes);
}

EXPORT_AUTO
const byte *ngenxx_crypto_aes_encrypt(const byte *inBytes, size_t inLen, const byte *keyBytes, size_t keyLen, size_t *outLen)
{
    const auto& bytes = ngenxxCryptoAesEncrypt(wrapBytes(inBytes, inLen), wrapBytes(keyBytes, keyLen));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *ngenxx_crypto_aes_decrypt(const byte *inBytes, size_t inLen, const byte *keyBytes, size_t keyLen, size_t *outLen)
{
    const auto& bytes = ngenxxCryptoAesDecrypt(wrapBytes(inBytes, inLen), wrapBytes(keyBytes, keyLen));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *ngenxx_crypto_aes_gcm_encrypt(const byte *inBytes, size_t inLen,
                                          const byte *keyBytes, size_t keyLen,
                                          const byte *initVectorBytes, size_t initVectorLen,
                                          const byte *aadBytes, size_t aadLen,
                                          size_t tagBits, size_t *outLen)
{
    const auto& bytes = ngenxxCryptoAesGcmEncrypt(wrapBytes(inBytes, inLen), wrapBytes(keyBytes, keyLen), wrapBytes(initVectorBytes, initVectorLen), tagBits,
                                       wrapBytes(aadBytes, aadLen));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *ngenxx_crypto_aes_gcm_decrypt(const byte *inBytes, size_t inLen,
                                          const byte *keyBytes, size_t keyLen,
                                          const byte *initVectorBytes, size_t initVectorLen,
                                          const byte *aadBytes, size_t aadLen,
                                          size_t tagBits, size_t *outLen)
{
    const auto& bytes = ngenxxCryptoAesGcmDecrypt(wrapBytes(inBytes, inLen), wrapBytes(keyBytes, keyLen), wrapBytes(initVectorBytes, initVectorLen), tagBits,
                                       wrapBytes(aadBytes, aadLen));
    return handleBytes(bytes, outLen);
}

const byte *ngenxx_crypto_rsa_encrypt(const byte *inBytes, size_t inLen,
                                                              const byte *keyBytes, size_t keyLen, int padding, size_t *outLen)
{
    const auto& bytes = ngenxxCryptoRsaEncrypt(wrapBytes(inBytes, inLen), wrapBytes(keyBytes, keyLen), static_cast<NGenXXCryptoRSAPaddingX>(padding));
    return handleBytes(bytes, outLen);
}

const byte *ngenxx_crypto_rsa_decrypt(const byte *inBytes, size_t inLen,
                                                              const byte *keyBytes, size_t keyLen, int padding, size_t *outLen)
{
    const auto& bytes = ngenxxCryptoRsaDecrypt(wrapBytes(inBytes, inLen), wrapBytes(keyBytes, keyLen), static_cast<NGenXXCryptoRSAPaddingX>(padding));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *ngenxx_crypto_hash_md5(const byte *inBytes, size_t inLen, size_t *outLen)
{
    const auto& bytes = ngenxxCryptoHashMd5(wrapBytes(inBytes, inLen));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *ngenxx_crypto_hash_sha1(const byte *inBytes, size_t inLen, size_t *outLen)
{
    const auto& bytes = ngenxxCryptoHashSha1(wrapBytes(inBytes, inLen));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *ngenxx_crypto_hash_sha256(const byte *inBytes, size_t inLen, size_t *outLen)
{
    const auto& bytes = ngenxxCryptoHashSha256(wrapBytes(inBytes, inLen));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *ngenxx_crypto_base64_encode(const byte *inBytes, size_t inLen, size_t *outLen)
{
    const auto& bytes = ngenxxCryptoBase64Encode(wrapBytes(inBytes, inLen));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *ngenxx_crypto_base64_decode(const byte *inBytes, size_t inLen, size_t *outLen)
{
    const auto& bytes = ngenxxCryptoBase64Decode(wrapBytes(inBytes, inLen));
    return handleBytes(bytes, outLen);
}

#pragma mark Net.Http

EXPORT_AUTO
const char *ngenxx_net_http_request(const char *url, const char *params, int method,
                                    const char **header_v, size_t header_c,
                                    const char **form_field_name_v,
                                    const char **form_field_mime_v,
                                    const char **form_field_data_v,
                                    size_t form_field_count,
                                    void *const cFILE, size_t file_size,
                                    size_t timeout)
{
    std::vector<std::string> vHeaders;
    if (header_v != nullptr && header_c > 0)
    {
        vHeaders = std::vector<std::string>(header_v, header_v + header_c);
    }

    std::vector<std::string> vFormFieldName;
    if (form_field_name_v != nullptr && form_field_count > 0)
    {
        vFormFieldName = std::vector<std::string>(form_field_name_v, form_field_name_v + form_field_count);
    }

    std::vector<std::string> vFormFieldMime;
    if (form_field_mime_v != nullptr && form_field_count > 0)
    {
        vFormFieldMime = std::vector<std::string>(form_field_mime_v, form_field_mime_v + form_field_count);
    }

    std::vector<std::string> vFormFieldData;
    if (form_field_data_v != nullptr && form_field_count > 0)
    {
        vFormFieldData = std::vector<std::string>(form_field_data_v, form_field_data_v + form_field_count);
    }

    const auto& t = ngenxxNetHttpRequest(wrapStr(url),
                                  static_cast<NGenXXHttpMethodX>(method),
                                  wrapStr(params),
                                  {},
                                  vHeaders, vFormFieldName, vFormFieldMime, vFormFieldData,
                                  static_cast<std::FILE *>(cFILE), file_size, timeout);
    const auto& s = t.toJson();
    return NGenXX::Core::Util::Type::copyStr(s.value_or(""));
}

EXPORT_AUTO
bool ngenxx_net_http_download(const char *url, const char *file_path, size_t timeout)
{
    return ngenxxNetHttpDownload(wrapStr(url), wrapStr(file_path), timeout);
}

#pragma mark Store.SQLite

EXPORT_AUTO
void *ngenxx_store_sqlite_open(const char *_id)
{
    return ngenxxStoreSqliteOpen(wrapStr(_id));
}

EXPORT_AUTO
bool ngenxx_store_sqlite_execute(void *const conn, const char *sql)
{
    return ngenxxStoreSqliteExecute(conn, wrapStr(sql));
}

EXPORT_AUTO
void *ngenxx_store_sqlite_query_do(void *const conn, const char *sql)
{
    return ngenxxStoreSqliteQueryDo(conn, wrapStr(sql));
}

EXPORT_AUTO
bool ngenxx_store_sqlite_query_read_row(void *const query_result)
{
    return ngenxxStoreSqliteQueryReadRow(query_result);
}

EXPORT_AUTO
const char *ngenxx_store_sqlite_query_read_column_text(void *const query_result, const char *column)
{
    const auto& s = ngenxxStoreSqliteQueryReadColumnText(query_result, wrapStr(column));
    return NGenXX::Core::Util::Type::copyStr(s.value_or(""));
}

EXPORT_AUTO
int64_t ngenxx_store_sqlite_query_read_column_integer(void *const query_result, const char *column)
{
    const auto& i = ngenxxStoreSqliteQueryReadColumnInteger(query_result, wrapStr(column));
    return i.value_or(0);
}

EXPORT_AUTO
double ngenxx_store_sqlite_query_read_column_float(void *const query_result, const char *column)
{
    const auto& f = ngenxxStoreSqliteQueryReadColumnFloat(query_result, wrapStr(column));
    return f.value_or(0.0);
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
void *ngenxx_store_kv_open(const char *_id)
{
    return ngenxxStoreKvOpen(wrapStr(_id));
}

EXPORT_AUTO
const char *ngenxx_store_kv_read_string(void *const conn, const char *k)
{
    const auto& s = ngenxxStoreKvReadString(conn, wrapStr(k));
    return NGenXX::Core::Util::Type::copyStr(s.value_or(""));
}

EXPORT_AUTO
bool ngenxx_store_kv_write_string(void *const conn, const char *k, const char *v)
{
    return ngenxxStoreKvWriteString(conn, wrapStr(k), wrapStr(v));
}

EXPORT_AUTO
int64_t ngenxx_store_kv_read_integer(void *const conn, const char *k)
{
    const auto& i = ngenxxStoreKvReadInteger(conn, wrapStr(k));
    return i.value_or(0);
}

EXPORT_AUTO
bool ngenxx_store_kv_write_integer(void *const conn, const char *k, int64_t v)
{
    return ngenxxStoreKvWriteInteger(conn, wrapStr(k), v);
}

EXPORT_AUTO
double ngenxx_store_kv_read_float(void *const conn, const char *k)
{
    const auto& f = ngenxxStoreKvReadFloat(conn, wrapStr(k));
    return f.value_or(0.0);
}

EXPORT_AUTO
bool ngenxx_store_kv_write_float(void *const conn, const char *k, double v)
{
    return ngenxxStoreKvWriteFloat(conn, wrapStr(k), v);
}

EXPORT_AUTO
char *const *ngenxx_store_kv_all_keys(void *const conn, size_t *len)
{
    const auto& t = ngenxxStoreKvAllKeys(conn);
    if (len)
    {
        *len = t.size();
    }
    return NGenXX::Core::Util::Type::copyStrVector(t, NGENXX_STORE_KV_KEY_MAX_LENGTH);
}

EXPORT_AUTO
bool ngenxx_store_kv_contains(void *const conn, const char *k)
{
    return ngenxxStoreKvContains(conn, wrapStr(k));
}

EXPORT_AUTO
bool ngenxx_store_kv_remove(void *const conn, const char *k)
{
    return ngenxxStoreKvRemove(conn, wrapStr(k));
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
int ngenxx_json_read_type(void *const cjson)
{
    return static_cast<int>(ngenxxJsonReadType(cjson));
}

EXPORT_AUTO
const char *ngenxx_json_to_str(void *const cjson)
{
    const auto& s = ngenxxJsonToStr(cjson);
    return NGenXX::Core::Util::Type::copyStr(s.value_or(""));
}

EXPORT_AUTO
void *ngenxx_json_decoder_init(const char *json)
{
    return ngenxxJsonDecoderInit(wrapStr(json));
}

EXPORT_AUTO
void *ngenxx_json_decoder_read_node(void *const decoder, void *const node, const char *k)
{
    return ngenxxJsonDecoderReadNode(decoder, wrapStr(k), node);
}

EXPORT_AUTO
const char *ngenxx_json_decoder_read_string(void *const decoder, void *const node)
{
    const auto& s = ngenxxJsonDecoderReadString(decoder, node);
    return NGenXX::Core::Util::Type::copyStr(s.value_or(""));
}

EXPORT_AUTO
double ngenxx_json_decoder_read_number(void *const decoder, void *const node)
{
    return ngenxxJsonDecoderReadNumber(decoder, node).value_or(0.0);
}

EXPORT_AUTO
void *ngenxx_json_decoder_read_child(void *const decoder, void *const node)
{
    return ngenxxJsonDecoderReadChild(decoder, node);
}

EXPORT_AUTO
void *ngenxx_json_decoder_read_next(void *const decoder, void *const node)
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
void *ngenxx_z_zip_init(int mode, size_t bufferSize, int format)
{
    return ngenxxZZipInit(static_cast<NGenXXZipCompressModeX>(mode), bufferSize, static_cast<NGenXXZFormatX>(format));
}

EXPORT_AUTO
size_t ngenxx_z_zip_input(void *const zip, const byte *inBytes, size_t inLen, bool inFinish)
{
    return ngenxxZZipInput(zip, wrapBytes(inBytes, inLen), inFinish);
}

EXPORT_AUTO
const byte *ngenxx_z_zip_process_do(void *const zip, size_t *outLen)
{
    const auto& bytes = ngenxxZZipProcessDo(zip);
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
bool ngenxx_z_zip_process_finished(void *const zip)
{
    return ngenxxZZipProcessFinished(zip);
}

EXPORT_AUTO
void ngenxx_z_zip_release(void *const zip)
{
    ngenxxZZipRelease(zip);
}

EXPORT_AUTO
void *ngenxx_z_unzip_init(size_t bufferSize, int format)
{
    return ngenxxZUnzipInit(bufferSize, static_cast<NGenXXZFormatX>(format));
}

EXPORT_AUTO
size_t ngenxx_z_unzip_input(void *const unzip, const byte *inBytes, size_t inLen, bool inFinish)
{
    return ngenxxZUnzipInput(unzip, wrapBytes(inBytes, inLen), inFinish);
}

EXPORT_AUTO
const byte *ngenxx_z_unzip_process_do(void *const unzip, size_t *outLen)
{
    const auto& bytes = ngenxxZUnzipProcessDo(unzip);
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
bool ngenxx_z_unzip_process_finished(void *const unzip)
{
    return ngenxxZUnzipProcessFinished(unzip);
}

EXPORT_AUTO
void ngenxx_z_unzip_release(void *const unzip)
{
    ngenxxZUnzipRelease(unzip);
}

EXPORT_AUTO
bool ngenxx_z_cfile_zip(int mode, size_t bufferSize, int format, void *const cFILEIn, void *const cFILEOut)
{
    return ngenxxZCFileZip(static_cast<std::FILE *>(cFILEIn), static_cast<std::FILE *>(cFILEOut),
                           static_cast<NGenXXZipCompressModeX>(mode), bufferSize, static_cast<NGenXXZFormatX>(format));
}

EXPORT_AUTO
bool ngenxx_z_cfile_unzip(size_t bufferSize, int format, void *const cFILEIn, void *const cFILEOut)
{
    return ngenxxZCFileUnzip(static_cast<std::FILE *>(cFILEIn), static_cast<std::FILE *>(cFILEOut),
                             bufferSize, static_cast<NGenXXZFormatX>(format));
}

EXPORT_AUTO
bool ngenxx_z_cxxstream_zip(int mode, size_t bufferSize, int format, void *const cxxStreamIn, void *const cxxStreamOut)
{
    return ngenxxZCxxStreamZip(static_cast<std::istream *>(cxxStreamIn), static_cast<std::ostream *>(cxxStreamOut),
                               static_cast<NGenXXZipCompressModeX>(mode), bufferSize, static_cast<NGenXXZFormatX>(format));
}

EXPORT_AUTO
bool ngenxx_z_cxxstream_unzip(size_t bufferSize, int format, void *const cxxStreamIn, void *const cxxStreamOut)
{
    return ngenxxZCxxStreamUnzip(static_cast<std::istream *>(cxxStreamIn), static_cast<std::ostream *>(cxxStreamOut),
                                 bufferSize, static_cast<NGenXXZFormatX>(format));
}

EXPORT_AUTO
const byte *ngenxx_z_bytes_zip(int mode, size_t bufferSize, int format, const byte *inBytes, size_t inLen, size_t *outLen)
{
    const auto& bytes = ngenxxZBytesZip(wrapBytes(inBytes, inLen),
                             static_cast<NGenXXZipCompressModeX>(mode), bufferSize, static_cast<NGenXXZFormatX>(format));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *ngenxx_z_bytes_unzip(size_t bufferSize, int format, const byte *inBytes, size_t inLen, size_t *outLen)
{
    const auto& bytes = ngenxxZBytesUnzip(wrapBytes(inBytes, inLen),
                               bufferSize, static_cast<NGenXXZFormatX>(format));
    return handleBytes(bytes, outLen);
}