#if defined(__cplusplus)

#include <vector>
#include <string>
#include <sstream>
#include <memory>

#include <NGenXX.hxx>
#include "net/HttpClient.hxx"
#include "store/SQLite.hxx"
#include "store/KV.hxx"
#include "json/JsonCodec.hxx"
#include "zip/Zip.hxx"
#include "coding/Coding.hxx"
#include "crypto/Crypto.hxx"
#include "device/DeviceInfo.hxx"
#include "log/Log.hxx"
#if defined(USE_LUA)
#include "NGenXX-Lua.hxx"
#endif
#if defined(USE_QJS)
#include "NGenXX-Js.hxx"
#endif
#if defined(USE_STD_FROM_CHARS)
#include <charconv>
#endif
#endif

namespace
{
    auto constexpr VERSION = "1.0.0";

    std::unique_ptr<NGenXX::Net::HttpClient> _http_client = nullptr;
    std::unique_ptr<NGenXX::Store::SQLite> _sqlite = nullptr;
    std::unique_ptr<NGenXX::Store::KV> _kv = nullptr;
    std::unique_ptr<const std::string> _root = nullptr;   

#if defined(USE_STD_FROM_CHARS)
    template <NumberT T>
    T from_chars(const std::string &str, const T defaultV)
    {
        if (str.empty())
        {
            return defaultV;
        }
        T v;
        auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), v);
        if (ec == std::errc() && ptr == str.data() + str.size()) [[likely]]
        {
            return v;
        }
        return defaultV;
    }
#endif

    template<IntegerT T>
    T stox(const std::string& str, T defaultValue, T (*f)(const std::string&, size_t*, int)) 
    {
        try 
        {
            return f(str, nullptr, 10);
        } 
        catch (const std::exception& e)
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "stox failed: {}", e.what());
            return defaultValue;
        }
    }

    template<FloatT T>
    T stox(const std::string& str, T defaultValue, T (*f)(const std::string&, size_t*))
    {
        try
        {
            return f(str, nullptr);
        }
        catch (const std::exception& e)
        {
            ngenxxLogPrintF(NGenXXLogLevelX::Error, "stox failed: {}", e.what());
            return defaultValue;
        }
    }
}

int32_t str2int32(const std::string &str, const int32_t defaultI)
{
#if defined(USE_STD_FROM_CHARS)
    return from_chars<int32_t>(str, defaultI);
#else
    return stox<int32_t>(str, defaultI, std::stoi);
#endif
}

int64_t str2int64(const std::string &str, const int64_t defaultI)
{
#if defined(USE_STD_FROM_CHARS)
    return from_chars<int64_t>(str, defaultI);
#else
    return stox<int64_t>(str, defaultI, std::stoll);
#endif
}

float str2float32(const std::string &str, const float defaultF)
{
#if defined(USE_STD_FROM_CHARS_FLOAT)
    return from_chars<float>(str, defaultF);
#else
    return stox<float>(str, defaultF, std::stof);
#endif
}

double str2float64(const std::string &str, const double defaultF)
{
#if defined(USE_STD_FROM_CHARS_FLOAT)
    return from_chars<double>(str, defaultF);
#else
    return stox<double>(str, defaultF, std::stod);
#endif
}

std::string ngenxxGetVersion()
{
    return VERSION;
}

std::string ngenxxRootPath()
{
    if (!_root)
    {
        return {};
    }
    return *_root;
}

bool ngenxxInit(const std::string &root)
{
    if (_root)
    {
        return true;
    }
    if (root.empty())
    {
        return false;
    }
    _root = std::make_unique<const std::string>(root);
    _sqlite = std::make_unique<NGenXX::Store::SQLite>();
    _kv = std::make_unique<NGenXX::Store::KV>(*_root);
    _http_client = std::make_unique<NGenXX::Net::HttpClient>();
#if defined(USE_LUA)
    _ngenxx_lua_init();
#endif
#if defined(USE_QJS)
    _ngenxx_js_init();
#endif
    return true;
}

