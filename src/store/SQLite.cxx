#include "SQLite.hxx"

#include <cstring>

#include <NGenXXLog.hxx>

#define PRINT_ERR(rc, db) ngenxxLogPrint(NGenXXLogLevelX::Error, std::string(db ? sqlite3_errmsg(db) : sqlite3_errstr(rc)))

NGenXX::Store::SQLite::SQLite()
{
    sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    sqlite3_initialize();
}

NGenXX::Store::SQLite::Connection *NGenXX::Store::SQLite::connect(const std::string &file)
{
    auto lock = std::lock_guard(this->mutex);
    auto conn = this->conns.at(file).get();
    if (conn == nullptr)
    {
        sqlite3 *db;
        auto rc = sqlite3_open_v2(file.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
        // ngenxxLogPrintF(NGenXXLogLevelX::Debug, "SQLite.open ret:{}", rc);
        if (rc != SQLITE_OK) [[unlikely]]
        {
            PRINT_ERR(rc, db);
            return nullptr;
        }
        
        auto upConn = std::make_unique<Connection>(db);
        this->conns.emplace(file, std::move(upConn));
        conn = upConn.get();
    }
    return conn;
}

void NGenXX::Store::SQLite::close(const std::string &file)
{
    auto lock = std::lock_guard(this->mutex);
    auto it = this->conns.find(file);
    if (it != this->conns.end())
    {
        it->second.reset();
        this->conns.erase(it);
    }
}

void NGenXX::Store::SQLite::closeAll()
{
    auto lock = std::lock_guard(this->mutex);
    for (auto &it : this->conns)
    {
        it.second.reset();
    }
    this->conns.clear();
}

NGenXX::Store::SQLite::~SQLite()
{
    sqlite3_shutdown();
}

NGenXX::Store::SQLite::Connection::Connection(sqlite3 *db) : db{db}
{
    constexpr const char* sEnableWAL = "PRAGMA journal_mode=WAL;";
    if (!this->execute(sEnableWAL)) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Warn, "SQLite enable WAL failed");
    }
}

bool NGenXX::Store::SQLite::Connection::execute(const std::string &sql) const
{
    auto lock = std::lock_guard(this->mutex);
    if (this->db == NULL) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "SQLite.execute DB NULL");
        return false;
    }
    sqlite3_stmt *stmt;
    auto rc = sqlite3_exec(this->db, sql.c_str(), NULL, NULL, NULL);
    // ngenxxLogPrintF(NGenXXLogLevelX::Debug, "SQLite.exec ret:{}", rc);
    if (rc != SQLITE_OK)
    {
        PRINT_ERR(rc, this->db);
    }
    return rc == SQLITE_OK;
}

NGenXX::Store::SQLite::Connection::QueryResult *NGenXX::Store::SQLite::Connection::query(const std::string &sql) const
{
    auto lock = std::lock_guard(this->mutex);
    if (this->db == NULL) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "SQLite.query DB NULL");
        return NULL;
    }
    sqlite3_stmt *stmt;
    auto rc = sqlite3_prepare_v2(this->db, sql.c_str(), -1, &stmt, NULL);
    // ngenxxLogPrintF(NGenXXLogLevelX::Debug, "SQLite.query ret:{}", rc);
    if (rc != SQLITE_OK)
    {
        PRINT_ERR(rc, this->db);
        return nullptr;
    }
    return new NGenXX::Store::SQLite::Connection::QueryResult(stmt);
}

NGenXX::Store::SQLite::Connection::~Connection()
{
    if (this->db != NULL) [[likely]]
    {
        sqlite3_close_v2(this->db);
    }
}

NGenXX::Store::SQLite::Connection::QueryResult::QueryResult(sqlite3_stmt *stmt) : stmt{stmt}
{
}

bool NGenXX::Store::SQLite::Connection::QueryResult::readRow() const
{
    auto lock = std::unique_lock(this->mutex);
    if (this->stmt == NULL) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "SQLite.readRow STMT NULL");
        return false;
    }
    auto rc = sqlite3_step(this->stmt);
    // ngenxxLogPrintF(NGenXXLogLevelX::Debug, "SQLite.step ret:{}", rc);
    if (rc != SQLITE_ROW && rc != SQLITE_DONE)
    {
        PRINT_ERR(rc, NULL);
    }
    return rc == SQLITE_ROW;
}

Any NGenXX::Store::SQLite::Connection::QueryResult::readColumn(const std::string &column) const
{
    auto lock = std::shared_lock(this->mutex);
    if (this->stmt == NULL) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "SQLite.readColumn STMT NULL");
        return AnyEmpty;
    }
    auto colCount = sqlite3_column_count(this->stmt);
    for (decltype(colCount) i(0); i < colCount; i++)
    {
        if (strcmp(sqlite3_column_name(this->stmt, i), column.c_str()) == 0)
        {
            auto columnType = sqlite3_column_type(this->stmt, i);
            if (columnType == SQLITE_TEXT)
            {
                return reinterpret_cast<const char *>(sqlite3_column_text(this->stmt, i));
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
    return AnyEmpty;
}

NGenXX::Store::SQLite::Connection::QueryResult::~QueryResult()
{
    if (this->stmt != NULL) [[likely]]
    {
        sqlite3_finalize(this->stmt);
    }
}
