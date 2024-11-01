#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus

#include <vector>
#include <string>
#include <sstream>
#include <memory>

#include "../include/NGenXX.hxx"
#include "net/HttpClient.hxx"
#include "store/SQLite.hxx"
#include "store/KV.hxx"
#include "json/JsonDecoder.hxx"
#include "zip/Zip.hxx"
#include "coding/Coding.hxx"
#include "crypto/Crypto.hxx"
#include "device/DeviceInfo.hxx"
#include "log/Log.hxx"
#ifdef USE_LUA
#include "NGenXX-Lua.hxx"
#endif
#ifdef USE_QJS
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
    return std::string(VERSION);
}

const std::string ngenxxRootPath()
{
    if (!_ngenxx_root)
        return NULL;
    return std::string(*_ngenxx_root);
}

bool ngenxxInit(const std::string &root)
{
    if (_ngenxx_root != nullptr)
        return true;
    if (root.length() == 0)
        return false;
    _ngenxx_root = std::make_shared<const std::string>(root);
    _ngenxx_sqlite = std::make_shared<NGenXX::Store::SQLite>();
    _ngenxx_kv = std::make_shared<NGenXX::Store::KV>(*_ngenxx_root);
    _ngenxx_http_client = std::make_shared<NGenXX::Net::HttpClient>();
#ifdef USE_LUA
    _ngenxx_lua_init();
#endif
#ifdef USE_QJS
    _ngenxx_js_init();
#endif
    return true;
}

