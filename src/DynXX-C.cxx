#include <DynXX/C/DynXX.h>
#include <DynXX/CXX/DynXX.hxx>

#include "core/util/TypeUtil.hxx"

namespace {
    using namespace DynXX::Core::Util::Type;

    byte *handleOutBytes(BytesView bytes, size_t *outLen = nullptr) {
        if (outLen != nullptr) [[likely]] {
            *outLen = bytes.size();
        }
        return dupBytes(bytes);
    }

    const char **handleOutCharsArray(const std::vector<std::string> &sv, size_t maxLen, size_t *outLen = nullptr) {
        if (outLen != nullptr) [[likely]] {
            *outLen = sv.size();
        }
        return dupCharsArray(sv, maxLen);
    }
}

DYNXX_EXPORT_AUTO
const char *dynxx_get_version() {
    const auto s = dynxxGetVersion();
    return dupStr(s);
}

#if defined(USE_DB) || defined(USE_KV)
DYNXX_EXPORT_AUTO
const char *dynxx_root_path() {
    const auto s = dynxxRootPath().value_or("");
    return dupStr(s);
}
#endif

DYNXX_EXPORT
bool dynxx_init(const char *root) {
    return dynxxInit(root);
}

DYNXX_EXPORT
void dynxx_release() {
    dynxxRelease();
}

// Device.DeviceInfo

#if defined(USE_DEVICE)

DYNXX_EXPORT_AUTO
DynXXDeviceType dynxx_device_type() {
    return static_cast<DynXXDeviceType>(dynxxDeviceType());
}

DYNXX_EXPORT_AUTO
const char *dynxx_device_name() {
    const auto s = dynxxDeviceName();
    return dupStr(s);
}

DYNXX_EXPORT_AUTO
const char *dynxx_device_manufacturer() {
    const auto s = dynxxDeviceManufacturer();
    return dupStr(s);
}

DYNXX_EXPORT_AUTO
const char *dynxx_device_model() {
    const auto s = dynxxDeviceModel();
    return dupStr(s);
}

DYNXX_EXPORT_AUTO
const char *dynxx_device_os_version() {
    const auto s = dynxxDeviceOsVersion();
    return dupStr(s);
}

DYNXX_EXPORT_AUTO
DynXXDeviceCpuArch dynxx_device_cpu_arch() {
    return static_cast<DynXXDeviceCpuArch>(dynxxDeviceCpuArch());
}

#endif

// Log

DYNXX_EXPORT_AUTO
void dynxx_log_set_level(DynXXLogLevel level) {
    dynxxLogSetLevel(static_cast<DynXXLogLevelX>(level));
}

DYNXX_EXPORT_AUTO
void dynxx_log_set_callback(void (*const callback)(int level, const char *content)) {
    dynxxLogSetCallback(callback);
}

DYNXX_EXPORT_AUTO
void dynxx_log_print(DynXXLogLevel level, const char *content) {
    if (content == nullptr) {
        return;
    }
    dynxxLogPrint(static_cast<DynXXLogLevelX>(level), content);
}

// Coding

DYNXX_EXPORT_AUTO
const char *dynxx_coding_case_upper(const char *str) {
    if (str == nullptr) {
        return "";
    }
    const auto s = dynxxCodingCaseUpper(str);
    return dupStr(s);
}

