#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "log/Log.hxx"
#include "device/DeviceInfo.hxx"
#include "net/HttpClient.hxx"
#include "store/SQLite.hxx"
#include "store/KV.hxx"
#include "util/JsonUtil.hxx"
#include "NGenXX-inner.hxx"
#ifdef USE_LUA
#include "NGenXX-Lua.hxx"
#endif

#define VERSION "0.0.1"

NGenXXHandle *_ngenxx_handle;

EXPORT_AUTO
const char *ngenxx_get_version(void)
{
    auto s = std::string(VERSION);
    return str2charp(s);
}

EXPORT
bool ngenxx_init(const char *root)
{
    if (_ngenxx_handle != NULL) return true;
    if (root == NULL)
        return false;
    _ngenxx_handle = (NGenXXHandle *)malloc(sizeof(NGenXXHandle));
    _ngenxx_handle->sqlite = new NGenXX::Store::SQLite();
    _ngenxx_handle->kv = new NGenXX::Store::KV(std::string(root));
    _ngenxx_handle->http_client = new NGenXX::Net::HttpClient();
#ifdef USE_LUA
    _ngenxx_lua_init();
#endif
    return true;
}

EXPORT
void ngenxx_release()
{
    if (_ngenxx_handle == NULL)
        return;
    delete (NGenXX::Net::HttpClient*)(_ngenxx_handle->http_client);
    _ngenxx_handle->http_client = NULL;
    delete ((NGenXX::Store::SQLite *)_ngenxx_handle->sqlite);
    _ngenxx_handle->sqlite = NULL;
    delete ((NGenXX::Store::KV *)_ngenxx_handle->kv);
    _ngenxx_handle->kv = NULL;
#ifdef USE_LUA
    _ngenxx_lua_release();
#endif
    free(_ngenxx_handle);
    _ngenxx_handle = NULL;
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
void ngenxx_log_set_level(int level)
{
    NGenXX::Log::setLevel(level);
}

EXPORT_AUTO
void ngenxx_log_set_callback(void (*callback)(int level, const char *log))
{
    NGenXX::Log::setCallback(callback);
}

EXPORT_AUTO
void ngenxx_log_print(int level, const char *content)
{
    NGenXX::Log::print(level, content);
}

#pragma mark Net.Http

EXPORT_AUTO
const char *ngenxx_net_http_request(const char *url, const char *params, int method, char **headers_v, int headers_c, long timeout)
{
    if (url == NULL)
        return NULL;
    const std::string sUrl(url);
    const std::string sParams(params);
    std::vector<std::string> vHeaders;
    if (headers_v != NULL && headers_c > 0)
    {
        vHeaders = std::vector<std::string>(headers_v, headers_v + headers_c);
    }

    auto s = ((NGenXX::Net::HttpClient *)(_ngenxx_handle->http_client))->request(sUrl, sParams, method, vHeaders, timeout);
    return str2charp(s);
}

#pragma mark Store.SQLite

EXPORT_AUTO
void *ngenxx_store_sqlite_open(const char *file)
{
    if (_ngenxx_handle == NULL || file == NULL)
        return NULL;
    NGenXX::Store::SQLite *sqlite = (NGenXX::Store::SQLite *)(((NGenXXHandle *)_ngenxx_handle)->sqlite);
    return sqlite->connect(std::string(file));
}

EXPORT_AUTO
bool ngenxx_store_sqlite_execute(void *conn, const char *sql)
{
    if (conn == NULL || sql == NULL)
        return false;
    NGenXX::Store::SQLite::Connection *xconn = (NGenXX::Store::SQLite::Connection *)conn;
    return xconn->execute(std::string(sql));
}

EXPORT_AUTO
void *ngenxx_store_sqlite_query_do(void *conn, const char *sql)
{
    if (conn == NULL || sql == NULL)
        return NULL;
    NGenXX::Store::SQLite::Connection *xconn = (NGenXX::Store::SQLite::Connection *)conn;
    return xconn->query(std::string(sql));
}

EXPORT_AUTO
bool ngenxx_store_sqlite_query_read_row(void *query_result)
{
    if (query_result == NULL)
        return false;
    NGenXX::Store::SQLite::Connection::QueryResult *xqr = (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
    return xqr->readRow();
}

EXPORT_AUTO
const char *ngenxx_store_sqlite_query_read_column_text(void *query_result, const char *column)
{
    if (query_result == NULL || column == NULL)
        return NULL;
    NGenXX::Store::SQLite::Connection::QueryResult *xqr = (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
    auto s = xqr->readColumnText(std::string(column));
    return str2charp(s);
}

EXPORT_AUTO
long long ngenxx_store_sqlite_query_read_column_integer(void *query_result, const char *column)
{
    if (query_result == NULL || column == NULL)
        return 0;
    NGenXX::Store::SQLite::Connection::QueryResult *xqr = (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
    return xqr->readColumnInteger(std::string(column));
}

EXPORT_AUTO
double ngenxx_store_sqlite_query_read_column_float(void *query_result, const char *column)
{
    if (query_result == NULL || column == NULL)
        return 0.f;
    NGenXX::Store::SQLite::Connection::QueryResult *xqr = (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
    return xqr->readColumnFloat(std::string(column));
}

EXPORT_AUTO
void ngenxx_store_sqlite_query_drop(void *query_result)
{
    if (query_result == NULL)
        return;
    delete (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
}

EXPORT_AUTO
void ngenxx_store_sqlite_close(void *conn)
{
    if (conn == NULL)
        return;
    delete (NGenXX::Store::SQLite::Connection *)conn;
}

#pragma mark Store.KV

EXPORT_AUTO
void *ngenxx_store_kv_open(const char *_id)
{
    if (_ngenxx_handle == NULL || _id == NULL)
        return NULL;
    return ((NGenXX::Store::KV *)(((NGenXXHandle *)_ngenxx_handle)->kv))->open(std::string(_id));
}

EXPORT_AUTO
const char *ngenxx_store_kv_read_string(void *conn, const char *k)
{
    if (conn == NULL || k == NULL)
        return NULL;
    return str2charp(((NGenXX::Store::KV::Connection *)conn)->readString(std::string(k)));
}

EXPORT_AUTO
bool ngenxx_store_kv_write_string(void *conn, const char *k, const char *v)
{
    if (conn == NULL || k == NULL)
        return false;
    return ((NGenXX::Store::KV::Connection *)conn)->write(std::string(k), v);
}

EXPORT_AUTO
long long ngenxx_store_kv_read_integer(void *conn, const char *k)
{
    if (conn == NULL || k == NULL)
        return 0;
    return ((NGenXX::Store::KV::Connection *)conn)->readInteger(std::string(k));
}

EXPORT_AUTO
bool ngenxx_store_kv_write_integer(void *conn, const char *k, long long v)
{
    if (conn == NULL || k == NULL)
        return false;
    return ((NGenXX::Store::KV::Connection *)conn)->write(std::string(k), (int64_t)v);
}

EXPORT_AUTO
double ngenxx_store_kv_read_float(void *conn, const char *k)
{
    if (conn == NULL || k == NULL)
        return 0;
    return ((NGenXX::Store::KV::Connection *)conn)->readFloat(std::string(k));
}

EXPORT_AUTO
bool ngenxx_store_kv_write_float(void *conn, const char *k, double v)
{
    if (conn == NULL || k == NULL)
        return false;
    return ((NGenXX::Store::KV::Connection *)conn)->write(std::string(k), v);
}

EXPORT_AUTO
bool ngenxx_store_kv_contains(void *conn, const char *k)
{
    if (conn == NULL || k == NULL)
        return false;
    return ((NGenXX::Store::KV::Connection *)conn)->contains(std::string(k));
}

EXPORT_AUTO
void ngenxx_store_kv_clear(void *conn)
{
    if (conn == NULL)
        return;
    ((NGenXX::Store::KV::Connection *)conn)->clear();
}

EXPORT_AUTO
void ngenxx_store_kv_close(void *conn)
{
    if (conn == NULL)
        return;
    delete (NGenXX::Store::KV *)conn;
}