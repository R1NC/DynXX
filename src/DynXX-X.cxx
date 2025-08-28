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

    using namespace DynXX::Core;

#if defined(USE_CURL)
    std::unique_ptr<Net::HttpClient> _http_client = nullptr;
#endif

#if defined(USE_DB)
    std::unique_ptr<Store::SQLite::SQLiteStore> _sqlite = nullptr;
#endif

#if defined(USE_KV)
    std::unique_ptr<Store::KV::KVStore> _kv = nullptr;
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

    template<NumberT T>
    constexpr auto stox() {
        if constexpr (std::is_same_v<T, int>) {
            return [](const std::string& s, size_t* idx, int base) { return std::stoi(s, idx, base); };
        } else if constexpr (std::is_same_v<T, long>) {
            return [](const std::string& s, size_t* idx, int base) { return std::stol(s, idx, base); };
        } else if constexpr (std::is_same_v<T, long long>) {
            return [](const std::string& s, size_t* idx, int base) { return std::stoll(s, idx, base); };
        } else if constexpr (std::is_same_v<T, unsigned long long>) {
            return [](const std::string& s, size_t* idx, int base) { return std::stoull(s, idx, base); };
        } else if constexpr (std::is_same_v<T, unsigned long>) {
            return [](const std::string& s, size_t* idx, int base) { return std::stoul(s, idx, base); };
        } else if constexpr (std::is_same_v<T, unsigned long long>) {
            return [](const std::string& s, size_t* idx, int base) { return std::stoull(s, idx, base); };
        } else if constexpr (std::is_same_v<T, float>) {
            return [](const std::string& s, size_t* idx) { return std::stof(s, idx); };
        } else if constexpr (std::is_same_v<T, double>) {
            return [](const std::string& s, size_t* idx) { return std::stod(s, idx); };
        } else if constexpr (std::is_same_v<T, long double>) {
            return [](const std::string& s, size_t* idx) { return std::stold(s, idx); };
        } else {
            static_assert(std::is_arithmetic_v<T>, "Unsupported numeric type");
        }
    }

    template<IntegerT T>
    T s2n(const std::string &str, T defaultValue, T (*f)(const std::string &, size_t *, int)) {
        try {
            return f(str, nullptr, 10);
        } catch (const std::invalid_argument &e) {
            dynxxLogPrintF(DynXXLogLevelX::Error, "s2n<i> invalid_argument err: {}", e.what());
            return defaultValue;
        } catch (const std::out_of_range &e) {
            dynxxLogPrintF(DynXXLogLevelX::Error, "s2n<i> out_of_range err: {}", e.what());
            return defaultValue;
        }
    }

    template<FloatT T>
    T s2n(const std::string &str, T defaultValue, T (*f)(const std::string &, size_t *)) {
        try {
            return f(str, nullptr);
        } catch (const std::invalid_argument &e) {
            dynxxLogPrintF(DynXXLogLevelX::Error, "s2n<f> invalid_argument err: {}", e.what());
            return defaultValue;
        } catch (const std::out_of_range &e) {
            dynxxLogPrintF(DynXXLogLevelX::Error, "s2n<f> out_of_range err: {}", e.what());
            return defaultValue;
        }
    }
}

int32_t str2int32(const std::string &str, const int32_t defaultI) {
#if defined(USE_STD_CHAR_CONV_INT)
    return fromChars<int32_t>(str, defaultI);
#else
    return s2n<int>(str, defaultI, stox<int>());
#endif
}

int64_t str2int64(const std::string &str, const int64_t defaultI) {
#if defined(USE_STD_CHAR_CONV_INT)
    return fromChars<int64_t>(str, defaultI);
#else
    return s2n<long long>(str, defaultI, stox<long long>());
#endif
}

float str2float32(const std::string &str, const float defaultF) {
#if defined(USE_STD_CHAR_CONV_FLOAT)
    return fromChars<float>(str, defaultF);
#else
    return s2n<float>(str, defaultF, stox<float>());
#endif
}

