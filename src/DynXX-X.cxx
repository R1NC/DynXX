#if defined(__cplusplus)

#include <memory>

#include <DynXX/CXX/DynXX.hxx>
#include "core/json/JsonCodec.hxx"
#include "core/zip/Zip.hxx"
#include "core/coding/Coding.hxx"
#include "core/crypto/Crypto.hxx"
#include "core/device/Device.hxx"
#include "core/log/Log.hxx"

#if defined(USE_CURL)
#include "core/net/HttpClient.hxx"
#else
#include "core/net/HttpClient-wasm.hxx"
#endif

#include <DynXX/CXX/Macro.hxx>

#if defined(USE_LUA)
#include "bridge/LuaBridge.hxx"
#endif

#if defined(USE_QJS)
#include "bridge/JSBridge.hxx"
#endif

#if defined(USE_DB)
#include "core/store/SQLite.hxx"
#endif

#if defined(USE_KV)
#include "core/store/KV.hxx"
#endif

#if defined(USE_STD_CHAR_CONV_INT)
#include <charconv>
#endif

#endif

namespace {
    auto constexpr VERSION = "1.0.0";

#if defined(USE_CURL)
    std::unique_ptr<DynXX::Core::Net::HttpClient> _http_client = nullptr;
#endif

#if defined(USE_DB)
    std::unique_ptr<DynXX::Core::Store::SQLite> _sqlite = nullptr;
#endif

#if defined(USE_KV)
    std::unique_ptr<DynXX::Core::Store::KV> _kv = nullptr;
#endif

#if defined(USE_KV) || defined(USE_DB)
    std::unique_ptr<const std::string> _root = nullptr;
#endif

#if defined(USE_STD_CHAR_CONV_INT)
    template <NumberT T>
    T fromChars(const std::string &str, const T defaultV)
    {
        if (str.empty())
        {
            return defaultV;
        }
        const auto begin = str.data();
        const auto end = begin + str.size();
        T v;
        auto [ptr, ec] = std::from_chars(begin, end, v);
        if (ec == std::errc() && ptr == end) [[likely]]
        {
            return v;
        }
        return defaultV;
    }
#endif

    template<IntegerT T>
    T s2n(const std::string &str, T defaultValue, T (*f)(const std::string &, size_t *, int)) {
        try {
            return f(str, nullptr, 10);
        } catch (const std::exception &e) {
            dynxxLogPrintF(DynXXLogLevelX::Error, "s2n<i> err: {}", e.what());
            return defaultValue;
        }
    }

    template<FloatT T>
    T s2n(const std::string &str, T defaultValue, T (*f)(const std::string &, size_t *)) {
        try {
            return f(str, nullptr);
        } catch (const std::exception &e) {
            dynxxLogPrintF(DynXXLogLevelX::Error, "s2n<f> err: {}", e.what());
            return defaultValue;
        }
    }
}

int32_t str2int32(const std::string &str, const int32_t defaultI) {
#if defined(USE_STD_CHAR_CONV_INT)
    return fromChars<int32_t>(str, defaultI);
#else
    return s2n<int>(str, defaultI, std::stoi);
#endif
}

int64_t str2int64(const std::string &str, const int64_t defaultI) {
#if defined(USE_STD_CHAR_CONV_INT)
    return fromChars<int64_t>(str, defaultI);
#else
    return s2n<long long>(str, defaultI, std::stoll);
#endif
}

float str2float32(const std::string &str, const float defaultF) {
#if defined(USE_STD_CHAR_CONV_FLOAT)
    return fromChars<float>(str, defaultF);
#else
    return s2n<float>(str, defaultF, std::stof);
#endif
}

double str2float64(const std::string &str, const double defaultF) {
#if defined(USE_STD_CHAR_CONV_FLOAT)
    return fromChars<double>(str, defaultF);
#else
    return s2n<double>(str, defaultF, std::stod);
#endif
}

std::string dynxxGetVersion() {
    return VERSION;
}

#if defined(USE_KV) || defined(USE_DB)
std::string dynxxRootPath() {
    if (!_root) {
        return {};
    }
    return *_root;
}
#endif

