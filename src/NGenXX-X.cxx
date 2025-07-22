#if defined(__cplusplus)

#include <memory>

#include <NGenXX.hxx>
#include "core/net/HttpClient.hxx"
#include "core/store/SQLite.hxx"
#include "core/store/KV.hxx"
#include "core/json/JsonCodec.hxx"
#include "core/zip/Zip.hxx"
#include "core/coding/Coding.hxx"
#include "core/crypto/Crypto.hxx"
#include "core/device/Device.hxx"
#include "core/log/Log.hxx"

#include <NGenXXMacro.hxx>
#if defined(USE_LUA)
#include "bridge/LuaBridge.hxx"
#endif
#if defined(USE_QJS)
#include "bridge/JSBridge.hxx"
#endif
#if defined(USE_STD_CHAR_CONV_INT)
#include <charconv>
#endif
#endif

namespace {
    auto constexpr VERSION = "1.0.0";

    std::unique_ptr<NGenXX::Core::Net::HttpClient> _http_client = nullptr;
    std::unique_ptr<NGenXX::Core::Store::SQLite> _sqlite = nullptr;
    std::unique_ptr<NGenXX::Core::Store::KV> _kv = nullptr;
    std::unique_ptr<const std::string> _root = nullptr;

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
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "s2n<i> err: {}", e.what());
            return defaultValue;
        }
    }

    template<FloatT T>
    T s2n(const std::string &str, T defaultValue, T (*f)(const std::string &, size_t *)) {
        try {
            return f(str, nullptr);
        } catch (const std::exception &e) {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "s2n<f> err: {}", e.what());
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

std::string ngenxxGetVersion() {
    return VERSION;
}

std::string ngenxxRootPath() {
    if (!_root) {
        return {};
    }
    return *_root;
}

bool ngenxxInit(const std::string &root) {
    if (_root) {
        return true;
    }
    if (root.empty()) {
        return false;
    }
    _root = std::make_unique<const std::string>(root);
    _sqlite = std::make_unique<NGenXX::Core::Store::SQLite>();
    _kv = std::make_unique<NGenXX::Core::Store::KV>(*_root);
    _http_client = std::make_unique<NGenXX::Core::Net::HttpClient>();
#if defined(USE_LUA)
    ngenxx_lua_init();
#endif
#if defined(USE_QJS)
    ngenxx_js_init();
#endif
    return true;
}

void ngenxxRelease() {
    if (!_root) {
        return;
    }
    _root.reset();
    _http_client.reset();
    _sqlite->closeAll();
    _sqlite.reset();
    _kv->closeAll();
    _kv.reset();
    ngenxxLogSetCallback(nullptr);
#if defined(USE_LUA)
    ngenxx_lua_release();
#endif
#if defined(USE_QJS)
    ngenxx_js_release();
#endif
}

#pragma mark Device.Device

NGenXXDeviceTypeX ngenxxDeviceType() {
    return static_cast<NGenXXDeviceTypeX>(NGenXX::Core::Device::deviceType());
}

std::string ngenxxDeviceName() {
    return NGenXX::Core::Device::deviceName();
}

std::string ngenxxDeviceManufacturer() {
    return NGenXX::Core::Device::deviceManufacturer();
}

std::string ngenxxDeviceModel() {
    return NGenXX::Core::Device::deviceModel();
}

std::string ngenxxDeviceOsVersion() {
    return NGenXX::Core::Device::osVersion();
}

NGenXXDeviceCpuArchX ngenxxDeviceCpuArch() {
    return static_cast<NGenXXDeviceCpuArchX>(NGenXX::Core::Device::cpuArch());
}

#pragma mark Log

void ngenxxLogSetLevel(NGenXXLogLevelX level) {
    NGenXX::Core::Log::setLevel(static_cast<int>(level));
}

void ngenxxLogSetCallback(const std::function<void(int level, const char *content)> &callback) {
    NGenXX::Core::Log::setCallback(callback);
}

void ngenxxLogPrint(NGenXXLogLevelX level, std::string_view content) {
    NGenXX::Core::Log::print(static_cast<int>(level), content);
}

#pragma mark Coding

std::string ngenxxCodingCaseUpper(std::string_view str) {
    return NGenXX::Core::Coding::Case::upper(str);
}

std::string ngenxxCodingCaseLower(std::string_view str) {
    return NGenXX::Core::Coding::Case::lower(str);
}

std::string ngenxxCodingHexBytes2str(BytesView bytes) {
    return NGenXX::Core::Coding::Hex::bytes2str(bytes);
}

Bytes ngenxxCodingHexStr2bytes(const std::string &str) {
    return NGenXX::Core::Coding::Hex::str2bytes(str);
}

std::string ngenxxCodingBytes2str(BytesView bytes) {
    return NGenXX::Core::Coding::bytes2str(bytes);
}

Bytes ngenxxCodingStr2bytes(std::string_view str) {
    return NGenXX::Core::Coding::str2bytes(str);
}

std::string ngenxxCodingStrTrim(std::string_view str) {
    return NGenXX::Core::Coding::strTrim(str);
}

#pragma mark Crypto

bool ngenxxCryptoRand(size_t len, byte *bytes) {
    return NGenXX::Core::Crypto::rand(len, bytes);
}

Bytes ngenxxCryptoAesEncrypt(BytesView in, BytesView key) {
    return NGenXX::Core::Crypto::AES::encrypt(in, key);
}

Bytes ngenxxCryptoAesDecrypt(BytesView in, BytesView key) {
    return NGenXX::Core::Crypto::AES::decrypt(in, key);
}

Bytes ngenxxCryptoAesGcmEncrypt(BytesView in, BytesView key, BytesView initVector, size_t tagBits,
                                BytesView aad) {
    return NGenXX::Core::Crypto::AES::gcmEncrypt(in, key, initVector, aad, tagBits);
}

Bytes ngenxxCryptoAesGcmDecrypt(BytesView in, BytesView key, BytesView initVector, size_t tagBits,
                                BytesView aad) {
    return NGenXX::Core::Crypto::AES::gcmDecrypt(in, key, initVector, aad, tagBits);
}

std::string ngenxxCryptoRsaGenKey(std::string_view base64, bool isPublic) {
    return NGenXX::Core::Crypto::RSA::genKey(base64, isPublic);
}

Bytes ngenxxCryptoRsaEncrypt(BytesView in, BytesView key, NGenXXCryptoRSAPaddingX padding) {
    const auto enc = NGenXX::Core::Crypto::RSA::Encrypt(key, static_cast<int>(padding));
    return enc.process(in).value_or(Bytes{});
}

Bytes ngenxxCryptoRsaDecrypt(BytesView in, BytesView key, NGenXXCryptoRSAPaddingX padding) {
    const auto dec = NGenXX::Core::Crypto::RSA::Decrypt(key, static_cast<int>(padding));
    return dec.process(in).value_or(Bytes{});
}

Bytes ngenxxCryptoHashMd5(BytesView in) {
    return NGenXX::Core::Crypto::Hash::md5(in);
}

Bytes ngenxxCryptoHashSha1(BytesView in) {
    return NGenXX::Core::Crypto::Hash::sha1(in);
}

Bytes ngenxxCryptoHashSha256(BytesView in) {
    return NGenXX::Core::Crypto::Hash::sha256(in);
}

Bytes ngenxxCryptoBase64Encode(BytesView in, bool noNewLines) {
    return NGenXX::Core::Crypto::Base64::encode(in, noNewLines);
}

Bytes ngenxxCryptoBase64Decode(BytesView in, bool noNewLines) {
    return NGenXX::Core::Crypto::Base64::decode(in, noNewLines);
}

#pragma mark Net.Http

NGenXXHttpResponse ngenxxNetHttpRequest(std::string_view url,
                                        NGenXXHttpMethodX method,
                                        std::string_view params,
                                        const Bytes &rawBody,
                                        const std::vector<std::string> &headerV,
                                        const std::vector<std::string> &formFieldNameV,
                                        const std::vector<std::string> &formFieldMimeV,
                                        const std::vector<std::string> &formFieldDataV,
                                        const std::FILE *cFILE, size_t fileSize,
                                        size_t timeout) {
    NGenXXHttpResponse rsp;
    if (!_http_client || url.empty()) {
        return rsp;
    }

    std::vector<NGenXX::Core::Net::HttpFormField> vFormFields;
    auto fieldNameCount = formFieldNameV.size();
    if (fieldNameCount > 0) {
        vFormFields.reserve(fieldNameCount);
    }
    for (decltype(fieldNameCount) i(0); i < fieldNameCount && i < formFieldMimeV.size() && i < formFieldDataV.size(); i
         ++) {
        vFormFields.emplace_back(NGenXX::Core::Net::HttpFormField{
            formFieldNameV[i],
            formFieldMimeV[i],
            formFieldDataV[i],
        });
    }

    return _http_client->request(url, static_cast<int>(method), headerV, params, rawBody, vFormFields, cFILE, fileSize,
                                 timeout);
}

std::optional<std::string> NGenXXHttpResponse::toJson() const {
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

    auto json = ngenxxJsonToStr(cj);
    cJSON_Delete(cj);
    return json;
}

NGenXXHttpResponse ngenxxNetHttpRequest(std::string_view url,
                                        NGenXXHttpMethodX method,
                                        const std::unordered_map<std::string, Any> &params,
                                        const Bytes &rawBody,
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
    return ngenxxNetHttpRequest(url, method, ssParams.str(), rawBody, headerV,
                                formFieldNameV, formFieldMimeV, formFieldDataV,
                                cFILE, fileSize, timeout);
}

bool ngenxxNetHttpDownload(std::string_view url, const std::string &filePath, size_t timeout) {
    if (!_http_client || url.empty() || filePath.empty()) {
        return false;
    }
    return _http_client->download(url, filePath, timeout);
}

#pragma mark Store.SQLite

void *ngenxxStoreSqliteOpen(const std::string &_id) {
    if (!_sqlite || !_root || _id.empty()) {
        return nullptr;
    }
    std::string dbPath = *_root + "/" + _id + ".db";
    if (const auto ptr = _sqlite->connect(dbPath).lock()) [[likely]] {
        return ptr.get();
    }
    return nullptr;
}

bool ngenxxStoreSqliteExecute(void *const conn, std::string_view sql) {
    if (conn == nullptr || sql.empty()) {
        return false;
    }
    const auto xconn = static_cast<NGenXX::Core::Store::SQLite::Connection *>(conn);
    return xconn->execute(sql);
}

void *ngenxxStoreSqliteQueryDo(void *const conn, std::string_view sql) {
    if (conn == nullptr || sql.empty()) {
        return nullptr;
    }
    const auto xconn = static_cast<NGenXX::Core::Store::SQLite::Connection *>(conn);
    auto ptrQr = xconn->query(sql);
    return ptrQr ? ptrQr.release() : nullptr; //trans ownership to C API, not release the memory.
}

bool ngenxxStoreSqliteQueryReadRow(void *const query_result) {
    if (query_result == nullptr) {
        return false;
    }
    const auto xqr = static_cast<NGenXX::Core::Store::SQLite::Connection::QueryResult *>(query_result);
    return xqr->readRow();
}

std::optional<std::string> ngenxxStoreSqliteQueryReadColumnText(void *const query_result, std::string_view column) {
    if (query_result == nullptr || column.empty()) {
        return std::nullopt;
    }
    const auto xqr = static_cast<NGenXX::Core::Store::SQLite::Connection::QueryResult *>(query_result);
    auto v = (*xqr)[column];
    if (!v.has_value()) [[unlikely]] {
        return std::nullopt;
    }
    return std::make_optional(*std::get_if<std::string>(&v.value()));
}

std::optional<int64_t> ngenxxStoreSqliteQueryReadColumnInteger(void *const query_result, std::string_view column) {
    if (query_result == nullptr || column.empty()) {
        return std::nullopt;
    }
    const auto xqr = static_cast<NGenXX::Core::Store::SQLite::Connection::QueryResult *>(query_result);
    const auto v = (*xqr)[column];
    if (!v.has_value()) [[unlikely]] {
        return std::nullopt;
    }
    return std::make_optional(*std::get_if<int64_t>(&v.value()));
}

std::optional<double> ngenxxStoreSqliteQueryReadColumnFloat(void *const query_result, std::string_view column) {
    if (query_result == nullptr || column.empty()) {
        return std::nullopt;
    }
    const auto xqr = static_cast<NGenXX::Core::Store::SQLite::Connection::QueryResult *>(query_result);
    const auto v = (*xqr)[column];
    if (!v.has_value()) [[unlikely]] {
        return std::nullopt;
    }
    return std::make_optional(*std::get_if<double>(&v.value()));
}

void ngenxxStoreSqliteQueryDrop(void *const query_result) {
    if (query_result == nullptr) {
        return;
    }
    const auto xqr = static_cast<NGenXX::Core::Store::SQLite::Connection::QueryResult *>(query_result);
    delete xqr;
}

void ngenxxStoreSqliteClose(void *const conn) {
    /*if (conn == nullptr)
        return;
    const auto xconn = static_cast<NGenXX::Core::Store::SQLite::Connection *>(conn);
    delete xconn;*/
}

#pragma mark Store.KV

void *ngenxxStoreKvOpen(const std::string &_id) {
    if (!_kv || _id.empty()) {
        return nullptr;
    }
    if (const auto ptr = _kv->open(_id).lock()) [[likely]] {
        return ptr.get();
    }
    return nullptr;
}

std::optional<std::string> ngenxxStoreKvReadString(void *const conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return std::nullopt;
    }
    const auto xconn = static_cast<NGenXX::Core::Store::KV::Connection *>(conn);
    return xconn->readString(k);
}

