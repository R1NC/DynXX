#if defined(__cplusplus)

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <vector>
#include <string>
#include <sstream>
#include <memory>

#include <NGenXX.hxx>
#include "net/HttpClient.hxx"
#include "store/SQLite.hxx"
#include "store/KV.hxx"
#include "json/JsonDecoder.hxx"
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
#endif

#define VERSION "1.0.0"

std::unique_ptr<NGenXX::Net::HttpClient> _ngenxx_http_client = nullptr;
std::unique_ptr<NGenXX::Store::SQLite> _ngenxx_sqlite = nullptr;
std::unique_ptr<NGenXX::Store::KV> _ngenxx_kv = nullptr;
std::unique_ptr<const std::string> _ngenxx_root = nullptr;

std::string ngenxxGetVersion()
{
    return VERSION;
}

std::string ngenxxRootPath()
{
    std::string s;
    if (!_ngenxx_root)
    {
        return s;
    }
    return *_ngenxx_root.get();
}

bool ngenxxInit(const std::string &root)
{
    if (_ngenxx_root)
    {
        return true;
    }
    if (root.empty())
    {
        return false;
    }
    _ngenxx_root = std::make_unique<const std::string>(root);
    _ngenxx_sqlite = std::make_unique<NGenXX::Store::SQLite>();
    _ngenxx_kv = std::make_unique<NGenXX::Store::KV>(*_ngenxx_root.get());
    _ngenxx_http_client = std::make_unique<NGenXX::Net::HttpClient>();
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
    if (!_ngenxx_root)
    {
        return;
    }
    _ngenxx_root.reset();
    _ngenxx_http_client.reset();
    _ngenxx_sqlite->closeAll();
    _ngenxx_sqlite.reset();
    _ngenxx_kv->closeAll();
    _ngenxx_kv.reset();
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
    return static_cast<NGenXXDeviceTypeX>(NGenXX::Device::DeviceInfo::deviceType());
}

std::string ngenxxDeviceName()
{
    return NGenXX::Device::DeviceInfo::deviceName();
}

std::string ngenxxDeviceManufacturer()
{
    return NGenXX::Device::DeviceInfo::deviceManufacturer();
}

std::string ngenxxDeviceOsVersion()
{
    return NGenXX::Device::DeviceInfo::osVersion();
}

NGenXXDeviceCpuArchX ngenxxDeviceCpuArch()
{
    return static_cast<NGenXXDeviceCpuArchX>(NGenXX::Device::DeviceInfo::cpuArch());
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
    if (!_ngenxx_http_client || url.empty())
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

    auto t = _ngenxx_http_client->request(url, static_cast<int>(method), headerV, params, rawBody, vFormFields, cFILE, fileSize, timeout);
    rsp = {
        .code = t.code,
        .contentType = t.contentType,
        .headers = t.headers,
        .data = t.data
    };
    return rsp;
}

std::string NGenXXHttpResponse::toJson() const
{
    auto cj = cJSON_CreateObject();
    cJSON_AddItemToObject(cj, "code", cJSON_CreateNumber(this->code));
    cJSON_AddItemToObject(cj, "contentType", cJSON_CreateString(this->contentType.c_str()));

    auto cjHeaders = cJSON_CreateObject();
    for (const auto &[k, v] : this->headers)
    {
        cJSON_AddItemToObject(cjHeaders, k.c_str(), cJSON_CreateString(v.c_str()));
    }
    cJSON_AddItemToObject(cj, "headers", cjHeaders);

    cJSON_AddItemToObject(cj, "data", cJSON_CreateString(this->data.c_str()));
    return cJSON_PrintUnformatted(cj);
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
    for (auto &it : params)
    {
        if (ssParams.str().length() > 0)
        {
            ssParams << "&";
        }
        ssParams << it.first << "=";
        std::visit(
            [&ssParams](auto &x)
            { 
                ssParams << x; 
            }, 
            it.second
        );
    }
    std::vector<std::string> headerV;
    auto headersCount = headers.size();
    if (headersCount > 0)
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
    if (!_ngenxx_http_client || url.empty()|| filePath.empty())
    {
        return false;
    }
    return _ngenxx_http_client->download(url, filePath, timeout);
}

#pragma mark Store.SQLite