void ngenxxRelease()
{
    if (!_root)
    {
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
    _ngenxx_lua_release();
#endif
#if defined(USE_QJS)
    _ngenxx_js_release();
#endif
}

#pragma mark Device.DeviceInfo

NGenXXDeviceTypeX ngenxxDeviceType()
{
    return static_cast<NGenXXDeviceTypeX>(NGenXX::DeviceInfo::deviceType());
}

std::string ngenxxDeviceName()
{
    return NGenXX::DeviceInfo::deviceName();
}

std::string ngenxxDeviceManufacturer()
{
    return NGenXX::DeviceInfo::deviceManufacturer();
}

std::string ngenxxDeviceModel()
{
    return NGenXX::DeviceInfo::deviceModel();
}

std::string ngenxxDeviceOsVersion()
{
    return NGenXX::DeviceInfo::osVersion();
}

NGenXXDeviceCpuArchX ngenxxDeviceCpuArch()
{
    return static_cast<NGenXXDeviceCpuArchX>(NGenXX::DeviceInfo::cpuArch());
}

#pragma mark Log

void ngenxxLogSetLevel(NGenXXLogLevelX level)
{
    NGenXX::Log::setLevel(static_cast<int>(level));
}

void ngenxxLogSetCallback(const std::function<void(int level, const char *content)> &callback)
{
    NGenXX::Log::setCallback(callback);
}

void ngenxxLogPrint(NGenXXLogLevelX level, const std::string &content)
{
    NGenXX::Log::print(static_cast<int>(level), content);
}

#pragma mark Coding

std::string ngenxxCodingCaseUpper(const std::string_view &str)
{
    return NGenXX::Coding::Case::upper(str);
}

std::string ngenxxCodingCaseLower(const std::string_view &str)
{
    return NGenXX::Coding::Case::lower(str);
}

std::string ngenxxCodingHexBytes2str(const Bytes &bytes)
{
    return NGenXX::Coding::Hex::bytes2str(bytes);
}

Bytes ngenxxCodingHexStr2bytes(const std::string &str)
{
    return NGenXX::Coding::Hex::str2bytes(str);
}

std::string ngenxxCodingBytes2str(const Bytes &bytes)
{
    return NGenXX::Coding::bytes2str(bytes);
}

Bytes ngenxxCodingStr2bytes(const std::string_view &str)
{
    return NGenXX::Coding::str2bytes(str);
}

std::string ngenxxCodingStrTrim(const std::string_view &str)
{
    return NGenXX::Coding::strTrim(str);
}

#pragma mark Crypto

bool ngenxxCryptoRand(size_t len, byte *bytes)
{
    return NGenXX::Crypto::rand(len, bytes);
}

Bytes ngenxxCryptoAesEncrypt(const Bytes &in, const Bytes &key)
{
    return NGenXX::Crypto::AES::encrypt(in, key);
}

Bytes ngenxxCryptoAesDecrypt(const Bytes &in, const Bytes &key)
{
    return NGenXX::Crypto::AES::decrypt(in, key);
}

Bytes ngenxxCryptoAesGcmEncrypt(const Bytes &in, const Bytes &key, const Bytes &initVector, size_t tagBits, const Bytes &aad)
{
    return NGenXX::Crypto::AES::gcmEncrypt(in, key, initVector, aad, tagBits);
}

Bytes ngenxxCryptoAesGcmDecrypt(const Bytes &in, const Bytes &key, const Bytes &initVector, size_t tagBits, const Bytes &aad)
{
    return NGenXX::Crypto::AES::gcmDecrypt(in, key, initVector, aad, tagBits);
}

Bytes ngenxxCryptoRsaEncrypt(const Bytes &in, const Bytes &key, NGenXXCryptoRSAPaddingX padding)
{
    return NGenXX::Crypto::RSA::encrypt(in, key, static_cast<int>(padding));
}

Bytes ngenxxCryptoRsaDecrypt(const Bytes &in, const Bytes &key, NGenXXCryptoRSAPaddingX padding)
{
    return NGenXX::Crypto::RSA::decrypt(in, key, static_cast<int>(padding));
}

Bytes ngenxxCryptoHashMd5(const Bytes &in)
{
    return NGenXX::Crypto::Hash::md5(in);
}

Bytes ngenxxCryptoHashSha1(const Bytes &in)
{
    return NGenXX::Crypto::Hash::sha1(in);
}

Bytes ngenxxCryptoHashSha256(const Bytes &in)
{
    return NGenXX::Crypto::Hash::sha256(in);
}

Bytes ngenxxCryptoBase64Encode(const Bytes &in)
{
    return NGenXX::Crypto::Base64::encode(in);
}

Bytes ngenxxCryptoBase64Decode(const Bytes &in)
{
    return NGenXX::Crypto::Base64::decode(in);
}

#pragma mark Net.Http

NGenXXHttpResponse ngenxxNetHttpRequest(const std::string &url,
                                            NGenXXHttpMethodX method,
                                              const std::string &params,
                                              const Bytes &rawBody,
                                              const std::vector<std::string> &headerV,
                                              const std::vector<std::string> &formFieldNameV,
                                              const std::vector<std::string> &formFieldMimeV,
                                              const std::vector<std::string> &formFieldDataV,
                                              const std::FILE *cFILE, size_t fileSize,
                                              size_t timeout)
{
    NGenXXHttpResponse rsp;
    if (!_http_client || url.empty())
    {
        return rsp;
    }

    std::vector<NGenXX::Net::HttpFormField> vFormFields;
    auto fieldNameCount = formFieldNameV.size();
    if (fieldNameCount > 0)
    {
        vFormFields.reserve(fieldNameCount);
    }
    for (decltype(fieldNameCount) i(0); i < fieldNameCount && i < formFieldMimeV.size() && i < formFieldDataV.size(); i++)
    {
        vFormFields.emplace_back(NGenXX::Net::HttpFormField{
            formFieldNameV[i],
            formFieldMimeV[i],
            formFieldDataV[i],
        });
    }

    auto [code, contentType, headers, data] = _http_client->request(url, static_cast<int>(method), headerV, params, rawBody, vFormFields, cFILE, fileSize, timeout);
    rsp = {
        .code = code,
        .contentType = contentType,
        .headers = headers,
        .data = data
    };
    return rsp;
}

std::string NGenXXHttpResponse::toJson() const
{
    const auto cj = cJSON_CreateObject();
    cJSON_AddItemToObject(cj, "code", cJSON_CreateNumber(this->code));
    cJSON_AddItemToObject(cj, "contentType", cJSON_CreateString(this->contentType.c_str()));

    const auto cjHeaders = cJSON_CreateObject();
    for (const auto &[k, v] : this->headers)
    {
        cJSON_AddItemToObject(cjHeaders, k.c_str(), cJSON_CreateString(v.c_str()));
    }
    cJSON_AddItemToObject(cj, "headers", cjHeaders);

    cJSON_AddItemToObject(cj, "data", cJSON_CreateString(this->data.c_str()));
    auto json = ngenxxJsonCJSON2Str(cj);
    cJSON_Delete(cj);
    return json;
}

NGenXXHttpResponse ngenxxNetHttpRequest(const std::string &url,
                                            NGenXXHttpMethodX method,
                                              const std::unordered_map<std::string, Any> &params,
                                              const Bytes &rawBody,
                                              const std::unordered_map<std::string, std::string> &headers,
                                              const std::vector<std::string> &formFieldNameV,
                                              const std::vector<std::string> &formFieldMimeV,
                                              const std::vector<std::string> &formFieldDataV,
                                              const std::FILE *cFILE, size_t fileSize,
                                              size_t timeout)
{
    std::stringstream ssParams;
    for (const auto &[k, v] : params)
    {
        if (!ssParams.str().empty())
        {
            ssParams << "&";
        }
        ssParams << k << "=";
        std::visit(
            [&ssParams](auto &x)
            { 
                ssParams << x; 
            }, 
            v
        );
    }
    std::vector<std::string> headerV;
    if (auto headersCount = headers.size(); headersCount > 0)
    {
        headerV.reserve(headersCount);
    }
    for (const auto &[k, v] : headers)
    {
        std::ostringstream ss;
        ss << k << ":" << v;
        headerV.emplace_back(ss.str());
    }
    return ngenxxNetHttpRequest(url, method, ssParams.str(), rawBody, headerV,
                                formFieldNameV, formFieldMimeV, formFieldDataV,
                                cFILE, fileSize, timeout);
}

bool ngenxxNetHttpDownload(const std::string &url, const std::string &filePath, size_t timeout)
{
    if (!_http_client || url.empty()|| filePath.empty())
    {
        return false;
    }
    return _http_client->download(url, filePath, timeout);
}

#pragma mark Store.SQLite

void *ngenxxStoreSqliteOpen(const std::string &_id)
{
    if (!_sqlite || !_root || _id.empty())
    {
        return nullptr;
    }
    std::ostringstream ss;
    ss << *_root << "/" << _id << ".db";
    if (const auto ptr = _sqlite->connect(ss.str()).lock()) [[likely]]
    {
        return ptr.get();
    }
    return nullptr;
}

bool ngenxxStoreSqliteExecute(void *const conn, const std::string &sql)
{
    if (conn == nullptr || sql.empty())
    {
        return false;
    }
    const auto xconn = static_cast<NGenXX::Store::SQLite::Connection *>(conn);
    return xconn->execute(sql);
}

void *ngenxxStoreSqliteQueryDo(void *const conn, const std::string &sql)
{
    if (conn == nullptr || sql.empty())
    {
        return nullptr;
    }
    const auto xconn = static_cast<NGenXX::Store::SQLite::Connection *>(conn);
    auto ptrQr = xconn->query(sql);
    return ptrQr? ptrQr.release() : nullptr;//trans ownership to C API, not release the memory.
}

bool ngenxxStoreSqliteQueryReadRow(void *const query_result)
{
    if (query_result == nullptr)
    {
        return false;
    }
    const auto xqr = static_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    return xqr->readRow();
}

std::string ngenxxStoreSqliteQueryReadColumnText(void *const query_result, const std::string &column)
{
    if (query_result == nullptr || column.empty())
    {
        return {};
    }
    const auto xqr = static_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    auto v = (*xqr)[column];
    return *std::get_if<std::string>(&v);
}

int64_t ngenxxStoreSqliteQueryReadColumnInteger(void *const query_result, const std::string &column)
{
    if (query_result == nullptr || column.empty())
    {
        return 0;
    }
    const auto xqr = static_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    const auto v = (*xqr)[column];
    return *std::get_if<int64_t>(&v);
}

double ngenxxStoreSqliteQueryReadColumnFloat(void *const query_result, const std::string &column)
{
    if (query_result == nullptr || column.empty())
    {
        return 0.f;
    }
    const auto xqr = static_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    const auto v = (*xqr)[column];
    return *std::get_if<double>(&v);
}

void ngenxxStoreSqliteQueryDrop(void *const query_result)
{
    if (query_result == nullptr)
    {
        return;
    }
    const auto xqr = static_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    delete xqr;
}

void ngenxxStoreSqliteClose(void *const conn)
{
    /*if (conn == nullptr)
        return;
    const auto xconn = static_cast<NGenXX::Store::SQLite::Connection *>(conn);
    delete xconn;*/
}

#pragma mark Store.KV

void *ngenxxStoreKvOpen(const std::string &_id)
{
    if (!_kv || _id.empty())
    {
        return nullptr;
    }
    if (const auto ptr = _kv->open(_id).lock()) [[likely]]
    {
        return ptr.get();
    }
    return nullptr;
}

std::string ngenxxStoreKvReadString(void *const conn, const std::string_view &k)
{
    if (conn == nullptr || k.empty())
    {
        return {};
    }
    const auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->readString(k);
}

bool ngenxxStoreKvWriteString(void *const conn, const std::string_view &k, const std::string &v)
{
    if (conn == nullptr || k.empty())
    {
        return false;
    }
    const auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

int64_t ngenxxStoreKvReadInteger(void *const conn, const std::string_view &k)
{
    if (conn == nullptr || k.empty())
    {
        return 0;
    }
    const auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->readInteger(k);
}

bool ngenxxStoreKvWriteInteger(void *const conn, const std::string_view &k, int64_t v)
{
    if (conn == nullptr || k.empty())
    {
        return false;
    }
    const auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

double ngenxxStoreKvReadFloat(void *const conn, const std::string_view &k)
{
    if (conn == nullptr || k.empty())
    {
        return 0;
    }
    const auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->readFloat(k);
}

bool ngenxxStoreKvWriteFloat(void *const conn, const std::string_view &k, double v)
{
    if (conn == nullptr || k.empty())
    {
        return false;
    }
    const auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

std::vector<std::string> ngenxxStoreKvAllKeys(void *const conn)
{
    if (conn == nullptr)
    {
        return {};
    }
    const auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->allKeys();
}

bool ngenxxStoreKvContains(void *const conn, const std::string_view &k)
{
    if (conn == nullptr || k.empty())
    {
        return false;
    }
    const auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->contains(k);
}

bool ngenxxStoreKvRemove(void *const conn, const std::string_view &k)
{
    if (conn == nullptr || k.empty())
    {
        return false;
    }
    const auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->remove(k);
}

void ngenxxStoreKvClear(void *const conn)
{
    if (conn == nullptr)
    {
        return;
    }
    const auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    xconn->clear();
}

void ngenxxStoreKvClose(void *const conn)
{
    /*if (conn == nullptr)
        return;
    const auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    delete xconn;*/
}

#pragma mark Json

std::string ngenxxJsonCJSON2Str(void *const cjson)
{
    return NGenXX::Json::cJSON2Str(cjson);
}

std::string ngenxxJsonFromDictAny(const DictAny &dict)
{
    return NGenXX::Json::jsonFromDictAny(dict);
}

DictAny ngenxxJsonToDictAny(const std::string &json)
{
    return NGenXX::Json::jsonToDictAny(json);
}

void *ngenxxJsonDecoderInit(const std::string &json)
{
    return new NGenXX::Json::Decoder(json);
}

bool ngenxxJsonDecoderIsArray(void *const decoder, void *const node)
{
    if (decoder == nullptr)
    {
        return false;
    }
    const auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->isArray(node);
}

bool ngenxxJsonDecoderIsObject(void *const decoder, void *const node)
{
    if (decoder == nullptr)
    {
        return false;
    }
    const auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->isObject(node);
}

void *ngenxxJsonDecoderReadNode(void *const decoder, const std::string &k, void *const node)
{
    if (decoder == nullptr || k.empty())
    {
        return nullptr;
    }
    const auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNode(node, k);
}

std::string ngenxxJsonDecoderReadString(void *const decoder, void *const node)
{
    if (decoder == nullptr)
    {
        return {};
    }
    const auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readString(node);
}

double ngenxxJsonDecoderReadNumber(void *const decoder, void *const node)
{
    if (decoder == nullptr)
    {
        return 0;
    }
    const auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNumber(node);
}

void *ngenxxJsonDecoderReadChild(void *const decoder, void *const node)
{
    if (decoder == nullptr)
    {
        return nullptr;
    }
    const auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readChild(node);
}

void ngenxxJsonDecoderReadChildren(void *const decoder, std::function<void(size_t idx, const void * child)> &&callback, void *const node)
{
    if (decoder == nullptr)
    {
        return;
    }
    const auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    xdecoder->readChildren(node, std::move(callback));
}

void *ngenxxJsonDecoderReadNext(void *const decoder, void *const node)
{
    if (decoder == nullptr)
    {
        return nullptr;
    }
    const auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNext(node);
}

void ngenxxJsonDecoderRelease(void *const decoder)
{
    if (decoder == nullptr)
    {
        return;
    }
    const auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    delete xdecoder;
}

#pragma mark Zip

void *ngenxxZZipInit(const NGenXXZipCompressModeX mode, size_t bufferSize, const NGenXXZFormatX format)
{
    void *zip = nullptr;
    try
    {
        zip = new NGenXX::Z::Zip(static_cast<int>(mode), bufferSize, static_cast<int>(format));
    }
    catch (const std::exception &e)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "ngenxxZZipInit failed: {}", e.what());
    }
    return zip;
}

size_t ngenxxZZipInput(void *const zip, const Bytes &inBytes, bool inFinish)
{
    if (zip == nullptr)
    {
        return 0;
    }
    const auto xzip = static_cast<NGenXX::Z::Zip *>(zip);
    return xzip->input(inBytes, inFinish);
}

Bytes ngenxxZZipProcessDo(void *const zip)
{
    if (zip == nullptr)
    {
        return {};
    }
    const auto xzip = static_cast<NGenXX::Z::Zip *>(zip);
    return xzip->processDo();
}

bool ngenxxZZipProcessFinished(void *const zip)
{
    if (zip == nullptr)
    {
        return false;
    }
    const auto xzip = static_cast<NGenXX::Z::Zip *>(zip);
    return xzip->processFinished();
}

void ngenxxZZipRelease(void *const zip)
{
    if (zip == nullptr)
    {
        return;
    }
    const auto xzip = static_cast<NGenXX::Z::Zip *>(zip);
    delete xzip;
}

void *ngenxxZUnzipInit(size_t bufferSize, const NGenXXZFormatX format)
{
    void *unzip = nullptr;
    try
    {
        unzip = new NGenXX::Z::UnZip(bufferSize, static_cast<int>(format));
    }
    catch (const std::exception &e)
    {
        ngenxxLogPrintF(NGenXXLogLevelX::Error, "ngenxxZUnzipInit failed: {}", e.what());
    }
    return unzip;
}

size_t ngenxxZUnzipInput(void *const unzip, const Bytes &inBytes, bool inFinish)
{
    if (unzip == nullptr)
    {
        return 0;
    }
    const auto xunzip = static_cast<NGenXX::Z::UnZip *>(unzip);
    return xunzip->input(inBytes, inFinish);
}

Bytes ngenxxZUnzipProcessDo(void *const unzip)
{
    if (unzip == nullptr)
    {
        return {};
    }
    const auto xunzip = static_cast<NGenXX::Z::UnZip *>(unzip);
    return xunzip->processDo();
}

bool ngenxxZUnzipProcessFinished(void *const unzip)
{
    if (unzip == nullptr)
    {
        return false;
    }
    const auto xunzip = static_cast<NGenXX::Z::UnZip *>(unzip);
    return xunzip->processFinished();
}

void ngenxxZUnzipRelease(void *const unzip)
{
    if (unzip == nullptr)
    {
        return;
    }
    const auto xunzip = static_cast<NGenXX::Z::UnZip *>(unzip);
    delete xunzip;
}

bool ngenxxZCFileZip(std::FILE *cFILEIn, std::FILE *cFILEOut, const NGenXXZipCompressModeX mode, size_t bufferSize, const NGenXXZFormatX format)
{
    if (bufferSize == 0)
    {
        return false;
    }
    if (cFILEIn == nullptr || cFILEOut == nullptr)
    {
        return false;
    }
    return NGenXX::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), cFILEIn, cFILEOut);
}