void ngenxxRelease()
{
    if (_ngenxx_root == nullptr)
        return;
    _ngenxx_root.reset();
    _ngenxx_http_client.reset();
    _ngenxx_sqlite.reset();
    _ngenxx_kv.reset();
    ngenxxLogSetCallback(nullptr);
#ifdef USE_LUA
    _ngenxx_lua_release();
#endif
#ifdef USE_QJS
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

const std::string ngenxxCodingHexBytes2str(const Bytes bytes)
{
    return NGenXX::Coding::Hex::bytes2str(bytes);
}

const Bytes ngenxxCodingHexStr2bytes(const std::string &str)
{
    return NGenXX::Coding::Hex::str2bytes(str);
}

#pragma mark Crypto

bool ngenxxCryptoRand(const size_t len, byte *bytes)
{
    return NGenXX::Crypto::rand(len, bytes);
}

const Bytes ngenxxCryptoAesEncrypt(const Bytes in, const Bytes key)
{
    return NGenXX::Crypto::AES::encrypt(in, key);
}

const Bytes ngenxxCryptoAesDecrypt(const Bytes in, const Bytes key)
{
    return NGenXX::Crypto::AES::decrypt(in, key);
}

const Bytes ngenxxCryptoAesGcmEncrypt(const Bytes in, const Bytes key, const Bytes initVector, const Bytes aad, const size_t tagBits)
{
    return NGenXX::Crypto::AES::gcmEncrypt(in, key, initVector, aad, tagBits);
}

const Bytes ngenxxCryptoAesGcmDecrypt(const Bytes in, const Bytes key, const Bytes initVector, const Bytes aad, const size_t tagBits)
{
    return NGenXX::Crypto::AES::gcmDecrypt(in, key, initVector, aad, tagBits);
}

const Bytes ngenxxCryptoHashMd5(const Bytes in)
{
    return NGenXX::Crypto::Hash::md5(in);
}

const Bytes ngenxxCryptoHashSha256(const Bytes in)
{
    return NGenXX::Crypto::Hash::sha256(in);
}

const Bytes ngenxxCryptoBase64Encode(const Bytes in)
{
    return NGenXX::Crypto::Base64::encode(in);
}

const Bytes ngenxxCryptoBase64Decode(const Bytes in)
{
    return NGenXX::Crypto::Base64::decode(in);
}

#pragma mark Net.Http

const std::string ngenxxNetHttpRequest(const std::string &url,
                                       const std::string &params,
                                       const NGenXXHttpMethodX method,
                                       const std::vector<std::string> &headerV,
                                       const std::vector<std::string> &formFieldNameV,
                                       const std::vector<std::string> &formFieldMimeV,
                                       const std::vector<std::string> &formFieldDataV,
                                       const std::FILE *cFILE, const size_t fileSize,
                                       const size_t timeout)
{
    std::string s;
    if (_ngenxx_http_client == nullptr || url.length() == 0)
        return s;

    std::vector<NGenXX::Net::HttpFormField> vFormFields;
    for (int i = 0; i < formFieldNameV.size() && i < formFieldMimeV.size() && i < formFieldDataV.size(); i++)
    {
        vFormFields.push_back({
            .name = formFieldNameV[i],
            .mime = formFieldMimeV[i],
            .data = formFieldDataV[i],
        });
    }

    return _ngenxx_http_client->request(url, params, static_cast<int>(method), headerV, vFormFields, cFILE, fileSize, timeout);
}

const std::string ngenxxNetHttpRequest(const std::string &url,
                                       const std::unordered_map<std::string, Any> &params,
                                       const NGenXXHttpMethodX method,
                                       const std::unordered_map<std::string, std::string> &headers,
                                       const std::vector<std::string> &formFieldNameV,
                                       const std::vector<std::string> &formFieldMimeV,
                                       const std::vector<std::string> &formFieldDataV,
                                       const std::FILE *cFILE, const size_t fileSize,
                                       const size_t timeout)
{
    std::stringstream ssParams;
    for (auto it = params.begin(); it != params.end(); ++it)
    {
        if (ssParams.str().length() > 0)
            ssParams << "&";
        ssParams << it->first << "=";
        std::visit(
            [&ssParams](auto &x)
            { 
                ssParams << x; 
            }, 
            it->second
        );
    }
    std::vector<std::string> headerV;
    return ngenxxNetHttpRequest(url, ssParams.str(), method, headerV,
                                formFieldNameV, formFieldMimeV, formFieldDataV,
                                cFILE, fileSize, timeout);
}

#pragma mark Store.SQLite

void *const ngenxxStoreSqliteOpen(const std::string &_id)
{
    if (_ngenxx_sqlite == nullptr || _ngenxx_root == nullptr || _id.length() == 0)
        return NULL;
    std::string dbFile = *_ngenxx_root + "/" + std::string(_id) + ".db";
    return _ngenxx_sqlite->connect(dbFile);
}

bool ngenxxStoreSqliteExecute(void *const conn, const std::string &sql)
{
    if (conn == NULL || sql.length() == 0)
        return false;
    auto xconn = reinterpret_cast<NGenXX::Store::SQLite::Connection *>(conn);
    return xconn->execute(sql);
}

void *const ngenxxStoreSqliteQueryDo(void *const conn, const std::string &sql)
{
    if (conn == NULL || sql.length() == 0)
        return NULL;
    auto xconn = reinterpret_cast<NGenXX::Store::SQLite::Connection *>(conn);
    return xconn->query(sql);
}

bool ngenxxStoreSqliteQueryReadRow(void *const query_result)
{
    if (query_result == NULL)
        return false;
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    return xqr->readRow();
}

const std::string ngenxxStoreSqliteQueryReadColumnText(void *const query_result, const std::string &column)
{
    if (query_result == NULL || column.length() == 0)
        return NULL;
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    auto a = xqr->readColumn(column);
    return *std::get_if<std::string>(&a);
}

long long ngenxxStoreSqliteQueryReadColumnInteger(void *const query_result, const std::string &column)
{
    if (query_result == NULL || column.length() == 0)
        return 0;
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    auto a = xqr->readColumn(column);
    return *std::get_if<int64_t>(&a);
}

double ngenxxStoreSqliteQueryReadColumnFloat(void *const query_result, const std::string &column)
{
    if (query_result == NULL || column.length() == 0)
        return 0.f;
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    auto a = xqr->readColumn(column);
    return *std::get_if<double>(&a);
}

void ngenxxStoreSqliteQueryDrop(void *const query_result)
{
    if (query_result == NULL)
        return;
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    delete xqr;
}

void ngenxxStoreSqliteClose(void *const conn)
{
    if (conn == NULL)
        return;
    auto xconn = reinterpret_cast<NGenXX::Store::SQLite::Connection *>(conn);
    delete xconn;
}

#pragma mark Store.KV

void *const ngenxxStoreKvOpen(const std::string &_id)
{
    if (_ngenxx_kv == nullptr || _id.length() == 0)
        return NULL;
    return _ngenxx_kv->open(_id);
}

const std::string ngenxxStoreKvReadString(void *const conn, const std::string &k)
{
    std::string s;
    if (conn == NULL || k.length() == 0)
        return s;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->readString(k);
}

bool ngenxxStoreKvWriteString(void *const conn, const std::string &k, const std::string &v)
{
    if (conn == NULL || k.length() == 0)
        return false;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

long long ngenxxStoreKvReadInteger(void *const conn, const std::string &k)
{
    if (conn == NULL || k.length() == 0)
        return 0;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->readInteger(k);
}

bool ngenxxStoreKvWriteInteger(void *const conn, const std::string &k, long long v)
{
    if (conn == NULL || k.length() == 0)
        return false;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

double ngenxxStoreKvReadFloat(void *const conn, const std::string &k)
{
    if (conn == NULL || k.length() == 0)
        return 0;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->readFloat(k);
}

bool ngenxxStoreKvWriteFloat(void *const conn, const std::string &k, double v)
{
    if (conn == NULL || k.length() == 0)
        return false;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(k, v);
}

const std::vector<std::string> ngenxxStoreKvAllKeys(void *const conn)
{
    std::vector<std::string> v;
    if (conn == NULL)
        return v;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->allKeys();
}

bool ngenxxStoreKvContains(void *const conn, const std::string &k)
{
    if (conn == NULL || k.length() == 0)
        return false;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->contains(k);
}

void ngenxxStoreKvRemove(void *const conn, const std::string &k)
{
    if (conn == NULL || k.length() == 0)
        return;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    xconn->remove(k);
}

void ngenxxStoreKvClear(void *const conn)
{
    if (conn == NULL)
        return;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    xconn->clear();
}

void ngenxxStoreKvClose(void *const conn)
{
    if (conn == NULL)
        return;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    delete xconn;
}

#pragma mark Json.Decoder

void *const ngenxxJsonDecoderInit(const std::string &json)
{
    return new NGenXX::Json::Decoder(json);
}

bool ngenxxJsonDecoderIsArray(void *const decoder, void *const node)
{
    if (decoder == NULL)
        return false;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->isArray(node);
}

bool ngenxxJsonDecoderIsObject(void *const decoder, void *const node)
{
    if (decoder == NULL)
        return false;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->isObject(node);
}

void *const ngenxxJsonDecoderReadNode(void *const decoder, void *const node, const std::string &k)
{
    if (decoder == NULL || k.length() == 0)
        return NULL;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNode(node, k);
}

const std::string ngenxxJsonDecoderReadString(void *const decoder, void *const node)
{
    if (decoder == NULL)
        return NULL;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readString(node);
}

double ngenxxJsonDecoderReadNumber(void *const decoder, void *const node)
{
    if (decoder == NULL)
        return 0;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNumber(node);
}

void *const ngenxxJsonDecoderReadChild(void *const decoder, void *const node)
{
    if (decoder == NULL)
        return NULL;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readChild(node);
}

void *const ngenxxJsonDecoderReadNext(void *const decoder, void *const node)
{
    if (decoder == NULL)
        return NULL;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNext(node);
}

void ngenxxJsonDecoderRelease(void *const decoder)
{
    if (decoder == NULL)
        return;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    delete xdecoder;
}

#pragma mark Zip

void *const ngenxxZZipInit(const NGenXXZipCompressModeX mode, const size_t bufferSize, const NGenXXZFormatX format)
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

const size_t ngenxxZZipInput(void *const zip, const Bytes inBytes, const bool inFinish)
{
    if (zip == NULL)
        return 0;
    auto xzip = reinterpret_cast<NGenXX::Z::Zip *>(zip);
    return xzip->input(inBytes, inFinish);
}

const Bytes ngenxxZZipProcessDo(void *const zip)
{
    if (zip == NULL)
        return BytesEmpty;
    auto xzip = reinterpret_cast<NGenXX::Z::Zip *>(zip);
    return xzip->processDo();
}

const bool ngenxxZZipProcessFinished(void *const zip)
{
    if (zip == NULL)
        return false;
    auto xzip = reinterpret_cast<NGenXX::Z::Zip *>(zip);
    return xzip->processFinished();
}

void ngenxxZZipRelease(void *const zip)
{
    if (zip == NULL)
        return;
    auto xzip = reinterpret_cast<NGenXX::Z::Zip *>(zip);
    delete xzip;
}

void *const ngenxxZUnzipInit(const size_t bufferSize, const NGenXXZFormatX format)
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

const size_t ngenxxZUnzipInput(void *const unzip, const Bytes inBytes, const bool inFinish)
{
    if (unzip == NULL)
        return 0;
    auto xunzip = reinterpret_cast<NGenXX::Z::UnZip *>(unzip);
    return xunzip->input(inBytes, inFinish);
}

const Bytes ngenxxZUnzipProcessDo(void *const unzip)
{
    if (unzip == NULL)
        return BytesEmpty;
    auto xunzip = reinterpret_cast<NGenXX::Z::UnZip *>(unzip);
    return xunzip->processDo();
}

const bool ngenxxZUnzipProcessFinished(void *const unzip)
{
    if (unzip == NULL)
        return false;
    auto xunzip = reinterpret_cast<NGenXX::Z::UnZip *>(unzip);
    return xunzip->processFinished();
}

void ngenxxZUnzipRelease(void *const unzip)
{
    if (unzip == NULL)
        return;
    auto xunzip = reinterpret_cast<NGenXX::Z::UnZip *>(unzip);
    delete xunzip;
}

bool ngenxxZCFileZip(const NGenXXZipCompressModeX mode, const size_t bufferSize, const NGenXXZFormatX format, std::FILE *cFILEIn, std::FILE *cFILEOut)
{
    if (bufferSize <= 0)
        return false;
    if (cFILEIn == NULL || cFILEOut == NULL)
        return false;
    return NGenXX::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), cFILEIn, cFILEOut);
}

bool ngenxxZCFileUnzip(const size_t bufferSize, const NGenXXZFormatX format, std::FILE *cFILEIn, std::FILE *cFILEOut)
{
    if (bufferSize <= 0)
        return false;
    if (cFILEIn == NULL || cFILEOut == NULL)
        return false;
    return NGenXX::Z::unzip(bufferSize, static_cast<int>(format), cFILEIn, cFILEOut);
}

bool ngenxxZCxxStreamZip(const NGenXXZipCompressModeX mode, const size_t bufferSize, const NGenXXZFormatX format, std::istream *cxxStreamIn, std::ostream *cxxStreamOut)
{
    if (bufferSize <= 0)
        return false;
    if (cxxStreamIn == NULL || cxxStreamOut == NULL)
        return false;
    return NGenXX::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), cxxStreamIn, cxxStreamOut);
}