bool ngenxxStoreKvWriteString(void *const conn, std::string_view k, const std::string &v) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<NGenXX::Core::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

std::optional<int64_t> ngenxxStoreKvReadInteger(void *const conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return std::nullopt;
    }
    const auto xconn = static_cast<NGenXX::Core::Store::KV::Connection *>(conn);
    return xconn->readInteger(k);
}

bool ngenxxStoreKvWriteInteger(void *const conn, std::string_view k, int64_t v) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<NGenXX::Core::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

std::optional<double> ngenxxStoreKvReadFloat(void *const conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return std::nullopt;
    }
    const auto xconn = static_cast<NGenXX::Core::Store::KV::Connection *>(conn);
    return xconn->readFloat(k);
}

bool ngenxxStoreKvWriteFloat(void *const conn, std::string_view k, double v) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<NGenXX::Core::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

std::vector<std::string> ngenxxStoreKvAllKeys(void *const conn) {
    if (conn == nullptr) {
        return {};
    }
    const auto xconn = static_cast<NGenXX::Core::Store::KV::Connection *>(conn);
    return xconn->allKeys();
}

bool ngenxxStoreKvContains(void *const conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<NGenXX::Core::Store::KV::Connection *>(conn);
    return xconn->contains(k);
}

bool ngenxxStoreKvRemove(void *const conn, std::string_view k) {
    if (conn == nullptr || k.empty()) {
        return false;
    }
    const auto xconn = static_cast<NGenXX::Core::Store::KV::Connection *>(conn);
    return xconn->remove(k);
}