bool dynxxInit(const std::string &root) {
#if defined(USE_KV) || defined(USE_DB)
    if (_root) {
        return true;
    }
    if (root.empty()) {
        return false;
    }
    _root = std::make_unique<const std::string>(root);
#endif
    
#if defined(USE_DB)
    _sqlite = std::make_unique<DynXX::Core::Store::SQLite>();
#endif

#if defined(USE_KV)
    _kv = std::make_unique<DynXX::Core::Store::KV>(*_root);
#endif

#if defined(USE_CURL)
    _http_client = std::make_unique<DynXX::Core::Net::HttpClient>();
#endif

#if defined(USE_LUA)
    dynxx_lua_init();
#endif

#if defined(USE_QJS)
    dynxx_js_init();
#endif

    return true;
}

void dynxxRelease() {
#if defined(USE_KV) || defined(USE_DB)
    if (!_root) {
        return;
    }
    _root.reset();
#endif

#if defined(USE_CURL)
    _http_client.reset();
#endif

#if defined(USE_DB)
    _sqlite->closeAll();
    _sqlite.reset();
#endif

#if defined(USE_KV)
    _kv->closeAll();
    _kv.reset();
#endif

    dynxxLogSetCallback(nullptr);

#if defined(USE_LUA)
    dynxx_lua_release();
#endif

#if defined(USE_QJS)
    dynxx_js_release();
#endif
}

#pragma mark Device.Device

DynXXDeviceTypeX dynxxDeviceType() {
    return static_cast<DynXXDeviceTypeX>(DynXX::Core::Device::deviceType());
}

std::string dynxxDeviceName() {
    return DynXX::Core::Device::deviceName();
}

std::string dynxxDeviceManufacturer() {
    return DynXX::Core::Device::deviceManufacturer();
}

std::string dynxxDeviceModel() {
    return DynXX::Core::Device::deviceModel();
}

std::string dynxxDeviceOsVersion() {
    return DynXX::Core::Device::osVersion();
}

DynXXDeviceCpuArchX dynxxDeviceCpuArch() {
    return static_cast<DynXXDeviceCpuArchX>(DynXX::Core::Device::cpuArch());
}

#pragma mark Log

void dynxxLogSetLevel(DynXXLogLevelX level) {
    DynXX::Core::Log::setLevel(static_cast<int>(level));
}

void dynxxLogSetCallback(const std::function<void(int level, const char *content)> &callback) {
    DynXX::Core::Log::setCallback(callback);
}

void dynxxLogPrint(DynXXLogLevelX level, std::string_view content) {
    DynXX::Core::Log::print(static_cast<int>(level), content);
}

#pragma mark Coding

std::string dynxxCodingCaseUpper(std::string_view str) {
    return DynXX::Core::Coding::Case::upper(str);
}

std::string dynxxCodingCaseLower(std::string_view str) {
    return DynXX::Core::Coding::Case::lower(str);
}

std::string dynxxCodingHexBytes2str(BytesView bytes) {
    return DynXX::Core::Coding::Hex::bytes2str(bytes);
}

Bytes dynxxCodingHexStr2bytes(const std::string &str) {
    return DynXX::Core::Coding::Hex::str2bytes(str);
}

std::string dynxxCodingBytes2str(BytesView bytes) {
    return DynXX::Core::Coding::bytes2str(bytes);
}

Bytes dynxxCodingStr2bytes(std::string_view str) {
    return DynXX::Core::Coding::str2bytes(str);
}

std::string dynxxCodingStrTrim(std::string_view str) {
    return DynXX::Core::Coding::strTrim(str);
}

#pragma mark Crypto

bool dynxxCryptoRand(size_t len, byte *bytes) {
    return DynXX::Core::Crypto::rand(len, bytes);
}

Bytes dynxxCryptoAesEncrypt(BytesView in, BytesView key) {
    return DynXX::Core::Crypto::AES::encrypt(in, key);
}

Bytes dynxxCryptoAesDecrypt(BytesView in, BytesView key) {
    return DynXX::Core::Crypto::AES::decrypt(in, key);
}

Bytes dynxxCryptoAesGcmEncrypt(BytesView in, BytesView key, BytesView initVector, size_t tagBits,
                                BytesView aad) {
    return DynXX::Core::Crypto::AES::gcmEncrypt(in, key, initVector, aad, tagBits);
}