double str2float64(const std::string &str, const double defaultF) {
#if defined(USE_STD_CHAR_CONV_FLOAT)
    return fromChars<double>(str, defaultF);
#else
    return s2n<double>(str, defaultF, stox<double>());
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
    _sqlite = std::make_unique<Store::SQLite::SQLiteStore>();
#endif

#if defined(USE_KV)
    _kv = std::make_unique<Store::KV::KVStore>(*_root);
#endif

#if defined(USE_CURL)
    _http_client = std::make_unique<Net::HttpClient>();
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

// Device.Device

DynXXDeviceTypeX dynxxDeviceType() {
    return static_cast<DynXXDeviceTypeX>(Device::deviceType());
}

std::string dynxxDeviceName() {
    return Device::deviceName();
}

std::string dynxxDeviceManufacturer() {
    return Device::deviceManufacturer();
}

std::string dynxxDeviceModel() {
    return Device::deviceModel();
}

std::string dynxxDeviceOsVersion() {
    return Device::osVersion();
}

DynXXDeviceCpuArchX dynxxDeviceCpuArch() {
    return static_cast<DynXXDeviceCpuArchX>(Device::cpuArch());
}

// Log

void dynxxLogSetLevel(DynXXLogLevelX level) {
    Log::setLevel(static_cast<int>(level));
}

void dynxxLogSetCallback(const std::function<void(int level, const char *content)> &callback) {
    Log::setCallback(callback);
}

void dynxxLogPrint(DynXXLogLevelX level, std::string_view content) {
    Log::print(static_cast<int>(level), content);
}

// Coding

std::string dynxxCodingCaseUpper(std::string_view str) {
    return Coding::Case::upper(str);
}

std::string dynxxCodingCaseLower(std::string_view str) {
    return Coding::Case::lower(str);
}

std::string dynxxCodingHexBytes2str(BytesView bytes) {
    return Coding::Hex::bytes2str(bytes);
}

Bytes dynxxCodingHexStr2bytes(const std::string &str) {
    return Coding::Hex::str2bytes(str);
}

std::string dynxxCodingBytes2str(BytesView bytes) {
    return Coding::bytes2str(bytes);
}

Bytes dynxxCodingStr2bytes(std::string_view str) {
    return Coding::str2bytes(str);
}

std::string dynxxCodingStrTrim(std::string_view str) {
    return Coding::strTrim(str);
}

// Crypto

Bytes dynxxCryptoRand(size_t len) {
    return Crypto::rand(len);
}

Bytes dynxxCryptoAesEncrypt(BytesView in, BytesView key) {
    return Crypto::AES::encrypt(in, key);
}

Bytes dynxxCryptoAesDecrypt(BytesView in, BytesView key) {
    return Crypto::AES::decrypt(in, key);
}

Bytes dynxxCryptoAesGcmEncrypt(BytesView in, BytesView key, BytesView initVector, size_t tagBits,
                                BytesView aad) {
    return Crypto::AES::gcmEncrypt(in, key, initVector, aad, tagBits);
}

Bytes dynxxCryptoAesGcmDecrypt(BytesView in, BytesView key, BytesView initVector, size_t tagBits,
                                BytesView aad) {
    return Crypto::AES::gcmDecrypt(in, key, initVector, aad, tagBits);
}

std::string dynxxCryptoRsaGenKey(std::string_view base64, bool isPublic) {
    return Crypto::RSA::genKey(base64, isPublic);
}

Bytes dynxxCryptoRsaEncrypt(BytesView in, BytesView key, DynXXCryptoRSAPaddingX padding) {
    const auto enc = Crypto::RSA::Encrypt(key, static_cast<int>(padding));
    return enc.process(in).value_or(Bytes{});
}

Bytes dynxxCryptoRsaDecrypt(BytesView in, BytesView key, DynXXCryptoRSAPaddingX padding) {
    const auto dec = Crypto::RSA::Decrypt(key, static_cast<int>(padding));
    return dec.process(in).value_or(Bytes{});
}

Bytes dynxxCryptoHashMd5(BytesView in) {
    return Crypto::Hash::md5(in);
}

Bytes dynxxCryptoHashSha1(BytesView in) {
    return Crypto::Hash::sha1(in);
}

Bytes dynxxCryptoHashSha256(BytesView in) {
    return Crypto::Hash::sha256(in);
}

Bytes dynxxCryptoBase64Encode(BytesView in, bool noNewLines) {
    return Crypto::Base64::encode(in, noNewLines);
}

Bytes dynxxCryptoBase64Decode(BytesView in, bool noNewLines) {
    return Crypto::Base64::decode(in, noNewLines);
}

// Net.Http

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
    std::vector<Net::HttpFormField> vFormFields;
    auto fieldNameCount = formFieldNameV.size();
    if (fieldNameCount > 0) {
        vFormFields.reserve(fieldNameCount);
    }
    for (decltype(fieldNameCount) i(0); i < fieldNameCount && i < formFieldMimeV.size() && i < formFieldDataV.size(); i
         ++) {
        vFormFields.emplace_back(Net::HttpFormField{
            formFieldNameV[i],
            formFieldMimeV[i],
            formFieldDataV[i]
        });
    }

    return _http_client->request(url, static_cast<int>(method), headerV, params, rawBody, vFormFields, cFILE, fileSize,
                                 timeout);
#else
    return Net::WasmHttpClient::request(url, static_cast<int>(method), headerV, params, rawBody, timeout);
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

    auto json = dynxxJsonNodeToStr(cj);
    cJSON_Delete(cj);
    return json;
}