DYNXX_EXPORT_AUTO
const char *dynxx_coding_case_lower(const char *str) {
    if (str == nullptr) {
        return "";
    }
    const auto s = dynxxCodingCaseLower(str);
    return dupStr(s);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_coding_hex_str2bytes(const char *str, size_t *outLen) {
    if (str == nullptr) {
        return nullptr;
    }
    const auto bytes = dynxxCodingHexStr2bytes(str);
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
const char *dynxx_coding_hex_bytes2str(const byte *inBytes, size_t inLen) {
    const auto s = dynxxCodingHexBytes2str(makeBytesView(inBytes, inLen));
    return dupStr(s);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_coding_str2bytes(const char *str, size_t *outLen) {
    if (str == nullptr) {
        return nullptr;
    }
    const auto bytes = dynxxCodingStr2bytes(str);
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
const char *dynxx_coding_bytes2str(const byte *inBytes, size_t inLen) {
    const auto s = dynxxCodingBytes2str(makeBytesView(inBytes, inLen));
    return dupStr(s);
}

DYNXX_EXPORT_AUTO
const char *dynxx_coding_str_trim(const char *str) {
    if (str == nullptr) {
        return "";
    }
    const auto s = dynxxCodingStrTrim(str);
    return dupStr(s);
}

// Crypto

DYNXX_EXPORT_AUTO
const byte *dynxx_crypto_rand(size_t len) {
    const auto bytes = dynxxCryptoRand(len);
    return handleOutBytes(bytes);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_crypto_aes_encrypt(const byte *inBytes, size_t inLen, const byte *keyBytes, size_t keyLen,
                                      size_t *outLen) {
    const auto bytes = dynxxCryptoAesEncrypt(makeBytesView(inBytes, inLen), makeBytesView(keyBytes, keyLen));
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_crypto_aes_decrypt(const byte *inBytes, size_t inLen, const byte *keyBytes, size_t keyLen,
                                      size_t *outLen) {
    const auto bytes = dynxxCryptoAesDecrypt(makeBytesView(inBytes, inLen), makeBytesView(keyBytes, keyLen));
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_crypto_aes_gcm_encrypt(const byte *inBytes, size_t inLen,
                                          const byte *keyBytes, size_t keyLen,
                                          const byte *initVectorBytes, size_t initVectorLen,
                                          const byte *aadBytes, size_t aadLen,
                                          size_t tagBits, size_t *outLen) {
    const auto bytes = dynxxCryptoAesGcmEncrypt(makeBytesView(inBytes, inLen), makeBytesView(keyBytes, keyLen),
                                                  makeBytesView(initVectorBytes, initVectorLen), tagBits,
                                                  makeBytesView(aadBytes, aadLen));
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_crypto_aes_gcm_decrypt(const byte *inBytes, size_t inLen,
                                          const byte *keyBytes, size_t keyLen,
                                          const byte *initVectorBytes, size_t initVectorLen,
                                          const byte *aadBytes, size_t aadLen,
                                          size_t tagBits, size_t *outLen) {
    const auto bytes = dynxxCryptoAesGcmDecrypt(makeBytesView(inBytes, inLen), makeBytesView(keyBytes, keyLen),
                                                  makeBytesView(initVectorBytes, initVectorLen), tagBits,
                                                  makeBytesView(aadBytes, aadLen));
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
const char *dynxx_crypto_rsa_gen_key(const char *base64, bool isPublic) {
    if (base64 == nullptr) {
        return "";
    }
    const auto s = dynxxCryptoRsaGenKey(base64, isPublic);
    return dupStr(s);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_crypto_rsa_encrypt(const byte *inBytes, size_t inLen,
                                      const byte *keyBytes, size_t keyLen, DynXXCryptoRSAPadding padding, size_t *outLen) {
    const auto bytes = dynxxCryptoRsaEncrypt(makeBytesView(inBytes, inLen), makeBytesView(keyBytes, keyLen),
                                               static_cast<DynXXCryptoRSAPaddingX>(padding));
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_crypto_rsa_decrypt(const byte *inBytes, size_t inLen,
                                      const byte *keyBytes, size_t keyLen, DynXXCryptoRSAPadding padding, size_t *outLen) {
    const auto bytes = dynxxCryptoRsaDecrypt(makeBytesView(inBytes, inLen), makeBytesView(keyBytes, keyLen),
                                               static_cast<DynXXCryptoRSAPaddingX>(padding));
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_crypto_hash_md5(const byte *inBytes, size_t inLen, size_t *outLen) {
    const auto bytes = dynxxCryptoHashMd5(makeBytesView(inBytes, inLen));
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_crypto_hash_sha1(const byte *inBytes, size_t inLen, size_t *outLen) {
    const auto bytes = dynxxCryptoHashSha1(makeBytesView(inBytes, inLen));
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_crypto_hash_sha256(const byte *inBytes, size_t inLen, size_t *outLen) {
    const auto bytes = dynxxCryptoHashSha256(makeBytesView(inBytes, inLen));
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_crypto_base64_encode(const byte *inBytes, size_t inLen, bool noNewLines, size_t *outLen) {
    const auto bytes = dynxxCryptoBase64Encode(makeBytesView(inBytes, inLen), noNewLines);
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_crypto_base64_decode(const byte *inBytes, size_t inLen, bool noNewLines, size_t *outLen) {
    const auto bytes = dynxxCryptoBase64Decode(makeBytesView(inBytes, inLen), noNewLines);
    return handleOutBytes(bytes, outLen);
}

// Net.Http

DYNXX_EXPORT_AUTO
const char *dynxx_net_http_request(const char *url, const char *params, DynXXHttpMethod method,
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
                                         params != nullptr ? params : "",
                                         {},
                                         vHeaders, vFormFieldName, vFormFieldMime, vFormFieldData,
                                         cFILE, file_size, timeout);
    const auto s = t.toJson();
    return dupStr(s.value_or(""));
}

#if defined(USE_CURL)

DYNXX_EXPORT_AUTO
bool dynxx_net_http_download(const char *url, const char *file_path, size_t timeout) {
    if (url == nullptr || file_path == nullptr) {
        return false;
    }
    return dynxxNetHttpDownload(url, file_path, timeout);
}

#endif

// SQLite

#if defined(USE_DB)

DYNXX_EXPORT_AUTO
DynXXSQLiteConnHandle dynxx_sqlite_open(const char *_id) {
    return dynxxSQLiteOpen(makeStr(_id));
}

DYNXX_EXPORT_AUTO
bool dynxx_sqlite_execute(DynXXSQLiteConnHandle conn, const char *sql) {
    if (conn == 0 || sql == nullptr) {
        return false;
    }
    return dynxxSQLiteExecute(conn, sql);
}

DYNXX_EXPORT_AUTO
DynXXSQLiteQueryResultHandle dynxx_sqlite_query_do(DynXXSQLiteConnHandle conn, const char *sql) {
    if (conn == 0 || sql == nullptr) {
        return 0;
    }
    return dynxxSQLiteQueryDo(conn, sql);
}

DYNXX_EXPORT_AUTO
bool dynxx_sqlite_query_read_row(DynXXSQLiteQueryResultHandle query_result) {
    return dynxxSQLiteQueryReadRow(query_result);
}

DYNXX_EXPORT_AUTO
const char *dynxx_sqlite_query_read_column_text(DynXXSQLiteQueryResultHandle query_result, const char *column) {
    if (query_result == 0 || column == nullptr) {
        return "";
    }
    const auto s = dynxxSQLiteQueryReadColumnText(query_result, column);
    return dupStr(s.value_or(""));
}

DYNXX_EXPORT_AUTO
int64_t dynxx_sqlite_query_read_column_integer(DynXXSQLiteQueryResultHandle query_result, const char *column) {
    if (query_result == 0 || column == nullptr) {
        return 0;
    }
    const auto i = dynxxSQLiteQueryReadColumnInteger(query_result, column);
    return i.value_or(0);
}

DYNXX_EXPORT_AUTO
double dynxx_sqlite_query_read_column_float(DynXXSQLiteQueryResultHandle query_result, const char *column) {
    if (query_result == 0 || column == nullptr) {
        return 0.0;
    }
    const auto f = dynxxSQLiteQueryReadColumnFloat(query_result, column);
    return f.value_or(0.0);
}

DYNXX_EXPORT_AUTO
void dynxx_sqlite_query_drop(DynXXSQLiteQueryResultHandle query_result) {
    dynxxSQLiteQueryDrop(query_result);
}

DYNXX_EXPORT_AUTO
void dynxx_sqlite_close(DynXXSQLiteConnHandle conn) {
    dynxxSQLiteClose(conn);
}

#endif

// KV

#if defined(USE_KV)

DYNXX_EXPORT_AUTO
DynXXKVConnHandle dynxx_kv_open(const char *_id) {
    return dynxxKVOpen(makeStr(_id));
}

DYNXX_EXPORT_AUTO
const char *dynxx_kv_read_string(DynXXKVConnHandle conn, const char *k) {
    if (conn == 0 || k == nullptr) {
        return nullptr;
    }
    const auto s = dynxxKVReadString(conn, k);
    return dupStr(s.value_or(""));
}

DYNXX_EXPORT_AUTO
bool dynxx_kv_write_string(DynXXKVConnHandle conn, const char *k, const char *v) {
    if (conn == 0 || k == nullptr) {
        return false;
    }
    return dynxxKVWriteString(conn, k, v == nullptr ? "" : v);
}

DYNXX_EXPORT_AUTO
int64_t dynxx_kv_read_integer(DynXXKVConnHandle conn, const char *k) {
    if (conn == 0 || k == nullptr) {
        return 0;
    }
    const auto i = dynxxKVReadInteger(conn, k);
    return i.value_or(0);
}

DYNXX_EXPORT_AUTO
bool dynxx_kv_write_integer(DynXXKVConnHandle conn, const char *k, int64_t v) {
    if (conn == 0 || k == nullptr) {
        return false;
    }
    return dynxxKVWriteInteger(conn, k, v);
}

DYNXX_EXPORT_AUTO
double dynxx_kv_read_float(DynXXKVConnHandle conn, const char *k) {
    if (conn == 0 || k == nullptr) {
        return 0.0;
    }
    const auto f = dynxxKVReadFloat(conn, k);
    return f.value_or(0.0);
}

DYNXX_EXPORT_AUTO
bool dynxx_kv_write_float(DynXXKVConnHandle conn, const char *k, double v) {
    if (conn == 0 || k == nullptr) {
        return false;
    }
    return dynxxKVWriteFloat(conn, k, v);
}

DYNXX_EXPORT_AUTO
const char **dynxx_kv_all_keys(DynXXKVConnHandle conn, size_t *outLen) {
    const auto t = dynxxKVAllKeys(conn);
    return handleOutCharsArray(t, DYNXX_STORE_KV_KEY_MAX_LENGTH, outLen);
}

DYNXX_EXPORT_AUTO
bool dynxx_kv_contains(DynXXKVConnHandle conn, const char *k) {
    if (conn == 0 || k == nullptr) {
        return false;
    }
    return dynxxKVContains(conn, k);
}

DYNXX_EXPORT_AUTO
bool dynxx_kv_remove(DynXXKVConnHandle conn, const char *k) {
    if (conn == 0 || k == nullptr) {
        return false;
    }
    return dynxxKVRemove(conn, k);
}

DYNXX_EXPORT_AUTO
void dynxx_kv_clear(DynXXKVConnHandle conn) {
    dynxxKVClear(conn);
}

DYNXX_EXPORT_AUTO
void dynxx_kv_close(DynXXKVConnHandle conn) {
    dynxxKVClose(conn);
}

#endif

// Json.Decoder

DYNXX_EXPORT_AUTO
DynXXJsonNodeType dynxx_json_node_read_type(DynXXJsonNodeHandle node) {
    return static_cast<DynXXJsonNodeType>(dynxxJsonNodeReadType(node));
}

DYNXX_EXPORT_AUTO
const char *dynxx_json_node_read_name(DynXXJsonNodeHandle node) {
    const auto s = dynxxJsonNodeReadName(node);
    return dupStr(s.value_or(""));
}

DYNXX_EXPORT_AUTO
const char *dynxx_json_node_to_str(DynXXJsonNodeHandle node) {
    const auto s = dynxxJsonNodeToStr(node);
    return dupStr(s.value_or(""));
}

DYNXX_EXPORT_AUTO
DynXXJsonDecoderHandle dynxx_json_decoder_init(const char *json) {
    return dynxxJsonDecoderInit(json);
}

DYNXX_EXPORT_AUTO
DynXXJsonNodeHandle dynxx_json_decoder_read_node(DynXXJsonDecoderHandle decoder, DynXXJsonNodeHandle node, const char *k) {
    if (decoder == 0 || k == nullptr) {
        return 0;
    }
    return dynxxJsonDecoderReadNode(decoder, k, node);
}

DYNXX_EXPORT_AUTO
const char *dynxx_json_decoder_read_string(DynXXJsonDecoderHandle decoder, DynXXJsonNodeHandle node) {
    const auto s = dynxxJsonDecoderReadString(decoder, node);
    return dupStr(s.value_or(""));
}

DYNXX_EXPORT_AUTO
int64_t dynxx_json_decoder_read_integer(DynXXJsonDecoderHandle decoder, DynXXJsonNodeHandle node) {
    return dynxxJsonDecoderReadInteger(decoder, node).value_or(0);
}

DYNXX_EXPORT_AUTO
double dynxx_json_decoder_read_float(DynXXJsonDecoderHandle decoder, DynXXJsonNodeHandle node) {
    return dynxxJsonDecoderReadFloat(decoder, node).value_or(0.0);
}

DYNXX_EXPORT_AUTO
DynXXJsonNodeHandle dynxx_json_decoder_read_child(DynXXJsonDecoderHandle decoder, DynXXJsonNodeHandle node) {
    return dynxxJsonDecoderReadChild(decoder, node);
}

DYNXX_EXPORT_AUTO
size_t dynxx_json_decoder_read_children_count(DynXXJsonDecoderHandle decoder, DynXXJsonNodeHandle node) {
    return dynxxJsonDecoderReadChildrenCount(decoder, node);
}

DYNXX_EXPORT_AUTO
DynXXJsonNodeHandle dynxx_json_decoder_read_next(DynXXJsonDecoderHandle decoder, DynXXJsonNodeHandle node) {
    return dynxxJsonDecoderReadNext(decoder, node);
}

DYNXX_EXPORT_AUTO
void dynxx_json_decoder_release(DynXXJsonDecoderHandle decoder) {
    dynxxJsonDecoderRelease(decoder);
}

// Zip

DYNXX_EXPORT_AUTO
DynXXZipHandle dynxx_z_zip_init(DynXXZipCompressMode mode, size_t bufferSize, DynXXZFormat format) {
    return dynxxZZipInit(static_cast<DynXXZipCompressModeX>(mode), bufferSize, static_cast<DynXXZFormatX>(format));
}

DYNXX_EXPORT_AUTO
size_t dynxx_z_zip_input(DynXXZipHandle zip, const byte *inBytes, size_t inLen, bool inFinish) {
    return dynxxZZipInput(zip, makeBytes(inBytes, inLen), inFinish);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_z_zip_process_do(DynXXZipHandle zip, size_t *outLen) {
    const auto bytes = dynxxZZipProcessDo(zip);
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
bool dynxx_z_zip_process_finished(DynXXZipHandle zip) {
    return dynxxZZipProcessFinished(zip);
}

DYNXX_EXPORT_AUTO
void dynxx_z_zip_release(DynXXZipHandle zip) {
    dynxxZZipRelease(zip);
}

DYNXX_EXPORT_AUTO
DynXXUnZipHandle dynxx_z_unzip_init(size_t bufferSize, DynXXZFormat format) {
    return dynxxZUnzipInit(bufferSize, static_cast<DynXXZFormatX>(format));
}

DYNXX_EXPORT_AUTO
size_t dynxx_z_unzip_input(DynXXUnZipHandle unzip, const byte *inBytes, size_t inLen, bool inFinish) {
    return dynxxZUnzipInput(unzip, makeBytes(inBytes, inLen), inFinish);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_z_unzip_process_do(DynXXUnZipHandle unzip, size_t *outLen) {
    const auto bytes = dynxxZUnzipProcessDo(unzip);
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
bool dynxx_z_unzip_process_finished(DynXXUnZipHandle unzip) {
    return dynxxZUnzipProcessFinished(unzip);
}

DYNXX_EXPORT_AUTO
void dynxx_z_unzip_release(DynXXUnZipHandle unzip) {
    dynxxZUnzipRelease(unzip);
}

#if !defined(__EMSCRIPTEN__)

DYNXX_EXPORT_AUTO
bool dynxx_z_cfile_zip(DynXXZipCompressMode mode, size_t bufferSize, DynXXZFormat format, FILE *cFILEIn, FILE *cFILEOut) {
    return dynxxZCFileZip(cFILEIn, cFILEOut,
                           static_cast<DynXXZipCompressModeX>(mode), bufferSize, static_cast<DynXXZFormatX>(format));
}

DYNXX_EXPORT_AUTO
bool dynxx_z_cfile_unzip(size_t bufferSize, DynXXZFormat format, FILE *cFILEIn, FILE *cFILEOut) {
    return dynxxZCFileUnzip(cFILEIn, cFILEOut, bufferSize, static_cast<DynXXZFormatX>(format));
}

#endif

DYNXX_EXPORT_AUTO
const byte *dynxx_z_bytes_zip(DynXXZipCompressMode mode, size_t bufferSize, DynXXZFormat format, const byte *inBytes, size_t inLen,
                               size_t *outLen) {
    const auto bytes = dynxxZBytesZip(makeBytes(inBytes, inLen),
                                        static_cast<DynXXZipCompressModeX>(mode), bufferSize,
                                        static_cast<DynXXZFormatX>(format));
    return handleOutBytes(bytes, outLen);
}

DYNXX_EXPORT_AUTO
const byte *dynxx_z_bytes_unzip(size_t bufferSize, DynXXZFormat format, const byte *inBytes, size_t inLen, size_t *outLen) {
    const auto bytes = dynxxZBytesUnzip(makeBytes(inBytes, inLen),
                                          bufferSize, static_cast<DynXXZFormatX>(format));
    return handleOutBytes(bytes, outLen);
}
