#include "DB.hxx"
#include "../../include/NGenXXLog.h"
#include "../log/Log.hxx"

NGenXX::Store::DB::DB(const std::string &file)
{
    int rc = sqlite3_open(file.c_str(), &this->db);
    if (rc != SQLITE_OK)
    {
        NGenXX::Log::print(Error, sqlite3_errstr(rc));
    }
}

NGenXX::Store::DB::QueryResult *NGenXX::Store::DB::execute(const std::string &sql)
{
    if (this->db == NULL)
        return NULL;
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(this->db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        NGenXX::Log::print(Error, sqlite3_errstr(rc));
        return nullptr;
    }
    return new NGenXX::Store::DB::QueryResult(stmt);
}

NGenXX::Store::DB::~DB()
{
    if (this->db != NULL)
        sqlite3_close(this->db);
}

NGenXX::Store::DB::QueryResult::QueryResult(sqlite3_stmt *stmt)
{
    this->stmt = stmt;
}

bool NGenXX::Store::DB::QueryResult::readRow()
{
    if (this->stmt == NULL)
        return false;
    return sqlite3_step(this->stmt) == SQLITE_ROW;
}

std::string NGenXX::Store::DB::QueryResult::readColumnText(const std::string &column)
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

long long NGenXX::Store::DB::QueryResult::readColumnInteger(const std::string &column)
{
    if (this->stmt == NULL)
        return NULL;
    for (int i = 0; i < sqlite3_column_count(this->stmt); i++)
    {
        if (strcmp(sqlite3_column_name(this->stmt, i), column.c_str()) == 0 && sqlite3_column_type(this->stmt, i) == SQLITE_INTEGER)
        {
            return sqlite3_column_int64(this->stmt, i);
        }
    }
    return 0;
}

double NGenXX::Store::DB::QueryResult::readColumnFloat(const std::string &column)
{
    if (this->stmt == NULL)
        return NULL;
    for (int i = 0; i < sqlite3_column_count(this->stmt); i++)
    {
        if (strcmp(sqlite3_column_name(this->stmt, i), column.c_str()) == 0 && sqlite3_column_type(this->stmt, i) == SQLITE_FLOAT)
        {
            return sqlite3_column_double(this->stmt, i);
        }
    }
    return 0;
}

NGenXX::Store::DB::QueryResult::~QueryResult()
{
    if (this->stmt != NULL)
        sqlite3_finalize(this->stmt);
}