Bytes dynxxCryptoAesGcmDecrypt(BytesView in, BytesView key, BytesView initVector, size_t tagBits,
                                BytesView aad) {
    return DynXX::Core::Crypto::AES::gcmDecrypt(in, key, initVector, aad, tagBits);
}

std::string dynxxCryptoRsaGenKey(std::string_view base64, bool isPublic) {
    return DynXX::Core::Crypto::RSA::genKey(base64, isPublic);
}

Bytes dynxxCryptoRsaEncrypt(BytesView in, BytesView key, DynXXCryptoRSAPaddingX padding) {
    const auto enc = DynXX::Core::Crypto::RSA::Encrypt(key, static_cast<int>(padding));
    return enc.process(in).value_or(Bytes{});
}

Bytes dynxxCryptoRsaDecrypt(BytesView in, BytesView key, DynXXCryptoRSAPaddingX padding) {
    const auto dec = DynXX::Core::Crypto::RSA::Decrypt(key, static_cast<int>(padding));
    return dec.process(in).value_or(Bytes{});
}

Bytes dynxxCryptoHashMd5(BytesView in) {
    return DynXX::Core::Crypto::Hash::md5(in);
}

Bytes dynxxCryptoHashSha1(BytesView in) {
    return DynXX::Core::Crypto::Hash::sha1(in);
}

Bytes dynxxCryptoHashSha256(BytesView in) {
    return DynXX::Core::Crypto::Hash::sha256(in);
}

Bytes dynxxCryptoBase64Encode(BytesView in, bool noNewLines) {
    return DynXX::Core::Crypto::Base64::encode(in, noNewLines);
}

Bytes dynxxCryptoBase64Decode(BytesView in, bool noNewLines) {
    return DynXX::Core::Crypto::Base64::decode(in, noNewLines);
}

#pragma mark Net.Http

DynXXHttpResponse dynxxNetHttpRequest(std::string_view url,
                                        DynXXHttpMethodX method,
                                        std::string_view params,
                                        const BytesView rawBody,
                                        const std::vector<std::string> &headerV,
                                        const std::vector<std::string> &formFieldNameV,
                                        const std::vector<std::string> &formFieldMimeV,
                                        const std::vector<std::string> &formFieldDataV,
                                        const std::FILE *cFILE, size_t fileSize,
                                        size_t timeout) {
    DynXXHttpResponse rsp;
#if defined(USE_CURL)
    if (!_http_client) {
        return rsp;
    }
#endif
    if (url.empty()) {
        return rsp;
    }

#if defined(USE_CURL)
    std::vector<DynXX::Core::Net::HttpFormField> vFormFields;
    auto fieldNameCount = formFieldNameV.size();
    if (fieldNameCount > 0) {
        vFormFields.reserve(fieldNameCount);
    }
    for (decltype(fieldNameCount) i(0); i < fieldNameCount && i < formFieldMimeV.size() && i < formFieldDataV.size(); i
         ++) {
        vFormFields.emplace_back(DynXX::Core::Net::HttpFormField{
            formFieldNameV[i],
            formFieldMimeV[i],
            formFieldDataV[i],
        });
    }

    return _http_client->request(url, static_cast<int>(method), headerV, params, rawBody, vFormFields, cFILE, fileSize,
                                 timeout);
#else
    return DynXX::Core::Net::WasmHttpClient::request(url, static_cast<int>(method), headerV, params, rawBody, timeout);
#endif
}

std::optional<std::string> DynXXHttpResponse::toJson() const {
    const auto cj = cJSON_CreateObject();

    if (!cJSON_AddNumberToObject(cj, "code", this->code)) [[unlikely]] {
        cJSON_Delete(cj);
        return std::nullopt;
    }

    if (!cJSON_AddStringToObject(cj, "contentType", this->contentType.c_str())) [[unlikely]] {
        cJSON_Delete(cj);
        return std::nullopt;
    }

    const auto cjHeaders = cJSON_CreateObject();
    for (const auto &[k, v]: this->headers) {
        if (!cJSON_AddStringToObject(cjHeaders, k.c_str(), v.c_str())) [[unlikely]] {
            cJSON_Delete(cjHeaders);
            cJSON_Delete(cj);
            return std::nullopt;
        }
    }

    if (!cJSON_AddItemToObject(cj, "headers", cjHeaders)) [[unlikely]] {
        cJSON_Delete(cjHeaders);
        cJSON_Delete(cj);
        return std::nullopt;
    }

    if (!cJSON_AddStringToObject(cj, "data", this->data.c_str())) [[unlikely]] {
        cJSON_Delete(cj);
        return std::nullopt;
    }

    auto json = dynxxJsonToStr(cj);
    cJSON_Delete(cj);
    return json;
}