void *ngenxxStoreSqliteOpen(const std::string &_id)
{
    if (!_ngenxx_sqlite || !_ngenxx_root || _id.empty())
    {
        return nullptr;
    }
    std::ostringstream ss;
    ss << *_ngenxx_root.get() << "/" << _id << ".db";
    if (auto ptr = _ngenxx_sqlite->connect(ss.str()).lock()) [[likely]]
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
    auto xconn = static_cast<NGenXX::Store::SQLite::Connection *>(conn);
    return xconn->execute(sql);
}

void *ngenxxStoreSqliteQueryDo(void *const conn, const std::string &sql)
{
    if (conn == nullptr || sql.empty())
    {
        return nullptr;
    }
    auto xconn = static_cast<NGenXX::Store::SQLite::Connection *>(conn);
    auto ptrQr = xconn->query(sql);
    return ptrQr? ptrQr.release() : nullptr;//trans ownership to C API, not release the memory.
}

bool ngenxxStoreSqliteQueryReadRow(void *const query_result)
{
    if (query_result == nullptr)
    {
        return false;
    }
    auto xqr = static_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    return xqr->readRow();
}

std::string ngenxxStoreSqliteQueryReadColumnText(void *const query_result, const std::string &column)
{
    std::string s;
    if (query_result == nullptr || column.empty())
    {
        return s;
    }
    auto xqr = static_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    auto a = xqr->readColumn(column);
    return *std::get_if<std::string>(&a);
}

int64_t ngenxxStoreSqliteQueryReadColumnInteger(void *const query_result, const std::string &column)
{
    if (query_result == nullptr || column.empty())
    {
        return 0;
    }
    auto xqr = static_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    auto a = xqr->readColumn(column);
    return *std::get_if<int64_t>(&a);
}

double ngenxxStoreSqliteQueryReadColumnFloat(void *const query_result, const std::string &column)
{
    if (query_result == nullptr || column.empty())
    {
        return 0.f;
    }
    auto xqr = static_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    auto a = xqr->readColumn(column);
    return *std::get_if<double>(&a);
}

void ngenxxStoreSqliteQueryDrop(void *const query_result)
{
    if (query_result == nullptr)
    {
        return;
    }
    auto xqr = static_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    delete xqr;
}

void ngenxxStoreSqliteClose(void *const conn)
{
    /*if (conn == nullptr)
        return;
    auto xconn = static_cast<NGenXX::Store::SQLite::Connection *>(conn);
    delete xconn;*/
}

#pragma mark Store.KV

void *ngenxxStoreKvOpen(const std::string &_id)
{
    if (!_ngenxx_kv || _id.empty())
    {
        return nullptr;
    }
    if (auto ptr = _ngenxx_kv->open(_id).lock()) [[likely]]
    {
        return ptr.get();
    }
    return nullptr;
}

std::string ngenxxStoreKvReadString(void *const conn, const std::string_view &k)
{
    std::string s;
    if (conn == nullptr || k.empty())
    {
        return s;
    }
    auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->readString(k);
}

