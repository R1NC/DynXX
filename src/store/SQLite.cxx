#include "SQLite.hxx"
#include "../../include/NGenXXLog.hxx"
#include <string.h>

#define PRINT_ERR(rc, db) ngenxxLogPrint(NGenXXLogLevelX::Error, std::string(db ? sqlite3_errmsg(db) : sqlite3_errstr(rc)))

NGenXX::Store::SQLite::SQLite()
{
    sqlite3_initialize();
}

NGenXX::Store::SQLite::Connection *NGenXX::Store::SQLite::connect(const std::string &file)
{
    const std::lock_guard<std::mutex> lock(this->mutex);
    auto conn = this->conns[file];
    if (conn == nullptr)
    {
        sqlite3 *db;
        int rc = sqlite3_open_v2(file.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
        ngenxxLogPrint(NGenXXLogLevelX::Debug, "SQLite.open ret:" + std::to_string(rc));
        if (rc != SQLITE_OK)
        {
            PRINT_ERR(rc, db);
            return nullptr;
        }
        conn = new NGenXX::Store::SQLite::Connection(db);
        this->conns[file] = conn;
    }
    return conn;
}

void NGenXX::Store::SQLite::closeAll()
{
    const std::lock_guard<std::mutex> lock(this->mutex);
    for (auto& it : this->conns)
    {
        delete it.second;
        it.second = nullptr;
    }
    this->conns.clear();
}

NGenXX::Store::SQLite::~SQLite()
{
    sqlite3_shutdown();
}

NGenXX::Store::SQLite::Connection::Connection(sqlite3 *db) : db{db}
{
    this->execute("PRAGMA journal_mode=WAL;");
}

bool NGenXX::Store::SQLite::Connection::execute(const std::string &sql)
{
    std::unique_lock lock(this->mutex);
    if (this->db == NULL)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "SQLite.execute DB NULL");
        return false;
    }
    sqlite3_stmt *stmt;
    int rc = sqlite3_exec(this->db, sql.c_str(), NULL, NULL, NULL);
    ngenxxLogPrint(NGenXXLogLevelX::Debug, "SQLite.exec ret:" + std::to_string(rc));
    if (rc != SQLITE_OK)
    {
        PRINT_ERR(rc, this->db);
    }
    return rc == SQLITE_OK;
}

NGenXX::Store::SQLite::Connection::QueryResult *NGenXX::Store::SQLite::Connection::query(const std::string &sql)
{
    std::shared_lock lock(this->mutex);
    if (this->db == NULL)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "SQLite.query DB NULL");
        return NULL;
    }
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(this->db, sql.c_str(), -1, &stmt, NULL);
    ngenxxLogPrint(NGenXXLogLevelX::Debug, "SQLite.query ret:" + std::to_string(rc));
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

NGenXX::Store::SQLite::Connection::QueryResult::QueryResult(sqlite3_stmt *stmt) : stmt{stmt}
{
}

bool NGenXX::Store::SQLite::Connection::QueryResult::readRow()
{
    std::unique_lock lock(this->mutex);
    if (this->stmt == NULL)
    {
         ngenxxLogPrint(NGenXXLogLevelX::Error, "SQLite.readRow STMT NULL");
         return false;
    }
    int rc = sqlite3_step(this->stmt);
    ngenxxLogPrint(NGenXXLogLevelX::Debug, "SQLite.step ret:" + std::to_string(rc));
    if (rc != SQLITE_ROW && rc != SQLITE_DONE)
    {
        PRINT_ERR(rc, NULL);
    }
    return rc == SQLITE_ROW;
}

Any NGenXX::Store::SQLite::Connection::QueryResult::readColumn(const std::string &column)
{
    std::shared_lock lock(this->mutex);
    if (this->stmt == NULL)
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "SQLite.readColumn STMT NULL");
        return {};
    }
    for (int i = 0; i < sqlite3_column_count(this->stmt); i++)
    {
        if (strcmp(sqlite3_column_name(this->stmt, i), column.c_str()) == 0)
        {
            int columnType = sqlite3_column_type(this->stmt, i);
            if (columnType == SQLITE_TEXT)
            {
                return std::string((const char *)sqlite3_column_text(this->stmt, i));
            }
            else if (columnType == SQLITE_INTEGER)
            {
                return sqlite3_column_int64(this->stmt, i);
            }
            else if (columnType == SQLITE_FLOAT)
            {
                return sqlite3_column_double(this->stmt, i);
            }
        }
    }
    return {};
}

NGenXX::Store::SQLite::Connection::QueryResult::~QueryResult()
{
    if (this->stmt != NULL)
        sqlite3_finalize(this->stmt);
}
