#if defined(__cplusplus)

#include <utility>

#include <DynXX/C/KV.h>
#include <DynXX/CXX/DynXX.hxx>
#include "core/util/TypeUtil.hxx"
#endif

namespace {
    using namespace DynXX::Core::Util::Type;

    byte *handleBytes(BytesView bytes, size_t *outLen = nullptr) {
        if (outLen) [[likely]] {
            *outLen = bytes.size();
        }
        return copyBytes(bytes);
    }
}

EXPORT_AUTO
const char *dynxx_get_version() {
    const auto s = dynxxGetVersion();
    return dupStr(s);
}

#if defined(USE_DB) || defined(USE_KV)
EXPORT_AUTO
const char *dynxx_root_path() {
    const auto s = dynxxRootPath().value_or("");
    return dupStr(s);
}
#endif

EXPORT
bool dynxx_init(const char *root) {
    return dynxxInit(makeStr(root));
}

EXPORT
void dynxx_release() {
    dynxxRelease();
}

// Device.DeviceInfo

#if defined(USE_DEVICE)

EXPORT_AUTO
int dynxx_device_type() {
    return std::to_underlying(dynxxDeviceType());
}

EXPORT_AUTO
const char *dynxx_device_name() {
    const auto s = dynxxDeviceName();
    return dupStr(s);
}

EXPORT_AUTO
const char *dynxx_device_manufacturer() {
    const auto s = dynxxDeviceManufacturer();
    return dupStr(s);
}

EXPORT_AUTO
const char *dynxx_device_model() {
    const auto s = dynxxDeviceModel();
    return dupStr(s);
}

EXPORT_AUTO
const char *dynxx_device_os_version() {
    const auto s = dynxxDeviceOsVersion();
    return dupStr(s);
}

EXPORT_AUTO
int dynxx_device_cpu_arch() {
    return std::to_underlying(dynxxDeviceCpuArch());
}

#endif

// Log

EXPORT_AUTO
void dynxx_log_set_level(int level) {
    dynxxLogSetLevel(static_cast<DynXXLogLevelX>(level));
}

EXPORT_AUTO
void dynxx_log_set_callback(void (*const callback)(int level, const char *content)) {
    dynxxLogSetCallback(callback);
}

EXPORT_AUTO
void dynxx_log_print(int level, const char *content) {
    if (content == nullptr) {
        return;
    }
    dynxxLogPrint(static_cast<DynXXLogLevelX>(level), content);
}

// Coding

EXPORT_AUTO
const char *dynxx_coding_case_upper(const char *str) {
    if (str == nullptr) {
        return "";
    }
    const auto s = dynxxCodingCaseUpper(str);
    return dupStr(s);
}

EXPORT_AUTO
const char *dynxx_coding_case_lower(const char *str) {
    if (str == nullptr) {
        return "";
    }
    const auto s = dynxxCodingCaseLower(str);
    return dupStr(s);
}