void ngenxxStoreKvClear(void *const conn) {
    if (conn == nullptr) {
        return;
    }
    const auto xconn = static_cast<NGenXX::Core::Store::KV::Connection *>(conn);
    xconn->clear();
}

void ngenxxStoreKvClose(void *const conn) {
    /*if (conn == nullptr)
        return;
    const auto xconn = static_cast<NGenXX::Core::Store::KV::Connection *>(conn);
    delete xconn;*/
}

#pragma mark Json

NGenXXJsonNodeTypeX ngenxxJsonReadType(void *const cjson) {
    return NGenXX::Core::Json::cJSONReadType(cjson);
}

std::optional<std::string> ngenxxJsonToStr(void *const cjson) {
    return NGenXX::Core::Json::cJSONToStr(cjson);
}

std::optional<std::string> ngenxxJsonFromDictAny(const DictAny &dict) {
    return NGenXX::Core::Json::jsonFromDictAny(dict);
}

std::optional<DictAny> ngenxxJsonToDictAny(const std::string &json) {
    return NGenXX::Core::Json::jsonToDictAny(json);
}

void *ngenxxJsonDecoderInit(const std::string &json) {
    auto decoder = new(std::nothrow) NGenXX::Core::Json::Decoder(json);
    if (decoder == nullptr) {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "new JsonDecoder failed");
    }
    return decoder;
}