bool ngenxxZCFileUnzip(std::FILE *cFILEIn, std::FILE *cFILEOut, size_t bufferSize, const NGenXXZFormatX format)
{
    if (bufferSize == 0)
    {
        return false;
    }
    if (cFILEIn == nullptr || cFILEOut == nullptr)
    {
        return false;
    }
    return NGenXX::Z::unzip(bufferSize, static_cast<int>(format), cFILEIn, cFILEOut);
}

bool ngenxxZCxxStreamZip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut, const NGenXXZipCompressModeX mode, size_t bufferSize, const NGenXXZFormatX format)
{
    if (bufferSize == 0)
    {
        return false;
    }
    if (cxxStreamIn == nullptr || cxxStreamOut == nullptr)
    {
        return false;
    }
    return NGenXX::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), cxxStreamIn, cxxStreamOut);
}

bool ngenxxZCxxStreamUnzip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut, size_t bufferSize, const NGenXXZFormatX format)
{
    if (bufferSize == 0)
    {
        return false;
    }
    if (cxxStreamIn == nullptr || cxxStreamOut == nullptr)
    {
        return false;
    }
    return NGenXX::Z::unzip(bufferSize, static_cast<int>(format), cxxStreamIn, cxxStreamOut);
}

Bytes ngenxxZBytesZip(const Bytes &inBytes, const NGenXXZipCompressModeX mode, size_t bufferSize, const NGenXXZFormatX format)
{
    if (bufferSize == 0)
    {
        return {};
    }
    if (inBytes.empty())
    {
        return {};
    }
    return NGenXX::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), inBytes);
}

Bytes ngenxxZBytesUnzip(const Bytes &inBytes, size_t bufferSize, const NGenXXZFormatX format)
{
    if (bufferSize == 0)
    {
        return {};
    }
    if (inBytes.empty())
    {
        return {};
    }
    return NGenXX::Z::unzip(bufferSize, static_cast<int>(format), inBytes);
}