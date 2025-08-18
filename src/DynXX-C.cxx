#if defined(__cplusplus)

#include <DynXX/CXX/DynXX.hxx>
#include <DynXX/C/KV.h>
#include <DynXX/CXX/Types.hxx>
#include <DynXX/CXX/Macro.hxx>
#include "core/util/TypeUtil.hxx"
#endif

namespace {
    byte *handleBytes(const BytesView bytes, size_t *outLen) {
        if (outLen) [[likely]] {
            *outLen = bytes.size();
        }
        return DynXX::Core::Util::Type::copyBytes(bytes);
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
    const auto s = dynxxRootPath();
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

EXPORT_AUTO
int dynxx_device_type() {
    return static_cast<int>(dynxxDeviceType());
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
    return static_cast<int>(dynxxDeviceCpuArch());
}

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
    const auto bytes = dynxxCodingHexStr2bytes(makeStr(str));
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
const char *dynxx_coding_hex_bytes2str(const byte *inBytes, size_t inLen) {
    const auto s = dynxxCodingHexBytes2str(makeBytesView(inBytes, inLen));
    return dupStr(s);
}

EXPORT_AUTO
const char *dynxx_coding_bytes2str(const byte *inBytes, size_t inLen) {
    const auto s = dynxxCodingBytes2str(makeBytesView(inBytes, inLen));
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
const char *dynxx_coding_str_trim(const char *str) {
    if (str == nullptr) {
        return "";
    }
    const auto s = dynxxCodingStrTrim(str);
    return dupStr(s);
}

// Crypto

EXPORT_AUTO
bool dynxx_crypto_rand(size_t len, byte *bytes) {
    return dynxxCryptoRand(len, bytes);
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
                                    void *const cFILE, size_t file_size,
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
                                         static_cast<std::FILE *>(cFILE), file_size, timeout);
    const auto s = t.toJson();
    return dupStr(s.value_or(""));
}

#if defined(USE_CURL)

EXPORT_AUTO
bool dynxx_net_http_download(const char *url, const char *file_path, size_t timeout) {
    if (url == nullptr || file_path == nullptr) {
        return false;
    }
    return dynxxNetHttpDownload(url, makeStr(file_path), timeout);
}

#endif

// Store.SQLite

#if defined(USE_DB)

EXPORT_AUTO
void *dynxx_store_sqlite_open(const char *_id) {
    return dynxxStoreSqliteOpen(makeStr(_id));
}

EXPORT_AUTO
bool dynxx_store_sqlite_execute(void *const conn, const char *sql) {
    if (conn == nullptr || sql == nullptr) {
        return false;
    }
    return dynxxStoreSqliteExecute(conn, sql);
}

EXPORT_AUTO
void *dynxx_store_sqlite_query_do(void *const conn, const char *sql) {
    if (conn == nullptr || sql == nullptr) {
        return nullptr;
    }
    return dynxxStoreSqliteQueryDo(conn, sql);
}

EXPORT_AUTO
bool dynxx_store_sqlite_query_read_row(void *const query_result) {
    return dynxxStoreSqliteQueryReadRow(query_result);
}

EXPORT_AUTO
const char *dynxx_store_sqlite_query_read_column_text(void *const query_result, const char *column) {
    if (query_result == nullptr || column == nullptr) {
        return "";
    }
    const auto s = dynxxStoreSqliteQueryReadColumnText(query_result, column);
    return dupStr(s.value_or(""));
}

EXPORT_AUTO
int64_t dynxx_store_sqlite_query_read_column_integer(void *const query_result, const char *column) {
    if (query_result == nullptr || column == nullptr) {
        return 0;
    }
    const auto i = dynxxStoreSqliteQueryReadColumnInteger(query_result, column);
    return i.value_or(0);
}

EXPORT_AUTO
double dynxx_store_sqlite_query_read_column_float(void *const query_result, const char *column) {
    if (query_result == nullptr || column == nullptr) {
        return 0.0;
    }
    const auto f = dynxxStoreSqliteQueryReadColumnFloat(query_result, column);
    return f.value_or(0.0);
}

EXPORT_AUTO
void dynxx_store_sqlite_query_drop(void *const query_result) {
    dynxxStoreSqliteQueryDrop(query_result);
}

EXPORT_AUTO
void dynxx_store_sqlite_close(void *const conn) {
    dynxxStoreSqliteClose(conn);
}

#endif

// Store.KV

#if defined(USE_KV)

EXPORT_AUTO
void *dynxx_store_kv_open(const char *_id) {
    return dynxxStoreKvOpen(makeStr(_id));
}

EXPORT_AUTO
const char *dynxx_store_kv_read_string(void *const conn, const char *k) {
    if (conn == nullptr || k == nullptr) {
        return nullptr;
    }
    const auto s = dynxxStoreKvReadString(conn, k);
    return dupStr(s.value_or(""));
}

EXPORT_AUTO
bool dynxx_store_kv_write_string(void *const conn, const char *k, const char *v) {
    if (conn == nullptr || k == nullptr) {
        return false;
    }
    return dynxxStoreKvWriteString(conn, k, v == nullptr ? "" : v);
}

EXPORT_AUTO
int64_t dynxx_store_kv_read_integer(void *const conn, const char *k) {
    if (conn == nullptr || k == nullptr) {
        return 0;
    }
    const auto i = dynxxStoreKvReadInteger(conn, k);
    return i.value_or(0);
}

EXPORT_AUTO
bool dynxx_store_kv_write_integer(void *const conn, const char *k, int64_t v) {
    if (conn == nullptr || k == nullptr) {
        return false;
    }
    return dynxxStoreKvWriteInteger(conn, k, v);
}

EXPORT_AUTO
double dynxx_store_kv_read_float(void *const conn, const char *k) {
    if (conn == nullptr || k == nullptr) {
        return 0.0;
    }
    const auto f = dynxxStoreKvReadFloat(conn, k);
    return f.value_or(0.0);
}

EXPORT_AUTO
bool dynxx_store_kv_write_float(void *const conn, const char *k, double v) {
    if (conn == nullptr || k == nullptr) {
        return false;
    }
    return dynxxStoreKvWriteFloat(conn, k, v);
}

EXPORT_AUTO
char *const *dynxx_store_kv_all_keys(void *const conn, size_t *len) {
    const auto t = dynxxStoreKvAllKeys(conn);
    if (len) {
        *len = t.size();
    }
    return DynXX::Core::Util::Type::copyStrVector(t, DYNXX_STORE_KV_KEY_MAX_LENGTH);
}

EXPORT_AUTO
bool dynxx_store_kv_contains(void *const conn, const char *k) {
    if (conn == nullptr || k == nullptr) {
        return false;
    }
    return dynxxStoreKvContains(conn, k);
}

EXPORT_AUTO
bool dynxx_store_kv_remove(void *const conn, const char *k) {
    if (conn == nullptr || k == nullptr) {
        return false;
    }
    return dynxxStoreKvRemove(conn, k);
}

EXPORT_AUTO
void dynxx_store_kv_clear(void *const conn) {
    dynxxStoreKvClear(conn);
}

EXPORT_AUTO
void dynxx_store_kv_close(void *const conn) {
    dynxxStoreKvClose(conn);
}

#endif

// Json.Decoder

EXPORT_AUTO
int dynxx_json_read_type(void *const cjson) {
    return static_cast<int>(dynxxJsonReadType(cjson));
}

EXPORT_AUTO
const char *dynxx_json_to_str(void *const cjson) {
    const auto s = dynxxJsonToStr(cjson);
    return dupStr(s.value_or(""));
}

EXPORT_AUTO
void *dynxx_json_decoder_init(const char *json) {
    return dynxxJsonDecoderInit(makeStr(json));
}

EXPORT_AUTO
void *dynxx_json_decoder_read_node(void *const decoder, void *const node, const char *k) {
    if (decoder == nullptr || k == nullptr) {
        return nullptr;
    }
    return dynxxJsonDecoderReadNode(decoder, k, node);
}

EXPORT_AUTO
const char *dynxx_json_decoder_read_string(void *const decoder, void *const node) {
    const auto s = dynxxJsonDecoderReadString(decoder, node);
    return dupStr(s.value_or(""));
}

EXPORT_AUTO
double dynxx_json_decoder_read_number(void *const decoder, void *const node) {
    return dynxxJsonDecoderReadNumber(decoder, node).value_or(0.0);
}

EXPORT_AUTO
void *dynxx_json_decoder_read_child(void *const decoder, void *const node) {
    return dynxxJsonDecoderReadChild(decoder, node);
}

EXPORT_AUTO
void *dynxx_json_decoder_read_next(void *const decoder, void *const node) {
    return dynxxJsonDecoderReadNext(decoder, node);
}

EXPORT_AUTO
void dynxx_json_decoder_release(void *const decoder) {
    dynxxJsonDecoderRelease(decoder);
}

// Zip

EXPORT_AUTO
void *dynxx_z_zip_init(int mode, size_t bufferSize, int format) {
    return dynxxZZipInit(static_cast<DynXXZipCompressModeX>(mode), bufferSize, static_cast<DynXXZFormatX>(format));
}

EXPORT_AUTO
size_t dynxx_z_zip_input(void *const zip, const byte *inBytes, size_t inLen, bool inFinish) {
    return dynxxZZipInput(zip, makeBytes(inBytes, inLen), inFinish);
}

EXPORT_AUTO
const byte *dynxx_z_zip_process_do(void *const zip, size_t *outLen) {
    const auto bytes = dynxxZZipProcessDo(zip);
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
bool dynxx_z_zip_process_finished(void *const zip) {
    return dynxxZZipProcessFinished(zip);
}

EXPORT_AUTO
void dynxx_z_zip_release(void *const zip) {
    dynxxZZipRelease(zip);
}

EXPORT_AUTO
void *dynxx_z_unzip_init(size_t bufferSize, int format) {
    return dynxxZUnzipInit(bufferSize, static_cast<DynXXZFormatX>(format));
}

EXPORT_AUTO
size_t dynxx_z_unzip_input(void *const unzip, const byte *inBytes, size_t inLen, bool inFinish) {
    return dynxxZUnzipInput(unzip, makeBytes(inBytes, inLen), inFinish);
}

EXPORT_AUTO
const byte *dynxx_z_unzip_process_do(void *const unzip, size_t *outLen) {
    const auto bytes = dynxxZUnzipProcessDo(unzip);
    return handleBytes(bytes, outLen);
}

EXPORT_AUTO
bool dynxx_z_unzip_process_finished(void *const unzip) {
    return dynxxZUnzipProcessFinished(unzip);
}

EXPORT_AUTO
void dynxx_z_unzip_release(void *const unzip) {
    dynxxZUnzipRelease(unzip);
}

#if !defined(__EMSCRIPTEN__)

EXPORT_AUTO
bool dynxx_z_cfile_zip(int mode, size_t bufferSize, int format, void *const cFILEIn, void *const cFILEOut) {
    return dynxxZCFileZip(static_cast<std::FILE *>(cFILEIn), static_cast<std::FILE *>(cFILEOut),
                           static_cast<DynXXZipCompressModeX>(mode), bufferSize, static_cast<DynXXZFormatX>(format));
}

EXPORT_AUTO
bool dynxx_z_cfile_unzip(size_t bufferSize, int format, void *const cFILEIn, void *const cFILEOut) {
    return dynxxZCFileUnzip(static_cast<std::FILE *>(cFILEIn), static_cast<std::FILE *>(cFILEOut),
                             bufferSize, static_cast<DynXXZFormatX>(format));
}

EXPORT_AUTO
bool dynxx_z_cxxstream_zip(int mode, size_t bufferSize, int format, void *const cxxStreamIn,
                            void *const cxxStreamOut) {
    return dynxxZCxxStreamZip(static_cast<std::istream *>(cxxStreamIn), static_cast<std::ostream *>(cxxStreamOut),
                               static_cast<DynXXZipCompressModeX>(mode), bufferSize,
                               static_cast<DynXXZFormatX>(format));
}

EXPORT_AUTO
bool dynxx_z_cxxstream_unzip(size_t bufferSize, int format, void *const cxxStreamIn, void *const cxxStreamOut) {
    return dynxxZCxxStreamUnzip(static_cast<std::istream *>(cxxStreamIn), static_cast<std::ostream *>(cxxStreamOut),
                                 bufferSize, static_cast<DynXXZFormatX>(format));
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