bool ngenxxZCxxStreamUnzip(const size_t bufferSize, const NGenXXZFormatX format, std::istream *cxxStreamIn, std::ostream *cxxStreamOut)
{
    if (bufferSize <= 0)
        return false;
    if (cxxStreamIn == NULL || cxxStreamOut == NULL)
        return false;
    return NGenXX::Z::unzip(bufferSize, static_cast<int>(format), cxxStreamIn, cxxStreamOut);
}

const Bytes ngenxxZBytesZip(const NGenXXZipCompressModeX mode, const size_t bufferSize, const NGenXXZFormatX format, const Bytes inBytes)
{
    if (bufferSize <= 0)
        return BytesEmpty;
    if (inBytes.first == NULL || inBytes.second <= 0)
        return BytesEmpty;
    return NGenXX::Z::zip(static_cast<int>(mode), bufferSize, static_cast<int>(format), inBytes);
}

const Bytes ngenxxZBytesUnzip(const size_t bufferSize, const NGenXXZFormatX format, const Bytes inBytes)
{
    if (bufferSize <= 0)
        return BytesEmpty;
    if (inBytes.first == NULL || inBytes.second <= 0)
        return BytesEmpty;
    return NGenXX::Z::unzip(bufferSize, static_cast<int>(format), inBytes);
}