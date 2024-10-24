#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>
#include <memory>

#include "../include/NGenXX.h"
#include "log/Log.hxx"
#include "crypto/Crypto.hxx"
#include "coding/Coding.hxx"
#include "device/DeviceInfo.hxx"
#include "net/HttpClient.hxx"
#include "store/SQLite.hxx"
#include "store/KV.hxx"
#include "json/JsonDecoder.hxx"
#include "zip/Zip.hxx"
#include "util/TypeUtil.hxx"
#include "NGenXX-inner.hxx"
#ifdef USE_LUA
#include "NGenXX-Lua.hxx"
#endif
#ifdef USE_QJS
#include "NGenXX-JS.hxx"
#endif

#define VERSION "0.0.1"

std::shared_ptr<NGenXX::Net::HttpClient> _ngenxx_http_client = nullptr;
std::shared_ptr<NGenXX::Store::SQLite> _ngenxx_sqlite = nullptr;
std::shared_ptr<NGenXX::Store::KV> _ngenxx_kv = nullptr;
std::shared_ptr<const std::string> _ngenxx_root = nullptr;

EXPORT_AUTO
const char *ngenxx_get_version(void)
{
    auto s = std::string(VERSION);
    return str2charp(s);
}

EXPORT
bool ngenxx_init(const char *root)
{
    if (_ngenxx_root != nullptr || root == NULL)
        return false;
    _ngenxx_root = std::make_shared<std::string>(root);
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

EXPORT_AUTO
const char *ngenxx_root_path()
{
    if (_ngenxx_root == nullptr)
        return NULL;
    return str2charp(*_ngenxx_root);
}

EXPORT
void ngenxx_release()
{
    if (_ngenxx_root == nullptr)
        return;
    _ngenxx_root.reset();
    _ngenxx_http_client.reset();
    _ngenxx_sqlite.reset();
    _ngenxx_kv.reset();
#ifdef USE_LUA
    _ngenxx_lua_release();
#endif
#ifdef USE_QJS
    _ngenxx_js_release();
#endif
}

#pragma mark Device.DeviceInfo

EXPORT_AUTO
int ngenxx_device_type()
{
    return NGenXX::Device::DeviceInfo::deviceType();
}

EXPORT_AUTO
const char *ngenxx_device_name()
{
    return str2charp(NGenXX::Device::DeviceInfo::deviceName());
}

EXPORT_AUTO
const char *ngenxx_device_manufacturer()
{
    return str2charp(NGenXX::Device::DeviceInfo::deviceManufacturer());
}

EXPORT_AUTO
const char *ngenxx_device_os_version()
{
    return str2charp(NGenXX::Device::DeviceInfo::osVersion());
}

EXPORT_AUTO
int ngenxx_device_cpu_arch()
{
    return NGenXX::Device::DeviceInfo::cpuArch();
}

#pragma mark Log

EXPORT_AUTO
void ngenxx_log_set_level(const int level)
{
    NGenXX::Log::setLevel(level);
}

EXPORT_AUTO
void ngenxx_log_set_callback(void (*callback)(const int level, const char *content))
{
    NGenXX::Log::setCallback(callback);
}

EXPORT_AUTO
void ngenxx_log_print(const int level, const char *content)
{
    if (!content)
        return;
    NGenXX::Log::print(level, std::string(content));
}

#pragma mark Net.Http

EXPORT_AUTO
const char *ngenxx_net_http_request(const char *url, const char *params, const int method,
                                    const char **header_v, const size header_c,
                                    const char **form_field_name_v,
                                    const char **form_field_mime_v,
                                    const char **form_field_data_v,
                                    const size form_field_count,
                                    void *const cFILE, const size file_size,
                                    const size timeout)
{
    if (_ngenxx_http_client == nullptr || url == NULL)
        return NULL;
    const std::string sUrl(url);
    const std::string sParams(params ?: "");
    std::vector<std::string> vHeaders;
    if (header_v != NULL && header_c > 0)
    {
        vHeaders = std::vector<std::string>(header_v, header_v + header_c);
    }
    std::vector<NGenXX::Net::HttpFormField> vFormFields;
    if (form_field_count > 0 && form_field_name_v != NULL && form_field_mime_v != NULL && form_field_data_v != NULL)
    {
        for (int i = 0; i < form_field_count; i++)
        {
            NGenXX::Net::HttpFormField form_field = {
                .name = std::string(form_field_name_v[i]),
                .mime = std::string(form_field_mime_v[i]),
                .data = std::string(form_field_data_v[i])};
            vFormFields.push_back(form_field);
        }
    }

    auto s = _ngenxx_http_client->request(sUrl, sParams, method, vHeaders, vFormFields,
                                          reinterpret_cast<std::FILE *>(cFILE),
                                          file_size, timeout);
    return str2charp(s);
}

#pragma mark Store.SQLite

EXPORT_AUTO
void *const ngenxx_store_sqlite_open(const char *_id)
{
    if (_ngenxx_sqlite == nullptr || _ngenxx_root == nullptr || _id == NULL)
        return NULL;
    std::string dbFile = *_ngenxx_root + "/" + std::string(_id) + ".db";
    return _ngenxx_sqlite->connect(dbFile);
}

EXPORT_AUTO
bool ngenxx_store_sqlite_execute(void *const conn, const char *sql)
{
    if (conn == NULL || sql == NULL)
        return false;
    auto xconn = reinterpret_cast<NGenXX::Store::SQLite::Connection *>(conn);
    return xconn->execute(std::string(sql));
}

EXPORT_AUTO
void *const ngenxx_store_sqlite_query_do(void *const conn, const char *sql)
{
    if (conn == NULL || sql == NULL)
        return NULL;
    auto xconn = reinterpret_cast<NGenXX::Store::SQLite::Connection *>(conn);
    return xconn->query(std::string(sql));
}

EXPORT_AUTO
bool ngenxx_store_sqlite_query_read_row(void *const query_result)
{
    if (query_result == NULL)
        return false;
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    return xqr->readRow();
}

EXPORT_AUTO
const char *ngenxx_store_sqlite_query_read_column_text(void *const query_result, const char *column)
{
    if (query_result == NULL || column == NULL)
        return NULL;
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    auto a = xqr->readColumn(std::string(column));
    return str2charp(*std::get_if<std::string>(&a));
}

EXPORT_AUTO
long long ngenxx_store_sqlite_query_read_column_integer(void *const query_result, const char *column)
{
    if (query_result == NULL || column == NULL)
        return 0;
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    auto a = xqr->readColumn(std::string(column));
    return *std::get_if<int64_t>(&a);
}

EXPORT_AUTO
double ngenxx_store_sqlite_query_read_column_float(void *const query_result, const char *column)
{
    if (query_result == NULL || column == NULL)
        return 0.f;
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    auto a = xqr->readColumn(std::string(column));
    return *std::get_if<double>(&a);
}

EXPORT_AUTO
void ngenxx_store_sqlite_query_drop(void *const query_result)
{
    if (query_result == NULL)
        return;
    auto xqr = reinterpret_cast<NGenXX::Store::SQLite::Connection::QueryResult *>(query_result);
    delete xqr;
}

EXPORT_AUTO
void ngenxx_store_sqlite_close(void *const conn)
{
    if (conn == NULL)
        return;
    auto xconn = reinterpret_cast<NGenXX::Store::SQLite::Connection *>(conn);
    delete xconn;
}

#pragma mark Store.KV

EXPORT_AUTO
void *const ngenxx_store_kv_open(const char *_id)
{
    if (_ngenxx_kv == nullptr || _id == NULL)
        return NULL;
    return _ngenxx_kv->open(std::string(_id));
}

EXPORT_AUTO
const char *ngenxx_store_kv_read_string(void *const conn, const char *k)
{
    if (conn == NULL || k == NULL)
        return NULL;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    auto s = xconn->readString(std::string(k));
    return str2charp(s);
}

EXPORT_AUTO
bool ngenxx_store_kv_write_string(void *const conn, const char *k, const char *v)
{
    if (conn == NULL || k == NULL)
        return false;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(std::string(k), std::string(v));
}

EXPORT_AUTO
long long ngenxx_store_kv_read_integer(void *const conn, const char *k)
{
    if (conn == NULL || k == NULL)
        return 0;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->readInteger(std::string(k));
}

EXPORT_AUTO
bool ngenxx_store_kv_write_integer(void *const conn, const char *k, long long v)
{
    if (conn == NULL || k == NULL)
        return false;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(std::string(k), v);
}

EXPORT_AUTO
double ngenxx_store_kv_read_float(void *const conn, const char *k)
{
    if (conn == NULL || k == NULL)
        return 0;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->readFloat(std::string(k));
}

EXPORT_AUTO
bool ngenxx_store_kv_write_float(void *const conn, const char *k, double v)
{
    if (conn == NULL || k == NULL)
        return false;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->write(std::string(k), v);
}

EXPORT_AUTO
bool ngenxx_store_kv_contains(void *const conn, const char *k)
{
    if (conn == NULL || k == NULL)
        return false;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    return xconn->contains(std::string(k));
}

EXPORT_AUTO
void ngenxx_store_kv_remove(void *const conn, const char *k)
{
    if (conn == NULL || k == NULL)
        return;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    xconn->remove(std::string(k));
}

EXPORT_AUTO
void ngenxx_store_kv_clear(void *const conn)
{
    if (conn == NULL)
        return;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    xconn->clear();
}

EXPORT_AUTO
void ngenxx_store_kv_close(void *const conn)
{
    if (conn == NULL)
        return;
    auto xconn = reinterpret_cast<NGenXX::Store::KV::Connection *>(conn);
    delete xconn;
}

#pragma mark Coding

EXPORT_AUTO
const byte *ngenxx_coding_hex_str2bytes(const char *str, size *outLen)
{
    auto t = NGenXX::Coding::Hex::str2bytes(std::string(str));
    *outLen = t.second;
    return copyBytes(t);
}

EXPORT_AUTO
const char *ngenxx_coding_hex_bytes2str(const byte *inBytes, const size inLen)
{
    auto s = NGenXX::Coding::Hex::bytes2str({inBytes, inLen});
    return str2charp(s);
}

#pragma mark Crypto

EXPORT_AUTO
bool ngenxx_crypto_rand(const unsigned long len, byte *bytes)
{
    return NGenXX::Crypto::rand(len, bytes);
}

EXPORT_AUTO
const byte *ngenxx_crypto_aes_encrypt(const byte *inBytes, const size inLen, const byte *keyBytes, const size keyLen, size *outLen)
{
    auto t = NGenXX::Crypto::AES::encrypt({inBytes, inLen}, {keyBytes, keyLen});
    if (outLen)
        *outLen = t.second;
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_crypto_aes_decrypt(const byte *inBytes, const size inLen, const byte *keyBytes, const size keyLen, size *outLen)
{
    auto t = NGenXX::Crypto::AES::decrypt({inBytes, inLen}, {keyBytes, keyLen});
    if (outLen)
        *outLen = t.second;
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_crypto_aes_gcm_encrypt(const byte *inBytes, const size inLen,
                                          const byte *keyBytes, const size keyLen,
                                          const byte *initVectorBytes, const size initVectorLen,
                                          const byte *aadBytes, const size aadLen,
                                          const size tagBits, size *outLen)
{
    auto t = NGenXX::Crypto::AES::gcmEncrypt({inBytes, inLen}, {keyBytes, keyLen}, {initVectorBytes, initVectorLen}, {aadBytes, aadLen}, tagBits);
    if (outLen)
        *outLen = t.second;
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_crypto_aes_gcm_decrypt(const byte *inBytes, const size inLen,
                                          const byte *keyBytes, const size keyLen,
                                          const byte *initVectorBytes, const size initVectorLen,
                                          const byte *aadBytes, const size aadLen,
                                          const size tagBits, size *outLen)
{
    auto t = NGenXX::Crypto::AES::gcmDecrypt({inBytes, inLen}, {keyBytes, keyLen}, {initVectorBytes, initVectorLen}, {aadBytes, aadLen}, tagBits);
    if (outLen)
        *outLen = t.second;
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_crypto_hash_md5(const byte *inBytes, const size inLen, size *outLen)
{
    auto t = NGenXX::Crypto::Hash::md5({inBytes, inLen});
    if (outLen)
        *outLen = t.second;
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_crypto_hash_sha256(const byte *inBytes, const size inLen, size *outLen)
{
    auto t = NGenXX::Crypto::Hash::sha256({inBytes, inLen});
    if (outLen)
        *outLen = t.second;
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_crypto_base64_encode(const byte *inBytes, const size inLen, size *outLen)
{
    auto t = NGenXX::Crypto::Base64::encode({inBytes, inLen});
    if (outLen)
        *outLen = t.second;
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_crypto_base64_decode(const byte *inBytes, const size inLen, size *outLen)
{
    auto t = NGenXX::Crypto::Base64::decode({inBytes, inLen});
    if (outLen)
        *outLen = t.second;
    return copyBytes(t);
}

#pragma mark Json.Decoder

EXPORT_AUTO
void *const ngenxx_json_decoder_init(const char *json)
{
    return json ? new NGenXX::Json::Decoder(std::string(json)) : NULL;
}

EXPORT_AUTO
bool ngenxx_json_decoder_is_array(void *const decoder, void *const node)
{
    if (decoder == NULL)
        return false;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->isArray(node);
}

EXPORT_AUTO
bool ngenxx_json_decoder_is_object(void *const decoder, void *const node)
{
    if (decoder == NULL)
        return false;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->isObject(node);
}

EXPORT_AUTO
void *const ngenxx_json_decoder_read_node(void *const decoder, void *const node, const char *k)
{
    if (decoder == NULL || k == NULL)
        return NULL;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNode(node, std::string(k));
}

EXPORT_AUTO
const char *ngenxx_json_decoder_read_string(void *const decoder, void *const node)
{
    if (decoder == NULL)
        return NULL;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return str2charp(xdecoder->readString(node));
}

EXPORT_AUTO
double ngenxx_json_decoder_read_number(void *const decoder, void *const node)
{
    if (decoder == NULL)
        return 0;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNumber(node);
}

EXPORT_AUTO
void *const ngenxx_json_decoder_read_child(void *const decoder, void *const node)
{
    if (decoder == NULL)
        return NULL;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readChild(node);
}

EXPORT_AUTO
void *const ngenxx_json_decoder_read_next(void *const decoder, void *const node)
{
    if (decoder == NULL)
        return NULL;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    return xdecoder->readNext(node);
}

EXPORT_AUTO
void ngenxx_json_decoder_release(void *const decoder)
{
    if (decoder == NULL)
        return;
    auto xdecoder = reinterpret_cast<NGenXX::Json::Decoder *>(decoder);
    delete xdecoder;
}

#pragma mark Zip

EXPORT_AUTO
void *const ngenxx_z_zip_init(const int mode, const size bufferSize, const int format)
{
    void *zip = NULL;
    try
    {
        zip = new NGenXX::Z::Zip(mode, bufferSize, format);
    }
    catch (const std::exception &e)
    {
        NGenXX::Log::print(NGenXXLogLevelError, "ngenxx_z_zip_init failed");
    }
    return zip;
}

EXPORT_AUTO
const size ngenxx_z_zip_input(void *const zip, const byte *inBytes, const size inLen, const bool inFinish)
{
    if (zip == NULL)
        return 0;
    auto xzip = reinterpret_cast<NGenXX::Z::Zip *>(zip);
    return xzip->input({inBytes, inLen}, inFinish);
}

EXPORT_AUTO
const byte *ngenxx_z_zip_process_do(void *const zip, size *outLen)
{
    if (zip == NULL)
        return NULL;
    auto xzip = reinterpret_cast<NGenXX::Z::Zip *>(zip);
    auto t = xzip->processDo();
    if (outLen)
        *outLen = t.second;
    return copyBytes(t);
}

EXPORT_AUTO
const bool ngenxx_z_zip_process_finished(void *const zip)
{
    if (zip == NULL)
        return false;
    auto xzip = reinterpret_cast<NGenXX::Z::Zip *>(zip);
    return xzip->processFinished();
}

EXPORT_AUTO
void ngenxx_z_zip_release(void *const zip)
{
    if (zip == NULL)
        return;
    auto xzip = reinterpret_cast<NGenXX::Z::Zip *>(zip);
    delete xzip;
}

EXPORT_AUTO
void *const ngenxx_z_unzip_init(const size bufferSize, const int format)
{
    void *zip = NULL;
    try
    {
        zip = new NGenXX::Z::UnZip(bufferSize, format);
    }
    catch (const std::exception &e)
    {
        NGenXX::Log::print(NGenXXLogLevelError, "ngenxx_z_unzip_init failed");
    }
    return zip;
}

EXPORT_AUTO
const size ngenxx_z_unzip_input(void *const unzip, const byte *inBytes, const size inLen, const bool inFinish)
{
    if (unzip == NULL)
        return 0;
    auto xunzip = reinterpret_cast<NGenXX::Z::UnZip *>(unzip);
    return xunzip->input({inBytes, inLen}, inFinish);
}

EXPORT_AUTO
const byte *ngenxx_z_unzip_process_do(void *const unzip, size *outLen)
{
    if (unzip == NULL)
        return NULL;
    auto xunzip = reinterpret_cast<NGenXX::Z::UnZip *>(unzip);
    auto t = xunzip->processDo();
    if (outLen)
        *outLen = t.second;
    return copyBytes(t);
}

EXPORT_AUTO
const bool ngenxx_z_unzip_process_finished(void *const unzip)
{
    if (unzip == NULL)
        return false;
    auto xunzip = reinterpret_cast<NGenXX::Z::UnZip *>(unzip);
    return xunzip->processFinished();
}

EXPORT_AUTO
void ngenxx_z_unzip_release(void *const unzip)
{
    if (unzip == NULL)
        return;
    auto xunzip = reinterpret_cast<NGenXX::Z::UnZip *>(unzip);
    delete xunzip;
}

EXPORT_AUTO
bool ngenxx_z_cfile_zip(const int mode, const size bufferSize, const int format, void *const cFILEIn, void *const cFILEOut)
{
    if (mode != NGenXXZipCompressModeDefault && mode != NGenXXZipCompressModePreferSize && mode != NGenXXZipCompressModePreferSpeed)
        return false;
    if (bufferSize <= 0)
        return false;
    if (cFILEIn == NULL || cFILEOut == NULL)
        return false;
    return NGenXX::Z::zip(mode, bufferSize, format,
                          reinterpret_cast<std::FILE *>(cFILEIn), reinterpret_cast<std::FILE *>(cFILEOut));
}

EXPORT_AUTO
bool ngenxx_z_cfile_unzip(const size bufferSize, const int format, void *const cFILEIn, void *const cFILEOut)
{
    if (bufferSize <= 0)
        return false;
    if (cFILEIn == NULL || cFILEOut == NULL)
        return false;
    return NGenXX::Z::unzip(bufferSize, format,
                            reinterpret_cast<std::FILE *>(cFILEIn), reinterpret_cast<std::FILE *>(cFILEOut));
}

EXPORT_AUTO
bool ngenxx_z_cxxstream_zip(const int mode, const size bufferSize, const int format, void *const cxxStreamIn, void *const cxxStreamOut)
{
    if (mode != NGenXXZipCompressModeDefault && mode != NGenXXZipCompressModePreferSize && mode != NGenXXZipCompressModePreferSpeed)
        return false;
    if (bufferSize <= 0)
        return false;
    if (cxxStreamIn == NULL || cxxStreamOut == NULL)
        return false;
    return NGenXX::Z::zip(mode, bufferSize, format,
                          reinterpret_cast<std::istream *>(cxxStreamIn), reinterpret_cast<std::ostream *>(cxxStreamOut));
}

EXPORT_AUTO
bool ngenxx_z_cxxstream_unzip(const size bufferSize, const int format, void *const cxxStreamIn, void *const cxxStreamOut)
{
    if (bufferSize <= 0)
        return false;
    if (cxxStreamIn == NULL || cxxStreamOut == NULL)
        return false;
    return NGenXX::Z::unzip(bufferSize, format,
                            reinterpret_cast<std::istream *>(cxxStreamIn), reinterpret_cast<std::ostream *>(cxxStreamOut));
}

EXPORT_AUTO
const byte *ngenxx_z_bytes_zip(const int mode, const size bufferSize, const int format, const byte *inBytes, const size inLen, size *outLen)
{
    if (mode != NGenXXZipCompressModeDefault && mode != NGenXXZipCompressModePreferSize && mode != NGenXXZipCompressModePreferSpeed)
        return NULL;
    if (bufferSize <= 0)
        return NULL;
    if (inBytes == NULL || inLen <= 0 || outLen == NULL)
        return NULL;
    auto t = NGenXX::Z::zip(mode, bufferSize, format, {inBytes, inLen});
    *outLen = t.second;
    return copyBytes(t);
}

EXPORT_AUTO
const byte *ngenxx_z_bytes_unzip(const size bufferSize, const int format, const byte *inBytes, const size inLen, size *outLen)
{
    if (bufferSize <= 0)
        return NULL;
    if (inBytes == NULL || inLen <= 0 || outLen == NULL)
        return NULL;
    auto t = NGenXX::Z::unzip(bufferSize, format, {inBytes, inLen});
    *outLen = t.second;
    return copyBytes(t);
}