DynXXHttpResponse dynxxNetHttpRequest(std::string_view url,
                                        DynXXHttpMethodX method,
                                        const std::unordered_map<std::string, Any> &params,
                                        const BytesView rawBody,
                                        const std::unordered_map<std::string, std::string> &headers,
                                        const std::vector<std::string> &formFieldNameV,
                                        const std::vector<std::string> &formFieldMimeV,
                                        const std::vector<std::string> &formFieldDataV,
                                        const std::FILE *cFILE, size_t fileSize,
                                        size_t timeout) {
    std::stringstream ssParams;
    for (const auto &[k, v]: params) {
        if (!ssParams.str().empty()) {
            ssParams << "&";
        }
        ssParams << k << "=";
        std::visit(
            [&ssParams](const auto &x) {
                ssParams << x;
            },
            v
        );
    }
    std::vector<std::string> headerV;
    if (auto headersCount = headers.size(); headersCount > 0) {
        headerV.reserve(headersCount);
    }
    for (const auto &[k, v]: headers) 
    {
        std::string header;
        header.reserve(k.size() + 1 + v.size());
        header.append(k).append(":").append(v);
        headerV.emplace_back(std::move(header));
    }
    return dynxxNetHttpRequest(url, method, ssParams.str(), rawBody, headerV,
                                formFieldNameV, formFieldMimeV, formFieldDataV,
                                cFILE, fileSize, timeout);
}

#if defined(USE_CURL)
bool dynxxNetHttpDownload(std::string_view url, const std::string &filePath, size_t timeout) {
    if (!_http_client || url.empty() || filePath.empty()) {
        return false;
    }
    return _http_client->download(url, filePath, timeout);
}
#endif

#pragma mark Store.SQLite

#if defined(USE_DB)

void *dynxxStoreSqliteOpen(const std::string &_id) {
    if (!_sqlite || !_root || _id.empty()) {
        return nullptr;
    }
    std::string dbPath = *_root + "/" + _id + ".db";
    if (const auto ptr = _sqlite->connect(dbPath).lock()) [[likely]] {
        return ptr.get();
    }
    return nullptr;
}

bool dynxxStoreSqliteExecute(void *const conn, std::string_view sql) {
    if (conn == nullptr || sql.empty()) {
        return false;
    }
    const auto xconn = static_cast<DynXX::Core::Store::SQLite::Connection *>(conn);
    return xconn->execute(sql);
}

void *dynxxStoreSqliteQueryDo(void *const conn, std::string_view sql) {
    if (conn == nullptr || sql.empty()) {
        return nullptr;
    }
    const auto xconn = static_cast<DynXX::Core::Store::SQLite::Connection *>(conn);
    auto ptrQr = xconn->query(sql);
    return ptrQr ? ptrQr.release() : nullptr; //trans ownership to C API, not release the memory.
}

bool dynxxStoreSqliteQueryReadRow(void *const query_result) {
    if (query_result == nullptr) {
        return false;
    }
    const auto xqr = static_cast<DynXX::Core::Store::SQLite::Connection::QueryResult *>(query_result);
    return xqr->readRow();
}

std::optional<std::string> dynxxStoreSqliteQueryReadColumnText(void *const query_result, std::string_view column) {
    if (query_result == nullptr || column.empty()) {
        return std::nullopt;
    }
    const auto xqr = static_cast<DynXX::Core::Store::SQLite::Connection::QueryResult *>(query_result);
    auto v = (*xqr)[column];
    if (!v.has_value()) [[unlikely]] {
        return std::nullopt;
    }
    return std::make_optional(*std::get_if<std::string>(&v.value()));
}