EXPORT_AUTO
const byte *dynxx_coding_hex_str2bytes(const char *str, size_t *outLen) {
    if (str == nullptr) {
        return nullptr;
    }
    const auto bytes = dynxxCodingHexStr2bytes(str);
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const char *dynxx_coding_hex_bytes2str(const byte *inBytes, size_t inLen) {
    const auto s = dynxxCodingHexBytes2str(makeBytesView(inBytes, inLen));
    return dupStr(s);
}

EXPORT_AUTO
const byte *dynxx_coding_str2bytes(const char *str, size_t *outLen) {
    if (str == nullptr) {
        return nullptr;
    }
    const auto bytes = dynxxCodingStr2bytes(str);
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const char *dynxx_coding_bytes2str(const byte *inBytes, size_t inLen) {
    const auto s = dynxxCodingBytes2str(makeBytesView(inBytes, inLen));
    return dupStr(s);
}

EXPORT_AUTO
const char *dynxx_coding_str_trim(const char *str) {
    if (str == nullptr) {
        return "";
    }
    const auto s = dynxxCodingStrTrim(str);
    return dupStr(s);
}

// Crypto

EXPORT_AUTO
const byte *dynxx_crypto_rand(size_t len) {
    const auto bytes = dynxxCryptoRand(len);
    return handleBytes(bytes);
}

EXPORT_AUTO
const byte *dynxx_crypto_aes_encrypt(const byte *inBytes, size_t inLen, const byte *keyBytes, size_t keyLen,
                                      size_t *outLen) {
    const auto bytes = dynxxCryptoAesEncrypt(makeBytesView(inBytes, inLen), makeBytesView(keyBytes, keyLen));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *dynxx_crypto_aes_decrypt(const byte *inBytes, size_t inLen, const byte *keyBytes, size_t keyLen,
                                      size_t *outLen) {
    const auto bytes = dynxxCryptoAesDecrypt(makeBytesView(inBytes, inLen), makeBytesView(keyBytes, keyLen));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *dynxx_crypto_aes_gcm_encrypt(const byte *inBytes, size_t inLen,
                                          const byte *keyBytes, size_t keyLen,
                                          const byte *initVectorBytes, size_t initVectorLen,
                                          const byte *aadBytes, size_t aadLen,
                                          size_t tagBits, size_t *outLen) {
    const auto bytes = dynxxCryptoAesGcmEncrypt(makeBytesView(inBytes, inLen), makeBytesView(keyBytes, keyLen),
                                                  makeBytesView(initVectorBytes, initVectorLen), tagBits,
                                                  makeBytesView(aadBytes, aadLen));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *dynxx_crypto_aes_gcm_decrypt(const byte *inBytes, size_t inLen,
                                          const byte *keyBytes, size_t keyLen,
                                          const byte *initVectorBytes, size_t initVectorLen,
                                          const byte *aadBytes, size_t aadLen,
                                          size_t tagBits, size_t *outLen) {
    const auto bytes = dynxxCryptoAesGcmDecrypt(makeBytesView(inBytes, inLen), makeBytesView(keyBytes, keyLen),
                                                  makeBytesView(initVectorBytes, initVectorLen), tagBits,
                                                  makeBytesView(aadBytes, aadLen));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const char *dynxx_crypto_rsa_gen_key(const char *base64, bool isPublic) {
    if (base64 == nullptr) {
        return "";
    }
    const auto s = dynxxCryptoRsaGenKey(base64, isPublic);
    return dupStr(s);
}

EXPORT_AUTO
const byte *dynxx_crypto_rsa_encrypt(const byte *inBytes, size_t inLen,
                                      const byte *keyBytes, size_t keyLen, int padding, size_t *outLen) {
    const auto bytes = dynxxCryptoRsaEncrypt(makeBytesView(inBytes, inLen), makeBytesView(keyBytes, keyLen),
                                               static_cast<DynXXCryptoRSAPaddingX>(padding));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *dynxx_crypto_rsa_decrypt(const byte *inBytes, size_t inLen,
                                      const byte *keyBytes, size_t keyLen, int padding, size_t *outLen) {
    const auto bytes = dynxxCryptoRsaDecrypt(makeBytesView(inBytes, inLen), makeBytesView(keyBytes, keyLen),
                                               static_cast<DynXXCryptoRSAPaddingX>(padding));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *dynxx_crypto_hash_md5(const byte *inBytes, size_t inLen, size_t *outLen) {
    const auto bytes = dynxxCryptoHashMd5(makeBytesView(inBytes, inLen));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *dynxx_crypto_hash_sha1(const byte *inBytes, size_t inLen, size_t *outLen) {
    const auto bytes = dynxxCryptoHashSha1(makeBytesView(inBytes, inLen));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *dynxx_crypto_hash_sha256(const byte *inBytes, size_t inLen, size_t *outLen) {
    const auto bytes = dynxxCryptoHashSha256(makeBytesView(inBytes, inLen));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *dynxx_crypto_base64_encode(const byte *inBytes, size_t inLen, bool noNewLines, size_t *outLen) {
    const auto bytes = dynxxCryptoBase64Encode(makeBytesView(inBytes, inLen), noNewLines);
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *dynxx_crypto_base64_decode(const byte *inBytes, size_t inLen, bool noNewLines, size_t *outLen) {
    const auto bytes = dynxxCryptoBase64Decode(makeBytesView(inBytes, inLen), noNewLines);
    return handleBytes(bytes, outLen);
}

// Net.Http

EXPORT_AUTO
const char *dynxx_net_http_request(const char *url, const char *params, int method,
                                    const char **header_v, size_t header_c,
                                    const char **form_field_name_v,
                                    const char **form_field_mime_v,
                                    const char **form_field_data_v,
                                    size_t form_field_count,
                                    FILE *cFILE, size_t file_size,
                                    size_t timeout) {
    if (url == nullptr) {
        return "";
    }

    std::vector<std::string> vHeaders;
    if (header_v != nullptr && header_c > 0) {
        vHeaders = std::vector<std::string>(header_v, header_v + header_c);
    }

    std::vector<std::string> vFormFieldName;
    if (form_field_name_v != nullptr && form_field_count > 0) {
        vFormFieldName = std::vector<std::string>(form_field_name_v, form_field_name_v + form_field_count);
    }

    std::vector<std::string> vFormFieldMime;
    if (form_field_mime_v != nullptr && form_field_count > 0) {
        vFormFieldMime = std::vector<std::string>(form_field_mime_v, form_field_mime_v + form_field_count);
    }

    std::vector<std::string> vFormFieldData;
    if (form_field_data_v != nullptr && form_field_count > 0) {
        vFormFieldData = std::vector<std::string>(form_field_data_v, form_field_data_v + form_field_count);
    }

    const auto t = dynxxNetHttpRequest(url,
                                         static_cast<DynXXHttpMethodX>(method),
                                         params ? params : "",
                                         {},
                                         vHeaders, vFormFieldName, vFormFieldMime, vFormFieldData,
                                         cFILE, file_size, timeout);
    const auto s = t.toJson();
    return dupStr(s.value_or(""));
}

#if defined(USE_CURL)

EXPORT_AUTO
bool dynxx_net_http_download(const char *url, const char *file_path, size_t timeout) {
    if (url == nullptr || file_path == nullptr) {
        return false;
    }
    return dynxxNetHttpDownload(url, file_path, timeout);
}

#endif

// SQLite

#if defined(USE_DB)

EXPORT_AUTO
DynXXSQLiteConnHandle dynxx_sqlite_open(const char *_id) {
    return dynxxSQLiteOpen(makeStr(_id));
}

EXPORT_AUTO
bool dynxx_sqlite_execute(const DynXXSQLiteConnHandle conn, const char *sql) {
    if (conn == 0 || sql == nullptr) {
        return false;
    }
    return dynxxSQLiteExecute(conn, sql);
}

EXPORT_AUTO
DynXXSQLiteQueryResultHandle dynxx_sqlite_query_do(const DynXXSQLiteConnHandle conn, const char *sql) {
    if (conn == 0 || sql == nullptr) {
        return 0;
    }
    return dynxxSQLiteQueryDo(conn, sql);
}

EXPORT_AUTO
bool dynxx_sqlite_query_read_row(const DynXXSQLiteQueryResultHandle query_result) {
    return dynxxSQLiteQueryReadRow(query_result);
}

EXPORT_AUTO
const char *dynxx_sqlite_query_read_column_text(const DynXXSQLiteQueryResultHandle query_result, const char *column) {
    if (query_result == 0 || column == nullptr) {
        return "";
    }
    const auto s = dynxxSQLiteQueryReadColumnText(query_result, column);
    return dupStr(s.value_or(""));
}

EXPORT_AUTO
int64_t dynxx_sqlite_query_read_column_integer(const DynXXSQLiteQueryResultHandle query_result, const char *column) {
    if (query_result == 0 || column == nullptr) {
        return 0;
    }
    const auto i = dynxxSQLiteQueryReadColumnInteger(query_result, column);
    return i.value_or(0);
}

EXPORT_AUTO
double dynxx_sqlite_query_read_column_float(const DynXXSQLiteQueryResultHandle query_result, const char *column) {
    if (query_result == 0 || column == nullptr) {
        return 0.0;
    }
    const auto f = dynxxSQLiteQueryReadColumnFloat(query_result, column);
    return f.value_or(0.0);
}

EXPORT_AUTO
void dynxx_sqlite_query_drop(const DynXXSQLiteQueryResultHandle query_result) {
    dynxxSQLiteQueryDrop(query_result);
}

EXPORT_AUTO
void dynxx_sqlite_close(const DynXXSQLiteConnHandle conn) {
    dynxxSQLiteClose(conn);
}

#endif

// KV

#if defined(USE_KV)

EXPORT_AUTO
DynXXKVConnHandle dynxx_kv_open(const char *_id) {
    return dynxxKVOpen(makeStr(_id));
}

EXPORT_AUTO
const char *dynxx_kv_read_string(const DynXXKVConnHandle conn, const char *k) {
    if (conn == 0 || k == nullptr) {
        return nullptr;
    }
    const auto s = dynxxKVReadString(conn, k);
    return dupStr(s.value_or(""));
}

EXPORT_AUTO
bool dynxx_kv_write_string(const DynXXKVConnHandle conn, const char *k, const char *v) {
    if (conn == 0 || k == nullptr) {
        return false;
    }
    return dynxxKVWriteString(conn, k, v == nullptr ? "" : v);
}

EXPORT_AUTO
int64_t dynxx_kv_read_integer(const DynXXKVConnHandle conn, const char *k) {
    if (conn == 0 || k == nullptr) {
        return 0;
    }
    const auto i = dynxxKVReadInteger(conn, k);
    return i.value_or(0);
}

EXPORT_AUTO
bool dynxx_kv_write_integer(const DynXXKVConnHandle conn, const char *k, int64_t v) {
    if (conn == 0 || k == nullptr) {
        return false;
    }
    return dynxxKVWriteInteger(conn, k, v);
}

EXPORT_AUTO
double dynxx_kv_read_float(const DynXXKVConnHandle conn, const char *k) {
    if (conn == 0 || k == nullptr) {
        return 0.0;
    }
    const auto f = dynxxKVReadFloat(conn, k);
    return f.value_or(0.0);
}

EXPORT_AUTO
bool dynxx_kv_write_float(const DynXXKVConnHandle conn, const char *k, double v) {
    if (conn == 0 || k == nullptr) {
        return false;
    }
    return dynxxKVWriteFloat(conn, k, v);
}

EXPORT_AUTO
char *const *dynxx_kv_all_keys(const DynXXKVConnHandle conn, size_t *len) {
    const auto t = dynxxKVAllKeys(conn);
    if (len) {
        *len = t.size();
    }
    return copyStrVector(t, DYNXX_STORE_KV_KEY_MAX_LENGTH);
}

EXPORT_AUTO
bool dynxx_kv_contains(const DynXXKVConnHandle conn, const char *k) {
    if (conn == 0 || k == nullptr) {
        return false;
    }
    return dynxxKVContains(conn, k);
}

EXPORT_AUTO
bool dynxx_kv_remove(const DynXXKVConnHandle conn, const char *k) {
    if (conn == 0 || k == nullptr) {
        return false;
    }
    return dynxxKVRemove(conn, k);
}

EXPORT_AUTO
void dynxx_kv_clear(const DynXXKVConnHandle conn) {
    dynxxKVClear(conn);
}

EXPORT_AUTO
void dynxx_kv_close(const DynXXKVConnHandle conn) {
    dynxxKVClose(conn);
}

#endif

// Json.Decoder

EXPORT_AUTO
int dynxx_json_node_read_type(const DynXXJsonNodeHandle node) {
    return std::to_underlying(dynxxJsonNodeReadType(node));
}

EXPORT_AUTO
const char *dynxx_json_node_read_name(const DynXXJsonNodeHandle node) {
    const auto s = dynxxJsonNodeReadName(node);
    return dupStr(s.value_or(""));
}

EXPORT_AUTO
const char *dynxx_json_node_to_str(const DynXXJsonNodeHandle node) {
    const auto s = dynxxJsonNodeToStr(node);
    return dupStr(s.value_or(""));
}

EXPORT_AUTO
DynXXJsonDecoderHandle dynxx_json_decoder_init(const char *json) {
    return dynxxJsonDecoderInit(json);
}

EXPORT_AUTO
DynXXJsonNodeHandle dynxx_json_decoder_read_node(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node, const char *k) {
    if (decoder == 0 || k == nullptr) {
        return 0;
    }
    return dynxxJsonDecoderReadNode(decoder, k, node);
}

EXPORT_AUTO
const char *dynxx_json_decoder_read_string(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node) {
    const auto s = dynxxJsonDecoderReadString(decoder, node);
    return dupStr(s.value_or(""));
}

EXPORT_AUTO
int64_t dynxx_json_decoder_read_integer(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node) {
    return dynxxJsonDecoderReadInteger(decoder, node).value_or(0);
}

EXPORT_AUTO
double dynxx_json_decoder_read_float(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node) {
    return dynxxJsonDecoderReadFloat(decoder, node).value_or(0.0);
}

EXPORT_AUTO
DynXXJsonNodeHandle dynxx_json_decoder_read_child(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node) {
    return dynxxJsonDecoderReadChild(decoder, node);
}

EXPORT_AUTO
int dynxx_json_decoder_read_children_count(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node) {
    return dynxxJsonDecoderReadChildrenCount(decoder, node);
}

EXPORT_AUTO
DynXXJsonNodeHandle dynxx_json_decoder_read_next(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node) {
    return dynxxJsonDecoderReadNext(decoder, node);
}

EXPORT_AUTO
void dynxx_json_decoder_release(const DynXXJsonDecoderHandle decoder) {
    dynxxJsonDecoderRelease(decoder);
}

// Zip

EXPORT_AUTO
DynXXZipHandle dynxx_z_zip_init(int mode, size_t bufferSize, int format) {
    return dynxxZZipInit(static_cast<DynXXZipCompressModeX>(mode), bufferSize, static_cast<DynXXZFormatX>(format));
}

EXPORT_AUTO
size_t dynxx_z_zip_input(const DynXXZipHandle zip, const byte *inBytes, size_t inLen, bool inFinish) {
    return dynxxZZipInput(zip, makeBytes(inBytes, inLen), inFinish);
}

EXPORT_AUTO
const byte *dynxx_z_zip_process_do(const DynXXZipHandle zip, size_t *outLen) {
    const auto bytes = dynxxZZipProcessDo(zip);
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
bool dynxx_z_zip_process_finished(const DynXXZipHandle zip) {
    return dynxxZZipProcessFinished(zip);
}

EXPORT_AUTO
void dynxx_z_zip_release(const DynXXZipHandle zip) {
    dynxxZZipRelease(zip);
}

EXPORT_AUTO
DynXXUnZipHandle dynxx_z_unzip_init(size_t bufferSize, int format) {
    return dynxxZUnzipInit(bufferSize, static_cast<DynXXZFormatX>(format));
}

EXPORT_AUTO
size_t dynxx_z_unzip_input(const DynXXUnZipHandle unzip, const byte *inBytes, size_t inLen, bool inFinish) {
    return dynxxZUnzipInput(unzip, makeBytes(inBytes, inLen), inFinish);
}

EXPORT_AUTO
const byte *dynxx_z_unzip_process_do(const DynXXUnZipHandle unzip, size_t *outLen) {
    const auto bytes = dynxxZUnzipProcessDo(unzip);
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
bool dynxx_z_unzip_process_finished(const DynXXUnZipHandle unzip) {
    return dynxxZUnzipProcessFinished(unzip);
}

EXPORT_AUTO
void dynxx_z_unzip_release(const DynXXUnZipHandle unzip) {
    dynxxZUnzipRelease(unzip);
}

#if !defined(__EMSCRIPTEN__)

EXPORT_AUTO
bool dynxx_z_cfile_zip(int mode, size_t bufferSize, int format, FILE *cFILEIn, FILE *cFILEOut) {
    return dynxxZCFileZip(cFILEIn, cFILEOut,
                           static_cast<DynXXZipCompressModeX>(mode), bufferSize, static_cast<DynXXZFormatX>(format));
}

EXPORT_AUTO
bool dynxx_z_cfile_unzip(size_t bufferSize, int format, FILE *cFILEIn, FILE *cFILEOut) {
    return dynxxZCFileUnzip(cFILEIn, cFILEOut, bufferSize, static_cast<DynXXZFormatX>(format));
}

#endif

EXPORT_AUTO
const byte *dynxx_z_bytes_zip(int mode, size_t bufferSize, int format, const byte *inBytes, size_t inLen,
                               size_t *outLen) {
    const auto bytes = dynxxZBytesZip(makeBytes(inBytes, inLen),
                                        static_cast<DynXXZipCompressModeX>(mode), bufferSize,
                                        static_cast<DynXXZFormatX>(format));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const byte *dynxx_z_bytes_unzip(size_t bufferSize, int format, const byte *inBytes, size_t inLen, size_t *outLen) {
    const auto bytes = dynxxZBytesUnzip(makeBytes(inBytes, inLen),
                                          bufferSize, static_cast<DynXXZFormatX>(format));
    return handleBytes(bytes, outLen);
}
