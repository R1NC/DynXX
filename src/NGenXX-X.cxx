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

#define VERSION "0.0.1"

std::shared_ptr<NGenXX::Net::HttpClient> _ngenxx_http_client = nullptr;
std::shared_ptr<NGenXX::Store::SQLite> _ngenxx_sqlite = nullptr;
std::shared_ptr<NGenXX::Store::KV> _ngenxx_kv = nullptr;
std::shared_ptr<const std::string> _ngenxx_root = nullptr;

const std::string ngenxxGetVersion(void)
{
    return VERSION;
}

const std::string ngenxxRootPath()
{
    std::string s;
    if (!_ngenxx_root)
    {
        return s;
    }
    return *_ngenxx_root;
}

bool ngenxxInit(const std::string &root)
{
    if (_ngenxx_root != nullptr)
    {
        return true;
    }
    if (root.length() == 0)
    {
        return false;
    }
    _ngenxx_root = std::make_shared<const std::string>(root);
    _ngenxx_sqlite = std::make_shared<NGenXX::Store::SQLite>();
    _ngenxx_kv = std::make_shared<NGenXX::Store::KV>(*_ngenxx_root);
    _ngenxx_http_client = std::make_shared<NGenXX::Net::HttpClient>();
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
    if (_ngenxx_root == nullptr)
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

const std::string ngenxxDeviceName()
{
    return NGenXX::Device::DeviceInfo::deviceName();
}

const std::string ngenxxDeviceManufacturer()
{
    return NGenXX::Device::DeviceInfo::deviceManufacturer();
}

const std::string ngenxxDeviceOsVersion()
{
    return NGenXX::Device::DeviceInfo::osVersion();
}

NGenXXDeviceCpuArchX ngenxxDeviceCpuArch()
{
    return static_cast<NGenXXDeviceCpuArchX>(NGenXX::Device::DeviceInfo::cpuArch());
}

#pragma mark Log

void ngenxxLogSetLevel(const NGenXXLogLevelX level)
{
    NGenXX::Log::setLevel(static_cast<int>(level));
}

void ngenxxLogSetCallback(const std::function<void(const int level, const char *content)> &callback)
{
    NGenXX::Log::setCallback(callback);
}

void ngenxxLogPrint(const NGenXXLogLevelX level, const std::string &content)
{
    NGenXX::Log::print(static_cast<int>(level), content);
}

#pragma mark Coding

const std::string ngenxxCodingHexBytes2str(const Bytes &bytes)
{
    return NGenXX::Coding::Hex::bytes2str(bytes);
}

const Bytes ngenxxCodingHexStr2bytes(const std::string &str)
{
    return NGenXX::Coding::Hex::str2bytes(str);
}

const std::string ngenxxCodingBytes2str(const Bytes &bytes)
{
    return NGenXX::Coding::bytes2str(bytes);
}

const Bytes ngenxxCodingStr2bytes(const std::string &str)
{
    return NGenXX::Coding::str2bytes(str);
}

const std::string ngenxxCodingCaseUpper(const std::string &str)
{
    return NGenXX::Coding::Case::upper(str);
}

const std::string ngenxxCodingCaseLower(const std::string &str)
{
    return NGenXX::Coding::Case::lower(str);
}

#pragma mark Crypto

bool ngenxxCryptoRand(const size_t len, byte *bytes)
{
    return NGenXX::Crypto::rand(len, bytes);
}

const Bytes ngenxxCryptoAesEncrypt(const Bytes &in, const Bytes &key)
{
    return NGenXX::Crypto::AES::encrypt(in, key);
}

const Bytes ngenxxCryptoAesDecrypt(const Bytes &in, const Bytes &key)
{
    return NGenXX::Crypto::AES::decrypt(in, key);
}

const Bytes ngenxxCryptoAesGcmEncrypt(const Bytes &in, const Bytes &key, const Bytes &initVector, const size_t tagBits, const Bytes &aad)
{
    return NGenXX::Crypto::AES::gcmEncrypt(in, key, initVector, aad, tagBits);
}

const Bytes ngenxxCryptoAesGcmDecrypt(const Bytes &in, const Bytes &key, const Bytes &initVector, const size_t tagBits, const Bytes &aad)
{
    return NGenXX::Crypto::AES::gcmDecrypt(in, key, initVector, aad, tagBits);
}

const Bytes ngenxxCryptoRsaEncrypt(const Bytes &in, const Bytes &key, const NGenXXCryptoRSAPaddingX padding)
{
    return NGenXX::Crypto::RSA::encrypt(in, key, static_cast<int>(padding));
}

const Bytes ngenxxCryptoRsaDecrypt(const Bytes &in, const Bytes &key, const NGenXXCryptoRSAPaddingX padding)
{
    return NGenXX::Crypto::RSA::decrypt(in, key, static_cast<int>(padding));
}

const Bytes ngenxxCryptoHashMd5(const Bytes &in)
{
    return NGenXX::Crypto::Hash::md5(in);
}

const Bytes ngenxxCryptoHashSha256(const Bytes &in)
{
    return NGenXX::Crypto::Hash::sha256(in);
}

const Bytes ngenxxCryptoBase64Encode(const Bytes &in)
{
    return NGenXX::Crypto::Base64::encode(in);
}

const Bytes ngenxxCryptoBase64Decode(const Bytes &in)
{
    return NGenXX::Crypto::Base64::decode(in);
}

#pragma mark Net.Http

const NGenXXHttpResponse ngenxxNetHttpRequest(const std::string &url,
                                              const NGenXXHttpMethodX method,
                                              const std::string &params,
                                              const Bytes &rawBody,
                                              const std::vector<std::string> &headerV,
                                              const std::vector<std::string> &formFieldNameV,
                                              const std::vector<std::string> &formFieldMimeV,
                                              const std::vector<std::string> &formFieldDataV,
                                              const std::FILE *cFILE, const size_t fileSize,
                                              const size_t timeout)
{
    NGenXXHttpResponse rsp;
    if (_ngenxx_http_client == nullptr || url.length() == 0)
    {
        return rsp;
    }

    std::vector<NGenXX::Net::HttpFormField> vFormFields;
    for (int i = 0; i < formFieldNameV.size() && i < formFieldMimeV.size() && i < formFieldDataV.size(); i++)
    {
        vFormFields.push_back({
            .name = formFieldNameV[i],
            .mime = formFieldMimeV[i],
            .data = formFieldDataV[i],
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

const std::string NGenXXHttpResponse::toJson()
{
    auto cj = cJSON_CreateObject();
    cJSON_AddItemToObject(cj, "code", cJSON_CreateNumber(this->code));
    cJSON_AddItemToObject(cj, "contentType", cJSON_CreateString(this->contentType.c_str()));

    auto cjHeaders = cJSON_CreateObject();
    std::for_each(this->headers.begin(), this->headers.end(), 
        [&cjHeaders](const auto& pair) 
        {
            auto [k, v] = pair;
            cJSON_AddItemToObject(cjHeaders, k.c_str(), cJSON_CreateString(v.c_str()));
        }
    );
    cJSON_AddItemToObject(cj, "headers", cjHeaders);

    cJSON_AddItemToObject(cj, "data", cJSON_CreateString(this->data.c_str()));
    return cJSON_PrintUnformatted(cj);
}

const NGenXXHttpResponse ngenxxNetHttpRequest(const std::string &url,
                                              const NGenXXHttpMethodX method,
                                              const std::unordered_map<std::string, Any> &params,
                                              const Bytes &rawBody,
                                              const std::unordered_map<std::string, std::string> &headers,
                                              const std::vector<std::string> &formFieldNameV,
                                              const std::vector<std::string> &formFieldMimeV,
                                              const std::vector<std::string> &formFieldDataV,
                                              const std::FILE *cFILE, const size_t fileSize,
                                              const size_t timeout)
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
    for (const auto &pair : headers)
    {
        auto [k, v] = pair;
        std::stringstream ss;
        ss << k << ":" << v;
        headerV.push_back(ss.str());
    }
    return ngenxxNetHttpRequest(url, method, ssParams.str(), rawBody, headerV,
                                formFieldNameV, formFieldMimeV, formFieldDataV,
                                cFILE, fileSize, timeout);
}

bool ngenxxNetHttpDownload(const std::string &url, const std::string &filePath, const size_t timeout)
{
    if (_ngenxx_http_client == nullptr || url.length() == 0 || filePath.length() == 0)
    {
        return false;
    }
    return _ngenxx_http_client->download(url, filePath, timeout);
}

#pragma mark Store.SQLite

void *ngenxxStoreSqliteOpen(const std::string &_id)
{
    if (_ngenxx_sqlite == nullptr || _ngenxx_root == nullptr || _id.length() == 0)
    {
        return NULL;
    }
    std::string dbFile = *_ngenxx_root + "/" + std::string(_id) + ".db";
    return _ngenxx_sqlite->connect(dbFile);
}

bool ngenxxStoreSqliteExecute(void *const conn, const std::string &sql)
{
    if (conn == NULL || sql.length() == 0)
    {
        return false;
    }
    auto xconn = reinterpret_cast<NGenXX::Store::SQLite::Connection *>(conn);
    return xconn->execute(sql);
}

void *ngenxxStoreSqliteQueryDo(void *const conn, const std::string &sql)
{
    if (conn == NULL || sql.length() == 0)
    {
        return NULL;
    }
    auto xconn = reinterpret_cast<NGenXX::Store::SQLite::Connection *>(conn);
    return xconn->query(sql);
}

bool ngenxxStoreSqliteQueryReadRow(void *const query_result)
{
    if (query_result == NULL)
    {
        return false;
    }
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    return xqr->readRow();
}

const std::string ngenxxStoreSqliteQueryReadColumnText(void *const query_result, const std::string &column)
{
    std::string s;
    if (query_result == NULL || column.length() == 0)
    {
        return s;
    }
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    auto a = xqr->readColumn(column);
    return *std::get_if<std::string>(&a);
}

int64_t ngenxxStoreSqliteQueryReadColumnInteger(void *const query_result, const std::string &column)
{
    if (query_result == NULL || column.length() == 0)
    {
        return 0;
    }
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    auto a = xqr->readColumn(column);
    return *std::get_if<int64_t>(&a);
}

double ngenxxStoreSqliteQueryReadColumnFloat(void *const query_result, const std::string &column)
{
    if (query_result == NULL || column.length() == 0)
    {
        return 0.f;
    }
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    auto a = xqr->readColumn(column);
    return *std::get_if<double>(&a);
}

void ngenxxStoreSqliteQueryDrop(void *const query_result)
{
    if (query_result == NULL)
    {
        return;
    }
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    delete xqr;
}

void ngenxxStoreSqliteClose(void *const conn)
{
    /*if (conn == NULL)
        return;
    auto xconn = reinterpret_cast<NGenXX::Store::SQLite::Connection *>(conn);
    delete xconn;*/
}

#pragma mark Store.KV

void *ngenxxStoreKvOpen(const std::string &_id)
{
    if (_ngenxx_kv == nullptr || _id.length() == 0)
    {
        return NULL;
    }
    return _ngenxx_kv->open(_id);
}

const std::string ngenxxStoreKvReadString(void *const conn, const std::string &k)
{
    std::string s;
    if (conn == NULL || k.length() == 0)
    {
        return s;
    }
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->readString(k);
}

bool ngenxxStoreKvWriteString(void *const conn, const std::string &k, const std::string &v)
{
    if (conn == NULL || k.length() == 0)
    {
        return false;
    }
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

int64_t ngenxxStoreKvReadInteger(void *const conn, const std::string &k)
{
    if (conn == NULL || k.length() == 0)
    {
        return 0;
    }
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->readInteger(k);
}

bool ngenxxStoreKvWriteInteger(void *const conn, const std::string &k, int64_t v)
{
    if (conn == NULL || k.length() == 0)
    {
        return false;
    }
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

double ngenxxStoreKvReadFloat(void *const conn, const std::string &k)
{
    if (conn == NULL || k.length() == 0)
    {
        return 0;
    }
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->readFloat(k);
}

bool ngenxxStoreKvWriteFloat(void *const conn, const std::string &k, double v)
{
    if (conn == NULL || k.length() == 0)
    {
        return false;
    }
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

const std::vector<std::string> ngenxxStoreKvAllKeys(void *const conn)
{
    std::vector<std::string> v;
    if (conn == NULL)
    {
        return v;
    }
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->allKeys();
}

bool ngenxxStoreKvContains(void *const conn, const std::string &k)
{
    if (conn == NULL || k.length() == 0)
    {
        return false;
    }
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->contains(k);
}

bool ngenxxStoreKvRemove(void *const conn, const std::string &k)
{
    if (conn == NULL || k.length() == 0)
    {
        return false;
    }
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->remove(k);
}

void ngenxxStoreKvClear(void *const conn)
{
    if (conn == NULL)
    {
        return;
    }
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    xconn->clear();
}

void ngenxxStoreKvClose(void *const conn)
{
    /*if (conn == NULL)
        return;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    delete xconn;*/
}

#pragma mark Json.Decoder

void *ngenxxJsonDecoderInit(const std::string &json)
{
    return new NGenXX::Json::Decoder(json);
}

bool ngenxxJsonDecoderIsArray(void *const decoder, void *const node)
{
    if (decoder == NULL)
    {
        return false;
    }
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->isArray(node);
}

bool ngenxxJsonDecoderIsObject(void *const decoder, void *const node)
{
    if (decoder == NULL)
    {
        return false;
    }
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->isObject(node);
}

void *ngenxxJsonDecoderReadNode(void *const decoder, const std::string &k, void *const node)
{
    if (decoder == NULL || k.length() == 0)
    {
        return NULL;
    }
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNode(node, k);
}

const std::string ngenxxJsonDecoderReadString(void *const decoder, void *const node)
{
    std::string s;
    if (decoder == NULL)
    {
        return s;
    }
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readString(node);
}

double ngenxxJsonDecoderReadNumber(void *const decoder, void *const node)
{
    if (decoder == NULL)
    {
        return 0;
    }
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNumber(node);
}

void *ngenxxJsonDecoderReadChild(void *const decoder, void *const node)
{
    if (decoder == NULL)
    {
        return NULL;
    }
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readChild(node);
}

void ngenxxJsonDecoderReadChildren(void *const decoder, const std::function<void(const size_t idx, const void *const child)> &callback, void *const node)
{
    if (decoder == NULL)
    {
        return;
    }
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    xdecoder->readChildren(node, callback);
}

void *ngenxxJsonDecoderReadNext(void *const decoder, void *const node)
{
    if (decoder == NULL)
    {
        return NULL;
    }
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNext(node);
}

void ngenxxJsonDecoderRelease(void *const decoder)
{
    if (decoder == NULL)
    {
        return;
    }
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    delete xdecoder;
}

#pragma mark Zip

void *ngenxxZZipInit(const NGenXXZipCompressModeX mode, const size_t bufferSize, const NGenXXZFormatX format)
{
    void *zip = NULL;
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

size_t ngenxxZZipInput(void *const zip, const Bytes &inBytes, const bool inFinish)
{
    if (zip == NULL)
    {
        return 0;
    }
    auto xzip = reinterpret_cast<NGenXX::Z::Zip *>(zip);
    return xzip->input(inBytes, inFinish);
}

const Bytes ngenxxZZipProcessDo(void *const zip)
{
    if (zip == NULL)
    {
        return BytesEmpty;
    }
    auto xzip = reinterpret_cast<NGenXX::Z::Zip *>(zip);
    return xzip->processDo();
}

bool ngenxxZZipProcessFinished(void *const zip)
{
    if (zip == NULL)
    {
        return false;
    }
    auto xzip = reinterpret_cast<NGenXX::Z::Zip *>(zip);
    return xzip->processFinished();
}

void ngenxxZZipRelease(void *const zip)
{
    if (zip == NULL)
    {
        return;
    }
    auto xzip = reinterpret_cast<NGenXX::Z::Zip *>(zip);
    delete xzip;
}

void *ngenxxZUnzipInit(const size_t bufferSize, const NGenXXZFormatX format)
{
    void *unzip = NULL;
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

size_t ngenxxZUnzipInput(void *const unzip, const Bytes &inBytes, const bool inFinish)
{
    if (unzip == NULL)
    {
        return 0;
    }
    auto xunzip = reinterpret_cast<NGenXX::Z::UnZip *>(unzip);
    return xunzip->input(inBytes, inFinish);
}

const Bytes ngenxxZUnzipProcessDo(void *const unzip)
{
    if (unzip == NULL)
    {
        return BytesEmpty;
    }
    auto xunzip = reinterpret_cast<NGenXX::Z::UnZip *>(unzip);
    return xunzip->processDo();
}

bool ngenxxZUnzipProcessFinished(void *const unzip)
{
    if (unzip == NULL)
    {
        return false;
    }
    auto xunzip = reinterpret_cast<NGenXX::Z::UnZip *>(unzip);
    return xunzip->processFinished();
}

void ngenxxZUnzipRelease(void *const unzip)
{
    if (unzip == NULL)
    {
        return;
    }
    auto xunzip = reinterpret_cast<NGenXX::Z::UnZip *>(unzip);
    delete xunzip;
}

bool ngenxxZCFileZip(std::FILE *cFILEIn, std::FILE *cFILEOut, const NGenXXZipCompressModeX mode, const size_t bufferSize, const NGenXXZFormatX format)
{
    if (bufferSize <= 0)
    {
        return false;
    }
    if (cFILEIn == NULL || cFILEOut == NULL)
    {
        return false;
    }
    return NGenXX::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), cFILEIn, cFILEOut);
}

bool ngenxxZCFileUnzip(std::FILE *cFILEIn, std::FILE *cFILEOut, const size_t bufferSize, const NGenXXZFormatX format)
{
    if (bufferSize <= 0)
    {
        return false;
    }
    if (cFILEIn == NULL || cFILEOut == NULL)
    {
        return false;
    }
    return NGenXX::Z::unzip(bufferSize, static_cast<int>(format), cFILEIn, cFILEOut);
}

bool ngenxxZCxxStreamZip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut, const NGenXXZipCompressModeX mode, const size_t bufferSize, const NGenXXZFormatX format)
{
    if (bufferSize <= 0)
    {
        return false;
    }
    if (cxxStreamIn == NULL || cxxStreamOut == NULL)
    {
        return false;
    }
    return NGenXX::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), cxxStreamIn, cxxStreamOut);
}

bool ngenxxZCxxStreamUnzip(std::istream *cxxStreamIn, std::ostream *cxxStreamOut, const size_t bufferSize, const NGenXXZFormatX format)
{
    if (bufferSize <= 0)
    {
        return false;
    }
    if (cxxStreamIn == NULL || cxxStreamOut == NULL)
    {
        return false;
    }
    return NGenXX::Z::unzip(bufferSize, static_cast<int>(format), cxxStreamIn, cxxStreamOut);
}

const Bytes ngenxxZBytesZip(const Bytes &inBytes, const NGenXXZipCompressModeX mode, const size_t bufferSize, const NGenXXZFormatX format)
{
    if (bufferSize <= 0)
    {
        return BytesEmpty;
    }
    if (inBytes.empty())
    {
        return BytesEmpty;
    }
    return NGenXX::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), inBytes);
}

const Bytes ngenxxZBytesUnzip(const Bytes &inBytes, const size_t bufferSize, const NGenXXZFormatX format)
{
    if (bufferSize <= 0)
    {
        return BytesEmpty;
    }
    if (inBytes.empty())
    {
        return BytesEmpty;
    }
    return NGenXX::Z::unzip(bufferSize, static_cast<int>(format), inBytes);
}