DynXXHttpResponse dynxxNetHttpRequest(std::string_view url,
                                        DynXXHttpMethodX method,
                                        const DictAny &params,
                                        const BytesView rawBody,
                                        const Dict &headers,
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
bool dynxxNetHttpDownload(std::string_view url, const std::string_view filePath, size_t timeout) {
    if (!_http_client || url.empty() || filePath.empty()) {
        return false;
    }
    return _http_client->download(url, filePath, timeout);
}
#endif

// Store.SQLite

#if defined(USE_DB)

DynXXDBConnHandle dynxxStoreSqliteOpen(const std::string &_id) {
    if (!_sqlite || !_root || _id.empty()) {
        return nullptr;
    }
    std::string dbPath = *_root + "/" + _id + ".db";
    if (const auto ptr = _sqlite->open(dbPath).lock()) [[likely]] {
        return ptr.get();
    }
    return nullptr;
}

bool dynxxStoreSqliteExecute(const DynXXDBConnHandle conn, std::string_view sql) {
    if (conn == nullptr || sql.empty()) {
        return false;
    }
    const auto xconn = static_cast<Store::SQLite::Connection *>(conn);
    return xconn->execute(sql);
}

DynXXDBQueryResultHandle dynxxStoreSqliteQueryDo(const DynXXDBConnHandle conn, std::string_view sql) {
    if (conn == nullptr || sql.empty()) {
        return nullptr;
    }
    const auto xconn = static_cast<Store::SQLite::Connection *>(conn);
    auto ptrQr = xconn->query(sql);
    return ptrQr ? ptrQr.release() : nullptr; //trans ownership to C API, not release the memory.
}

bool dynxxStoreSqliteQueryReadRow(const DynXXDBQueryResultHandle query_result) {
    if (query_result == nullptr) {
        return false;
    }
    const auto xqr = static_cast<Store::SQLite::Connection::QueryResult *>(query_result);
    return xqr->readRow();
}

std::optional<std::string> dynxxStoreSqliteQueryReadColumnText(const DynXXDBQueryResultHandle query_result, std::string_view column) {
    if (query_result == nullptr || column.empty()) {
        return std::nullopt;
    }
    const auto xqr = static_cast<Store::SQLite::Connection::QueryResult *>(query_result);
    auto v = (*xqr)[column];
    if (!v.has_value()) [[unlikely]] {
        return std::nullopt;
    }
    return {*std::get_if<std::string>(&v.value())};
}

std::optional<int64_t> dynxxStoreSqliteQueryReadColumnInteger(const DynXXDBQueryResultHandle query_result, std::string_view column) {
    if (query_result == nullptr || column.empty()) {
        return std::nullopt;
    }
    const auto xqr = static_cast<Store::SQLite::Connection::QueryResult *>(query_result);
    const auto v = (*xqr)[column];
    if (!v.has_value()) [[unlikely]] {
        return std::nullopt;
    }
    return {*std::get_if<int64_t>(&v.value())};
}

std::optional<double> dynxxStoreSqliteQueryReadColumnFloat(const DynXXDBQueryResultHandle query_result, std::string_view column) {
    if (query_result == nullptr || column.empty()) {
        return std::nullopt;
    }
    const auto xqr = static_cast<Store::SQLite::Connection::QueryResult *>(query_result);
    const auto v = (*xqr)[column];
    if (!v.has_value()) [[unlikely]] {
        return std::nullopt;
    }
    return {*std::get_if<double>(&v.value())};
}

void dynxxStoreSqliteQueryDrop(const DynXXDBQueryResultHandle query_result) {
    if (query_result == nullptr) {
        return;
    }
    const auto xqr = static_cast<Store::SQLite::Connection::QueryResult *>(query_result);
    delete xqr;
}

void dynxxStoreSqliteClose(const DynXXDBConnHandle conn) {
    if (conn == nullptr || _sqlite == nullptr)
        return;
    const auto xconn = static_cast<Store::SQLite::Connection *>(conn);
    _sqlite->close(xconn->id());
}

#endif

// Store.KV

#if defined(USE_KV)

DynXXKVConnHandle dynxxStoreKvOpen(const std::string &_id) {
    if (!_kv || _id.empty()) {
        return nullptr;
    }
    if (const auto ptr = _kv->open(_id).lock()) [[likely]] {
        return ptr.get();
    }
    return nullptr;
}

std::optional<std::string> dynxxStoreKvReadString(const DynXXKVConnHandle conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return std::nullopt;
    }
    const auto xconn = static_cast<Store::KV::Connection *>(conn);
    return xconn->readString(k);
}