std::optional<int64_t> dynxxStoreSqliteQueryReadColumnInteger(void *const query_result, std::string_view column) {
    if (query_result == nullptr || column.empty()) {
        return std::nullopt;
    }
    const auto xqr = static_cast<DynXX::Core::Store::SQLite::Connection::QueryResult *>(query_result);
    const auto v = (*xqr)[column];
    if (!v.has_value()) [[unlikely]] {
        return std::nullopt;
    }
    return std::make_optional(*std::get_if<int64_t>(&v.value()));
}

std::optional<double> dynxxStoreSqliteQueryReadColumnFloat(void *const query_result, std::string_view column) {
    if (query_result == nullptr || column.empty()) {
        return std::nullopt;
    }
    const auto xqr = static_cast<DynXX::Core::Store::SQLite::Connection::QueryResult *>(query_result);
    const auto v = (*xqr)[column];
    if (!v.has_value()) [[unlikely]] {
        return std::nullopt;
    }
    return std::make_optional(*std::get_if<double>(&v.value()));
}

void dynxxStoreSqliteQueryDrop(void *const query_result) {
    if (query_result == nullptr) {
        return;
    }
    const auto xqr = static_cast<DynXX::Core::Store::SQLite::Connection::QueryResult *>(query_result);
    delete xqr;
}

void dynxxStoreSqliteClose(void *const conn) {
    /*if (conn == nullptr)
        return;
    const auto xconn = static_cast<DynXX::Core::Store::SQLite::Connection *>(conn);
    delete xconn;*/
}

#endif

#pragma mark Store.KV

#if defined(USE_KV)

void *dynxxStoreKvOpen(const std::string &_id) {
    if (!_kv || _id.empty()) {
        return nullptr;
    }
    if (const auto ptr = _kv->open(_id).lock()) [[likely]] {
        return ptr.get();
    }
    return nullptr;
}

std::optional<std::string> dynxxStoreKvReadString(void *const conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return std::nullopt;
    }
    const auto xconn = static_cast<DynXX::Core::Store::KV::Connection *>(conn);
    return xconn->readString(k);
}

bool dynxxStoreKvWriteString(void *const conn, std::string_view k, const std::string &v) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<DynXX::Core::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

std::optional<int64_t> dynxxStoreKvReadInteger(void *const conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return std::nullopt;
    }
    const auto xconn = static_cast<DynXX::Core::Store::KV::Connection *>(conn);
    return xconn->readInteger(k);
}

bool dynxxStoreKvWriteInteger(void *const conn, std::string_view k, int64_t v) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<DynXX::Core::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

std::optional<double> dynxxStoreKvReadFloat(void *const conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return std::nullopt;
    }
    const auto xconn = static_cast<DynXX::Core::Store::KV::Connection *>(conn);
    return xconn->readFloat(k);
}

bool dynxxStoreKvWriteFloat(void *const conn, std::string_view k, double v) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<DynXX::Core::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

std::vector<std::string> dynxxStoreKvAllKeys(void *const conn) {
    if (conn == nullptr) {
        return {};
    }
    const auto xconn = static_cast<DynXX::Core::Store::KV::Connection *>(conn);
    return xconn->allKeys();
}

bool dynxxStoreKvContains(void *const conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<DynXX::Core::Store::KV::Connection *>(conn);
    return xconn->contains(k);
}

bool dynxxStoreKvRemove(void *const conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<DynXX::Core::Store::KV::Connection *>(conn);
    return xconn->remove(k);
}

void dynxxStoreKvClear(void *const conn) {
    if (conn == nullptr) {
        return;
    }
    const auto xconn = static_cast<DynXX::Core::Store::KV::Connection *>(conn);
    xconn->clear();
}

void dynxxStoreKvClose(void *const conn) {
    /*if (conn == nullptr)
        return;
    const auto xconn = static_cast<DynXX::Core::Store::KV::Connection *>(conn);
    delete xconn;*/
}

#endif

#pragma mark Json

DynXXJsonNodeTypeX dynxxJsonReadType(void *const cjson) {
    return DynXX::Core::Json::cJSONReadType(cjson);
}

std::optional<std::string> dynxxJsonToStr(void *const cjson) {
    return DynXX::Core::Json::cJSONToStr(cjson);
}

