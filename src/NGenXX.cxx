#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

#include "log/Log.hxx"
#include "net/HttpClient.hxx"
#include "store/SQLite.hxx"
#include "store/KV.hxx"
#include "util/JsonUtil.hxx"
#include "NGenXX-inner.hxx"
#ifdef USE_LUA
#include "NGenXX-Lua.hxx"
#endif

#define VERSION "0.0.1"

// WARNING: Export with `EMSCRIPTEN_KEEPALIVE` will cause Lua running automatically.
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define EXPORT_WASM extern "C" EMSCRIPTEN_KEEPALIVE
#define EXPORT_WASM_LUA extern "C"
#endif

static inline const char *str2charp(std::string s)
{
    const char *c = s.c_str();
    char *nc = (char *)malloc(strlen(c) + 1);
    strcpy(nc, c);
    return nc;
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
const char *ngenxx_get_version(void)
{
    auto s = std::string(VERSION);
    return str2charp(s);
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
void *ngenxx_init(const char *root)
{
    if (root == NULL)
        return NULL;
    NGenXX::Net::HttpClient::create();
    NGenXXHandle *h = (NGenXXHandle *)malloc(sizeof(NGenXXHandle));
    h->sqlite = new NGenXX::Store::SQLite();
    h->kv = new NGenXX::Store::KV(std::string(root));
#ifdef USE_LUA
    _ngenxx_lua_init(h);
#endif
    return h;
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM_LUA
#endif
void ngenxx_release(void *sdk)
{
    NGenXX::Net::HttpClient::destroy();
    if (sdk == NULL)
        return;
    NGenXXHandle *h = (NGenXXHandle *)sdk;
    delete ((NGenXX::Store::SQLite *)h->sqlite);
    delete ((NGenXX::Store::KV *)h->kv);
#ifdef USE_LUA
    _ngenxx_lua_release(h);
#endif
    free(sdk);
}

#pragma mark Log

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_log_set_level(int level)
{
    NGenXX::Log::setLevel(level);
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_log_set_callback(void (*callback)(int level, const char *log))
{
    NGenXX::Log::setCallback(callback);
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_log_print(int level, const char *content)
{
    NGenXX::Log::print(level, content);
}

#pragma mark Net.Http

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
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

    auto s = NGenXX::Net::HttpClient::request(sUrl, sParams, method, vHeaders, timeout);
    return str2charp(s);
}

#pragma mark Store.SQLite

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void *ngenxx_store_sqlite_open(void *sdk, const char *file)
{
    if (sdk == NULL || file == NULL)
        return NULL;
    NGenXX::Store::SQLite *sqlite = (NGenXX::Store::SQLite *)(((NGenXXHandle *)sdk)->sqlite);
    return sqlite->connect(std::string(file));
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
bool ngenxx_store_sqlite_execute(void *conn, const char *sql)
{
    if (conn == NULL || sql == NULL)
        return false;
    NGenXX::Store::SQLite::Connection *xconn = (NGenXX::Store::SQLite::Connection *)conn;
    return xconn->execute(std::string(sql));
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void *ngenxx_store_sqlite_query_do(void *conn, const char *sql)
{
    if (conn == NULL || sql == NULL)
        return NULL;
    NGenXX::Store::SQLite::Connection *xconn = (NGenXX::Store::SQLite::Connection *)conn;
    return xconn->query(std::string(sql));
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
bool ngenxx_store_sqlite_query_read_row(void *query_result)
{
    if (query_result == NULL)
        return false;
    NGenXX::Store::SQLite::Connection::QueryResult *xqr = (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
    return xqr->readRow();
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
const char *ngenxx_store_sqlite_query_read_column_text(void *query_result, const char *column)
{
    if (query_result == NULL || column == NULL)
        return NULL;
    NGenXX::Store::SQLite::Connection::QueryResult *xqr = (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
    auto s = xqr->readColumnText(std::string(column));
    return str2charp(s);
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
long long ngenxx_store_sqlite_query_read_column_integer(void *query_result, const char *column)
{
    if (query_result == NULL || column == NULL)
        return 0;
    NGenXX::Store::SQLite::Connection::QueryResult *xqr = (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
    return xqr->readColumnInteger(std::string(column));
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
double ngenxx_store_sqlite_query_read_column_float(void *query_result, const char *column)
{
    if (query_result == NULL || column == NULL)
        return 0.f;
    NGenXX::Store::SQLite::Connection::QueryResult *xqr = (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
    return xqr->readColumnFloat(std::string(column));
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_store_sqlite_query_drop(void *query_result)
{
    if (query_result == NULL)
        return;
    delete (NGenXX::Store::SQLite::Connection::QueryResult *)query_result;
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_store_sqlite_close(void *conn)
{
    if (conn == NULL)
        return;
    delete (NGenXX::Store::SQLite::Connection *)conn;
}

#pragma mark Store.KV

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void *ngenxx_store_kv_open(void *sdk, const char *_id)
{
    if (sdk == NULL || _id == NULL)
        return NULL;
    return ((NGenXX::Store::KV *)(((NGenXXHandle *)sdk)->kv))->open(std::string(_id));
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
const char *ngenxx_store_kv_read_string(void *conn, const char *k)
{
    if (conn == NULL || k == NULL)
        return NULL;
    return str2charp(((NGenXX::Store::KV::Connection *)conn)->readString(std::string(k)));
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
bool ngenxx_store_kv_write_string(void *conn, const char *k, const char *v)
{
    if (conn == NULL || k == NULL)
        return false;
    return ((NGenXX::Store::KV::Connection *)conn)->write(std::string(k), v);
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
long long ngenxx_store_kv_read_integer(void *conn, const char *k)
{
    if (conn == NULL || k == NULL)
        return 0;
    return ((NGenXX::Store::KV::Connection *)conn)->readInteger(std::string(k));
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
bool ngenxx_store_kv_write_integer(void *conn, const char *k, long long v)
{
    if (conn == NULL || k == NULL)
        return false;
    return ((NGenXX::Store::KV::Connection *)conn)->write(std::string(k), (int64_t)v);
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
double ngenxx_store_kv_read_float(void *conn, const char *k)
{
    if (conn == NULL || k == NULL)
        return 0;
    return ((NGenXX::Store::KV::Connection *)conn)->readFloat(std::string(k));
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
bool ngenxx_store_kv_write_float(void *conn, const char *k, double v)
{
    if (conn == NULL || k == NULL)
        return false;
    return ((NGenXX::Store::KV::Connection *)conn)->write(std::string(k), v);
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
bool ngenxx_store_kv_contains(void *conn, const char *k)
{
    if (conn == NULL || k == NULL)
        return false;
    return ((NGenXX::Store::KV::Connection *)conn)->contains(std::string(k));
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_store_kv_clear(void *conn)
{
    if (conn == NULL)
        return;
    ((NGenXX::Store::KV::Connection *)conn)->clear();
}

#ifdef __EMSCRIPTEN__
EXPORT_WASM
#endif
void ngenxx_store_kv_close(void *conn)
{
    if (conn == NULL)
        return;
    delete (NGenXX::Store::KV *)conn;
}