bool dynxxStoreKvWriteString(const DynXXKVConnHandle conn, std::string_view k, const std::string &v) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

std::optional<int64_t> dynxxStoreKvReadInteger(const DynXXKVConnHandle conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return std::nullopt;
    }
    const auto xconn = static_cast<Store::KV::Connection *>(conn);
    return xconn->readInteger(k);
}

bool dynxxStoreKvWriteInteger(const DynXXKVConnHandle conn, std::string_view k, int64_t v) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

std::optional<double> dynxxStoreKvReadFloat(const DynXXKVConnHandle conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return std::nullopt;
    }
    const auto xconn = static_cast<Store::KV::Connection *>(conn);
    return xconn->readFloat(k);
}

bool dynxxStoreKvWriteFloat(const DynXXKVConnHandle conn, std::string_view k, double v) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

std::vector<std::string> dynxxStoreKvAllKeys(const DynXXKVConnHandle conn) {
    if (conn == nullptr) {
        return {};
    }
    const auto xconn = static_cast<Store::KV::Connection *>(conn);
    return xconn->allKeys();
}

bool dynxxStoreKvContains(const DynXXKVConnHandle conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<Store::KV::Connection *>(conn);
    return xconn->contains(k);
}

bool dynxxStoreKvRemove(const DynXXKVConnHandle conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<Store::KV::Connection *>(conn);
    return xconn->remove(k);
}

void dynxxStoreKvClear(const DynXXKVConnHandle conn) {
    if (conn == nullptr) {
        return;
    }
    const auto xconn = static_cast<Store::KV::Connection *>(conn);
    xconn->clear();
}

void dynxxStoreKvClose(const DynXXKVConnHandle conn) {
    if (conn == nullptr || _kv == nullptr)
        return;
    const auto xconn = static_cast<Store::KV::Connection *>(conn);
    _kv->close(xconn->id());
}

#endif

// Json

DynXXJsonNodeTypeX dynxxJsonNodeReadType(const DynXXJsonNodeHandle node) {
    return Json::nodeReadType(node);
}