std::optional<std::string> dynxxJsonFromDictAny(const DictAny &dict) {
    return DynXX::Core::Json::jsonFromDictAny(dict);
}

std::optional<DictAny> dynxxJsonToDictAny(const std::string &json) {
    return DynXX::Core::Json::jsonToDictAny(json);
}

void *dynxxJsonDecoderInit(const std::string &json) {
    auto decoder = new(std::nothrow) DynXX::Core::Json::Decoder(json);
    if (decoder == nullptr) {
        dynxxLogPrint(DynXXLogLevelX::Error, "new JsonDecoder failed");
    }
    return decoder;
}

void *dynxxJsonDecoderReadNode(void *const decoder, std::string_view k, void *const node) {
    if (decoder == nullptr || k.empty()) {
        return nullptr;
    }
    const auto xdecoder = static_cast<DynXX::Core::Json::Decoder *>(decoder);
    return xdecoder->readNode(node, k);
}

std::optional<std::string> dynxxJsonDecoderReadString(void *const decoder, void *const node) {
    if (decoder == nullptr) {
        return std::nullopt;
    }
    const auto xdecoder = static_cast<DynXX::Core::Json::Decoder *>(decoder);
    return xdecoder->readString(node);
}

std::optional<double> dynxxJsonDecoderReadNumber(void *const decoder, void *const node) {
    if (decoder == nullptr) {
        return std::nullopt;
    }
    const auto xdecoder = static_cast<DynXX::Core::Json::Decoder *>(decoder);
    return xdecoder->readNumber(node);
}

void *dynxxJsonDecoderReadChild(void *const decoder, void *const node) {
    if (decoder == nullptr) {
        return nullptr;
    }
    const auto xdecoder = static_cast<DynXX::Core::Json::Decoder *>(decoder);
    return xdecoder->readChild(node);
}

size_t dynxxJsonDecoderReadChildrenCount(void *const decoder, void *const node) {
    if (decoder == nullptr) {
        return 0;
    }
    const auto xdecoder = static_cast<DynXX::Core::Json::Decoder *>(decoder);
    return xdecoder->readChildrenCount(node);
}

void dynxxJsonDecoderReadChildren(void *const decoder, std::function<void(size_t idx, const void *child)> &&callback,
                                   void *const node) {
    if (decoder == nullptr) {
        return;
    }
    const auto xdecoder = static_cast<DynXX::Core::Json::Decoder *>(decoder);
    xdecoder->readChildren(node, std::move(callback));
}

void *dynxxJsonDecoderReadNext(void *const decoder, void *const node) {
    if (decoder == nullptr) {
        return nullptr;
    }
    const auto xdecoder = static_cast<DynXX::Core::Json::Decoder *>(decoder);
    return xdecoder->readNext(node);
}

void dynxxJsonDecoderRelease(void *const decoder) {
    if (decoder == nullptr) {
        return;
    }
    const auto xdecoder = static_cast<DynXX::Core::Json::Decoder *>(decoder);
    delete xdecoder;
}

#pragma mark Zip

void *dynxxZZipInit(const DynXXZipCompressModeX mode, size_t bufferSize, const DynXXZFormatX format) {
    void *zip = nullptr;
    try {
        zip = new DynXX::Core::Z::Zip(static_cast<int>(mode), bufferSize, static_cast<int>(format));
    } catch (const std::exception &e) {
        dynxxLogPrintF(DynXXLogLevelX::Error, "dynxxZZipInit failed: {}", e.what());
    }
    return zip;
}

size_t dynxxZZipInput(void *const zip, const Bytes &inBytes, bool inFinish) {
    if (zip == nullptr) {
        return 0;
    }
    const auto xzip = static_cast<DynXX::Core::Z::Zip *>(zip);
    return xzip->input(inBytes, inFinish);
}

Bytes dynxxZZipProcessDo(void *const zip) {
    if (zip == nullptr) {
        return {};
    }
    const auto xzip = static_cast<DynXX::Core::Z::Zip *>(zip);
    return xzip->processDo();
}

bool dynxxZZipProcessFinished(void *const zip) {
    if (zip == nullptr) {
        return false;
    }
    const auto xzip = static_cast<DynXX::Core::Z::Zip *>(zip);
    return xzip->processFinished();
}