void *ngenxxJsonDecoderReadNode(void *const decoder, std::string_view k, void *const node) {
    if (decoder == nullptr || k.empty()) {
        return nullptr;
    }
    const auto xdecoder = static_cast<NGenXX::Core::Json::Decoder *>(decoder);
    return xdecoder->readNode(node, k);
}

std::optional<std::string> ngenxxJsonDecoderReadString(void *const decoder, void *const node) {
    if (decoder == nullptr) {
        return std::nullopt;
    }
    const auto xdecoder = static_cast<NGenXX::Core::Json::Decoder *>(decoder);
    return xdecoder->readString(node);
}

std::optional<double> ngenxxJsonDecoderReadNumber(void *const decoder, void *const node) {
    if (decoder == nullptr) {
        return std::nullopt;
    }
    const auto xdecoder = static_cast<NGenXX::Core::Json::Decoder *>(decoder);
    return xdecoder->readNumber(node);
}

void *ngenxxJsonDecoderReadChild(void *const decoder, void *const node) {
    if (decoder == nullptr) {
        return nullptr;
    }
    const auto xdecoder = static_cast<NGenXX::Core::Json::Decoder *>(decoder);
    return xdecoder->readChild(node);
}

void ngenxxJsonDecoderReadChildren(void *const decoder, std::function<void(size_t idx, const void *child)> &&callback,
                                   void *const node) {
    if (decoder == nullptr) {
        return;
    }
    const auto xdecoder = static_cast<NGenXX::Core::Json::Decoder *>(decoder);
    xdecoder->readChildren(node, std::move(callback));
}