std::optional<std::string> dynxxJsonNodeReadName(const DynXXJsonNodeHandle node) {
    return Json::nodeReadName(node);
}

std::optional<std::string> dynxxJsonNodeToStr(const DynXXJsonNodeHandle node) {
    return Json::nodeToStr(node);
}

std::optional<std::string> dynxxJsonFromDictAny(const DictAny &dict) {
    return Json::jsonFromDictAny(dict);
}

std::optional<DictAny> dynxxJsonToDictAny(const std::string &json) {
    return Json::jsonToDictAny(json);
}

DynXXJsonDecoderHandle dynxxJsonDecoderInit(const std::string_view json) {
    auto decoder = new(std::nothrow) Json::Decoder(json);
    if (decoder == nullptr) {
        dynxxLogPrint(DynXXLogLevelX::Error, "new JsonDecoder failed");
    }
    return decoder;
}

DynXXJsonNodeHandle dynxxJsonDecoderReadNode(const DynXXJsonDecoderHandle decoder, std::string_view k, const DynXXJsonNodeHandle node) {
    if (decoder == nullptr || k.empty()) {
        return nullptr;
    }
    const auto xdecoder = static_cast<Json::Decoder *>(decoder);
    return xdecoder->readNode(node, k);
}

std::optional<std::string> dynxxJsonDecoderReadString(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node) {
    if (decoder == nullptr) {
        return std::nullopt;
    }
    const auto xdecoder = static_cast<Json::Decoder *>(decoder);
    return xdecoder->readString(node);
}

std::optional<double> dynxxJsonDecoderReadNumber(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node) {
    if (decoder == nullptr) {
        return std::nullopt;
    }
    const auto xdecoder = static_cast<Json::Decoder *>(decoder);
    return xdecoder->readNumber(node);
}

DynXXJsonNodeHandle dynxxJsonDecoderReadChild(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node) {
    if (decoder == nullptr) {
        return nullptr;
    }
    const auto xdecoder = static_cast<Json::Decoder *>(decoder);
    return xdecoder->readChild(node);
}

size_t dynxxJsonDecoderReadChildrenCount(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node) {
    if (decoder == nullptr) {
        return 0;
    }
    const auto xdecoder = static_cast<Json::Decoder *>(decoder);
    return xdecoder->readChildrenCount(node);
}

void dynxxJsonDecoderReadChildren(const DynXXJsonDecoderHandle decoder, std::function<void(size_t idx, const DynXXJsonNodeHandle childNode, const DynXXJsonNodeTypeX childType, const std::optional<std::string> childName)> &&callback,
                                   const DynXXJsonNodeHandle node) {
    if (decoder == nullptr) {
        return;
    }
    const auto xdecoder = static_cast<Json::Decoder *>(decoder);
    xdecoder->readChildren(node, std::move(callback));
}

DynXXJsonNodeHandle dynxxJsonDecoderReadNext(const DynXXJsonDecoderHandle decoder, const DynXXJsonNodeHandle node) {
    if (decoder == nullptr) {
        return nullptr;
    }
    const auto xdecoder = static_cast<Json::Decoder *>(decoder);
    return xdecoder->readNext(node);
}

void dynxxJsonDecoderRelease(const DynXXJsonDecoderHandle decoder) {
    if (decoder == nullptr) {
        return;
    }
    const auto xdecoder = static_cast<Json::Decoder *>(decoder);
    delete xdecoder;
}

// Zip

DynXXZipHandle dynxxZZipInit(const DynXXZipCompressModeX mode, size_t bufferSize, const DynXXZFormatX format) {
    DynXXZipHandle zip = nullptr;
    try {
        zip = new Z::Zip(static_cast<int>(mode), bufferSize, static_cast<int>(format));
    } catch (const std::invalid_argument &e) {
        dynxxLogPrintF(DynXXLogLevelX::Error, "dynxxZZipInit invalid_argument: {}", e.what());
    } catch (const std::runtime_error &e) {
        dynxxLogPrintF(DynXXLogLevelX::Error, "dynxxZZipInit runtime_error: {}", e.what());
    }
    return zip;
}