void dynxxZZipRelease(void *const zip) {
    if (zip == nullptr) {
        return;
    }
    const auto xzip = static_cast<DynXX::Core::Z::Zip *>(zip);
    delete xzip;
}

void *dynxxZUnzipInit(size_t bufferSize, const DynXXZFormatX format) {
    void *unzip = nullptr;
    try {
        unzip = new DynXX::Core::Z::UnZip(bufferSize, static_cast<int>(format));
    } catch (const std::exception &e) {
        dynxxLogPrintF(DynXXLogLevelX::Error, "dynxxZUnzipInit failed: {}", e.what());
    }
    return unzip;
}

size_t dynxxZUnzipInput(void *const unzip, const Bytes &inBytes, bool inFinish) {
    if (unzip == nullptr) {
        return 0;
    }
    const auto xunzip = static_cast<DynXX::Core::Z::UnZip *>(unzip);
    return xunzip->input(inBytes, inFinish);
}

Bytes dynxxZUnzipProcessDo(void *const unzip) {
    if (unzip == nullptr) {
        return {};
    }
    const auto xunzip = static_cast<DynXX::Core::Z::UnZip *>(unzip);
    return xunzip->processDo();
}

bool dynxxZUnzipProcessFinished(void *const unzip) {
    if (unzip == nullptr) {
        return false;
    }
    const auto xunzip = static_cast<DynXX::Core::Z::UnZip *>(unzip);
    return xunzip->processFinished();
}

void dynxxZUnzipRelease(void *const unzip) {
    if (unzip == nullptr) {
        return;
    }
    const auto xunzip = static_cast<DynXX::Core::Z::UnZip *>(unzip);
    delete xunzip;
}

#if !defined(__EMSCRIPTEN__)

bool dynxxZCFileZip(std::FILE *cFILEIn, std::FILE *cFILEOut, const DynXXZipCompressModeX mode, size_t bufferSize,
                     const DynXXZFormatX format) {
    if (bufferSize == 0) {
        return false;
    }
    if (cFILEIn == nullptr || cFILEOut == nullptr) {
        return false;
    }
    return DynXX::Core::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), cFILEIn, cFILEOut);
}

bool dynxxZCFileUnzip(std::FILE *cFILEIn, std::FILE *cFILEOut, size_t bufferSize, const DynXXZFormatX format) {
    if (bufferSize == 0) {
        return false;
    }
    if (cFILEIn == nullptr || cFILEOut == nullptr) {
        return false;
    }
    return DynXX::Core::Z::unzip(bufferSize, static_cast<int>(format), cFILEIn, cFILEOut);
}

bool dynxxZCxxStreamZip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut, const DynXXZipCompressModeX mode,
                         size_t bufferSize, const DynXXZFormatX format) {
    if (bufferSize == 0) {
        return false;
    }
    if (cxxStreamIn == nullptr || cxxStreamOut == nullptr) {
        return false;
    }
    return DynXX::Core::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), cxxStreamIn,
                                cxxStreamOut);
}

bool dynxxZCxxStreamUnzip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut, size_t bufferSize,
                           const DynXXZFormatX format) {
    if (bufferSize == 0) {
        return false;
    }
    if (cxxStreamIn == nullptr || cxxStreamOut == nullptr) {
        return false;
    }
    return DynXX::Core::Z::unzip(bufferSize, static_cast<int>(format), cxxStreamIn, cxxStreamOut);
}

#endif

Bytes dynxxZBytesZip(const Bytes &inBytes, const DynXXZipCompressModeX mode, size_t bufferSize,
                      const DynXXZFormatX format) {
    if (bufferSize == 0) {
        return {};
    }
    if (inBytes.empty()) {
        return {};
    }
    return DynXX::Core::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), inBytes);
}

Bytes dynxxZBytesUnzip(const Bytes &inBytes, size_t bufferSize, const DynXXZFormatX format) {
    if (bufferSize == 0) {
        return {};
    }
    if (inBytes.empty()) {
        return {};
    }
    return DynXX::Core::Z::unzip(bufferSize, static_cast<int>(format), inBytes);
}