void *ngenxxJsonDecoderReadNext(void *const decoder, void *const node) {
    if (decoder == nullptr) {
        return nullptr;
    }
    const auto xdecoder = static_cast<NGenXX::Core::Json::Decoder *>(decoder);
    return xdecoder->readNext(node);
}

void ngenxxJsonDecoderRelease(void *const decoder) {
    if (decoder == nullptr) {
        return;
    }
    const auto xdecoder = static_cast<NGenXX::Core::Json::Decoder *>(decoder);
    delete xdecoder;
}

#pragma mark Zip

void *ngenxxZZipInit(const NGenXXZipCompressModeX mode, size_t bufferSize, const NGenXXZFormatX format) {
    void *zip = nullptr;
    try {
        zip = new NGenXX::Core::Z::Zip(static_cast<int>(mode), bufferSize, static_cast<int>(format));
    } catch (const std::exception &e) {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "ngenxxZZipInit failed: {}", e.what());
    }
    return zip;
}

size_t ngenxxZZipInput(void *const zip, const Bytes &inBytes, bool inFinish) {
    if (zip == nullptr) {
        return 0;
    }
    const auto xzip = static_cast<NGenXX::Core::Z::Zip *>(zip);
    return xzip->input(inBytes, inFinish);
}

Bytes ngenxxZZipProcessDo(void *const zip) {
    if (zip == nullptr) {
        return {};
    }
    const auto xzip = static_cast<NGenXX::Core::Z::Zip *>(zip);
    return xzip->processDo();
}

bool ngenxxZZipProcessFinished(void *const zip) {
    if (zip == nullptr) {
        return false;
    }
    const auto xzip = static_cast<NGenXX::Core::Z::Zip *>(zip);
    return xzip->processFinished();
}