bool ngenxxStoreKvWriteString(void *const conn, const std::string_view &k, const std::string &v)
{
    if (conn == nullptr || k.empty())
    {
        return false;
    }
    auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

int64_t ngenxxStoreKvReadInteger(void *const conn, const std::string_view &k)
{
    if (conn == nullptr || k.empty())
    {
        return 0;
    }
    auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->readInteger(k);
}

bool ngenxxStoreKvWriteInteger(void *const conn, const std::string_view &k, int64_t v)
{
    if (conn == nullptr || k.empty())
    {
        return false;
    }
    auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

double ngenxxStoreKvReadFloat(void *const conn, const std::string_view &k)
{
    if (conn == nullptr || k.empty())
    {
        return 0;
    }
    auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->readFloat(k);
}

bool ngenxxStoreKvWriteFloat(void *const conn, const std::string_view &k, double v)
{
    if (conn == nullptr || k.empty())
    {
        return false;
    }
    auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

const std::vector<std::string> ngenxxStoreKvAllKeys(void *const conn)
{
    std::vector<std::string> v;
    if (conn == nullptr)
    {
        return v;
    }
    auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->allKeys();
}

bool ngenxxStoreKvContains(void *const conn, const std::string_view &k)
{
    if (conn == nullptr || k.empty())
    {
        return false;
    }
    auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->contains(k);
}

bool ngenxxStoreKvRemove(void *const conn, const std::string_view &k)
{
    if (conn == nullptr || k.empty())
    {
        return false;
    }
    auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->remove(k);
}

void ngenxxStoreKvClear(void *const conn)
{
    if (conn == nullptr)
    {
        return;
    }
    auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    xconn->clear();
}

void ngenxxStoreKvClose(void *const conn)
{
    /*if (conn == nullptr)
        return;
    auto xconn = static_cast<NGenXX::Store::KV::Connection *>(conn);
    delete xconn;*/
}

#pragma mark Json.Decoder

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
    auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->isArray(node);
}

bool ngenxxJsonDecoderIsObject(void *const decoder, void *const node)
{
    if (decoder == nullptr)
    {
        return false;
    }
    auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->isObject(node);
}

void *ngenxxJsonDecoderReadNode(void *const decoder, const std::string &k, void *const node)
{
    if (decoder == nullptr || k.empty())
    {
        return nullptr;
    }
    auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNode(node, k);
}

std::string ngenxxJsonDecoderReadString(void *const decoder, void *const node)
{
    std::string s;
    if (decoder == nullptr)
    {
        return s;
    }
    auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readString(node);
}

double ngenxxJsonDecoderReadNumber(void *const decoder, void *const node)
{
    if (decoder == nullptr)
    {
        return 0;
    }
    auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNumber(node);
}

void *ngenxxJsonDecoderReadChild(void *const decoder, void *const node)
{
    if (decoder == nullptr)
    {
        return nullptr;
    }
    auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readChild(node);
}

void ngenxxJsonDecoderReadChildren(void *const decoder, std::function<void(size_t idx, const void * child)> &&callback, void *const node)
{
    if (decoder == nullptr)
    {
        return;
    }
    auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    xdecoder->readChildren(node, std::move(callback));
}

void *ngenxxJsonDecoderReadNext(void *const decoder, void *const node)
{
    if (decoder == nullptr)
    {
        return nullptr;
    }
    auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNext(node);
}

void ngenxxJsonDecoderRelease(void *const decoder)
{
    if (decoder == nullptr)
    {
        return;
    }
    auto xdecoder = static_cast<NGenXX::Json::Decoder *>(decoder);
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
        ngenxxLogPrint(NGenXXLogLevelX::Error, "ngenxx_z_zip_init failed");
    }
    return zip;
}

size_t ngenxxZZipInput(void *const zip, const Bytes &inBytes, bool inFinish)
{
    if (zip == nullptr)
    {
        return 0;
    }
    auto xzip = static_cast<NGenXX::Z::Zip *>(zip);
    return xzip->input(inBytes, inFinish);
}

Bytes ngenxxZZipProcessDo(void *const zip)
{
    if (zip == nullptr)
    {
        return {};
    }
    auto xzip = static_cast<NGenXX::Z::Zip *>(zip);
    return xzip->processDo();
}

bool ngenxxZZipProcessFinished(void *const zip)
{
    if (zip == nullptr)
    {
        return false;
    }
    auto xzip = static_cast<NGenXX::Z::Zip *>(zip);
    return xzip->processFinished();
}

void ngenxxZZipRelease(void *const zip)
{
    if (zip == nullptr)
    {
        return;
    }
    auto xzip = static_cast<NGenXX::Z::Zip *>(zip);
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
        ngenxxLogPrint(NGenXXLogLevelX::Error, "ngenxx_z_unzip_init failed");
    }
    return unzip;
}

size_t ngenxxZUnzipInput(void *const unzip, const Bytes &inBytes, bool inFinish)
{
    if (unzip == nullptr)
    {
        return 0;
    }
    auto xunzip = static_cast<NGenXX::Z::UnZip *>(unzip);
    return xunzip->input(inBytes, inFinish);
}

Bytes ngenxxZUnzipProcessDo(void *const unzip)
{
    if (unzip == nullptr)
    {
        return {};
    }
    auto xunzip = static_cast<NGenXX::Z::UnZip *>(unzip);
    return xunzip->processDo();
}

bool ngenxxZUnzipProcessFinished(void *const unzip)
{
    if (unzip == nullptr)
    {
        return false;
    }
    auto xunzip = static_cast<NGenXX::Z::UnZip *>(unzip);
    return xunzip->processFinished();
}

void ngenxxZUnzipRelease(void *const unzip)
{
    if (unzip == nullptr)
    {
        return;
    }
    auto xunzip = static_cast<NGenXX::Z::UnZip *>(unzip);
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