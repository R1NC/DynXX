#include "SQLite.hxx"
#include "../../include/NGenXXLog.h"
#include "../log/Log.hxx"
#include <string.h>

#define PRINT_ERR(rc, db) NGenXX::Log::print(NGenXXLogLevelError, db ? sqlite3_errmsg(db) : sqlite3_errstr(rc))

NGenXX::Store::SQLite::SQLite()
{
    sqlite3_initialize();
}

NGenXX::Store::SQLite::Connection *NGenXX::Store::SQLite::connect(const std::string &file)
{
    sqlite3 *db;
    int rc = sqlite3_open_v2(file.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    if (rc != SQLITE_OK)
    {
        PRINT_ERR(rc, db);
        return nullptr;
    }
    return new NGenXX::Store::SQLite::Connection(db);
}

NGenXX::Store::SQLite::~SQLite()
{
    sqlite3_shutdown();
}

NGenXX::Store::SQLite::Connection::Connection(sqlite3 *db)
{
    this->db = db;
}

bool NGenXX::Store::SQLite::Connection::execute(const std::string &sql)
{
    if (this->db == NULL)
        return false;
    sqlite3_stmt *stmt;
    int rc = sqlite3_exec(this->db, sql.c_str(), NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        PRINT_ERR(rc, this->db);
    }
    return rc == SQLITE_OK;
}

NGenXX::Store::SQLite::Connection::QueryResult *NGenXX::Store::SQLite::Connection::query(const std::string &sql)
{
    if (this->db == NULL)
        return NULL;
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(this->db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        PRINT_ERR(rc, this->db);
        return nullptr;
    }
    return new NGenXX::Store::SQLite::Connection::QueryResult(stmt);
}

NGenXX::Store::SQLite::Connection::~Connection()
{
    if (this->db != NULL)
        sqlite3_close_v2(this->db);
}

NGenXX::Store::SQLite::Connection::QueryResult::QueryResult(sqlite3_stmt *stmt)
{
    this->stmt = stmt;
}

bool NGenXX::Store::SQLite::Connection::QueryResult::readRow()
{
    if (this->stmt == NULL)
        return false;
    int rc = sqlite3_step(this->stmt);
    if (rc != SQLITE_ROW)
    {
        PRINT_ERR(rc, NULL);
    }
    return rc == SQLITE_ROW;
}

std::string NGenXX::Store::SQLite::Connection::QueryResult::readColumnText(const std::string &column)
{
    if (this->stmt == NULL)
        return NULL;
    for (int i = 0; i < sqlite3_column_count(this->stmt); i++)
    {
        if (strcmp(sqlite3_column_name(this->stmt, i), column.c_str()) == 0 && sqlite3_column_type(this->stmt, i) == SQLITE_TEXT)
        {
            return (const char *)sqlite3_column_text(this->stmt, i);
        }
    }
    return NULL;
}

long long NGenXX::Store::SQLite::Connection::QueryResult::readColumnInteger(const std::string &column)
{
    if (this->stmt == NULL)
        return 0;
    for (int i = 0; i < sqlite3_column_count(this->stmt); i++)
    {
        if (strcmp(sqlite3_column_name(this->stmt, i), column.c_str()) == 0 && sqlite3_column_type(this->stmt, i) == SQLITE_INTEGER)
        {
            return sqlite3_column_int64(this->stmt, i);
        }
    }
    return 0;
}

double NGenXX::Store::SQLite::Connection::QueryResult::readColumnFloat(const std::string &column)
{
    if (this->stmt == NULL)
        return 0.f;
    for (int i = 0; i < sqlite3_column_count(this->stmt); i++)
    {
        if (strcmp(sqlite3_column_name(this->stmt, i), column.c_str()) == 0 && sqlite3_column_type(this->stmt, i) == SQLITE_FLOAT)
        {
            return sqlite3_column_double(this->stmt, i);
        }
    }
    return 0;
}

NGenXX::Store::SQLite::Connection::QueryResult::~QueryResult()
{
    if (this->stmt != NULL)
        sqlite3_finalize(this->stmt);
}