size_t dynxxZZipInput(const DynXXZipHandle zip, const Bytes &inBytes, bool inFinish) {
    if (zip == nullptr) {
        return 0;
    }
    const auto xzip = static_cast<Z::Zip *>(zip);
    return xzip->input(inBytes, inFinish);
}

Bytes dynxxZZipProcessDo(const DynXXZipHandle zip) {
    if (zip == nullptr) {
        return {};
    }
    const auto xzip = static_cast<Z::Zip *>(zip);
    return xzip->processDo();
}

bool dynxxZZipProcessFinished(const DynXXZipHandle zip) {
    if (zip == nullptr) {
        return false;
    }
    const auto xzip = static_cast<Z::Zip *>(zip);
    return xzip->processFinished();
}

void dynxxZZipRelease(const DynXXZipHandle zip) {
    if (zip == nullptr) {
        return;
    }
    const auto xzip = static_cast<Z::Zip *>(zip);
    delete xzip;
}

DynXXUnZipHandle dynxxZUnzipInit(size_t bufferSize, const DynXXZFormatX format) {
    DynXXUnZipHandle unzip = nullptr;
    try {
        unzip = new Z::UnZip(bufferSize, static_cast<int>(format));
    } catch (const std::invalid_argument &e) {
        dynxxLogPrintF(DynXXLogLevelX::Error, "dynxxZUnzipInit invalid_argument: {}", e.what());
    } catch (const std::runtime_error &e) {
        dynxxLogPrintF(DynXXLogLevelX::Error, "dynxxZUnzipInit runtime_error: {}", e.what());
    }
    return unzip;
}

size_t dynxxZUnzipInput(const DynXXUnZipHandle unzip, const Bytes &inBytes, bool inFinish) {
    if (unzip == nullptr) {
        return 0;
    }
    const auto xunzip = static_cast<Z::UnZip *>(unzip);
    return xunzip->input(inBytes, inFinish);
}

Bytes dynxxZUnzipProcessDo(const DynXXUnZipHandle unzip) {
    if (unzip == nullptr) {
        return {};
    }
    const auto xunzip = static_cast<Z::UnZip *>(unzip);
    return xunzip->processDo();
}

bool dynxxZUnzipProcessFinished(const DynXXUnZipHandle unzip) {
    if (unzip == nullptr) {
        return false;
    }
    const auto xunzip = static_cast<Z::UnZip *>(unzip);
    return xunzip->processFinished();
}

void dynxxZUnzipRelease(const DynXXUnZipHandle unzip) {
    if (unzip == nullptr) {
        return;
    }
    const auto xunzip = static_cast<Z::UnZip *>(unzip);
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
    return Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), cFILEIn, cFILEOut);
}

bool dynxxZCFileUnzip(std::FILE *cFILEIn, std::FILE *cFILEOut, size_t bufferSize, const DynXXZFormatX format) {
    if (bufferSize == 0) {
        return false;
    }
    if (cFILEIn == nullptr || cFILEOut == nullptr) {
        return false;
    }
    return Z::unzip(bufferSize, static_cast<int>(format), cFILEIn, cFILEOut);
}

bool dynxxZCxxStreamZip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut, const DynXXZipCompressModeX mode,
                         size_t bufferSize, const DynXXZFormatX format) {
    if (bufferSize == 0) {
        return false;
    }
    if (cxxStreamIn == nullptr || cxxStreamOut == nullptr) {
        return false;
    }
    return Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), cxxStreamIn,
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
    return Z::unzip(bufferSize, static_cast<int>(format), cxxStreamIn, cxxStreamOut);
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
    return Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), inBytes);
}

Bytes dynxxZBytesUnzip(const Bytes &inBytes, size_t bufferSize, const DynXXZFormatX format) {
    if (bufferSize == 0) {
        return {};
    }
    if (inBytes.empty()) {
        return {};
    }
    return Z::unzip(bufferSize, static_cast<int>(format), inBytes);
}