void ngenxxZZipRelease(void *const zip) {
    if (zip == nullptr) {
        return;
    }
    const auto xzip = static_cast<NGenXX::Core::Z::Zip *>(zip);
    delete xzip;
}

void *ngenxxZUnzipInit(size_t bufferSize, const NGenXXZFormatX format) {
    void *unzip = nullptr;
    try {
        unzip = new NGenXX::Core::Z::UnZip(bufferSize, static_cast<int>(format));
    } catch (const std::exception &e) {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "ngenxxZUnzipInit failed: {}", e.what());
    }
    return unzip;
}

size_t ngenxxZUnzipInput(void *const unzip, const Bytes &inBytes, bool inFinish) {
    if (unzip == nullptr) {
        return 0;
    }
    const auto xunzip = static_cast<NGenXX::Core::Z::UnZip *>(unzip);
    return xunzip->input(inBytes, inFinish);
}

Bytes ngenxxZUnzipProcessDo(void *const unzip) {
    if (unzip == nullptr) {
        return {};
    }
    const auto xunzip = static_cast<NGenXX::Core::Z::UnZip *>(unzip);
    return xunzip->processDo();
}

bool ngenxxZUnzipProcessFinished(void *const unzip) {
    if (unzip == nullptr) {
        return false;
    }
    const auto xunzip = static_cast<NGenXX::Core::Z::UnZip *>(unzip);
    return xunzip->processFinished();
}

void ngenxxZUnzipRelease(void *const unzip) {
    if (unzip == nullptr) {
        return;
    }
    const auto xunzip = static_cast<NGenXX::Core::Z::UnZip *>(unzip);
    delete xunzip;
}

bool ngenxxZCFileZip(std::FILE *cFILEIn, std::FILE *cFILEOut, const NGenXXZipCompressModeX mode, size_t bufferSize,
                     const NGenXXZFormatX format) {
    if (bufferSize == 0) {
        return false;
    }
    if (cFILEIn == nullptr || cFILEOut == nullptr) {
        return false;
    }
    return NGenXX::Core::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), cFILEIn, cFILEOut);
}

bool ngenxxZCFileUnzip(std::FILE *cFILEIn, std::FILE *cFILEOut, size_t bufferSize, const NGenXXZFormatX format) {
    if (bufferSize == 0) {
        return false;
    }
    if (cFILEIn == nullptr || cFILEOut == nullptr) {
        return false;
    }
    return NGenXX::Core::Z::unzip(bufferSize, static_cast<int>(format), cFILEIn, cFILEOut);
}

bool ngenxxZCxxStreamZip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut, const NGenXXZipCompressModeX mode,
                         size_t bufferSize, const NGenXXZFormatX format) {
    if (bufferSize == 0) {
        return false;
    }
    if (cxxStreamIn == nullptr || cxxStreamOut == nullptr) {
        return false;
    }
    return NGenXX::Core::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), cxxStreamIn,
                                cxxStreamOut);
}

bool ngenxxZCxxStreamUnzip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut, size_t bufferSize,
                           const NGenXXZFormatX format) {
    if (bufferSize == 0) {
        return false;
    }
    if (cxxStreamIn == nullptr || cxxStreamOut == nullptr) {
        return false;
    }
    return NGenXX::Core::Z::unzip(bufferSize, static_cast<int>(format), cxxStreamIn, cxxStreamOut);
}

Bytes ngenxxZBytesZip(const Bytes &inBytes, const NGenXXZipCompressModeX mode, size_t bufferSize,
                      const NGenXXZFormatX format) {
    if (bufferSize == 0) {
        return {};
    }
    if (inBytes.empty()) {
        return {};
    }
    return NGenXX::Core::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), inBytes);
}

Bytes ngenxxZBytesUnzip(const Bytes &inBytes, size_t bufferSize, const NGenXXZFormatX format) {
    if (bufferSize == 0) {
        return {};
    }
    if (inBytes.empty()) {
        return {};
    }
    return NGenXX::Core::Z::unzip(bufferSize, static_cast<int>(format), inBytes);
}
