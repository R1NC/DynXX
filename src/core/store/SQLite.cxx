#if defined(USE_DB)

#include "SQLite.hxx"

#include <DynXX/CXX/Log.hxx>

namespace
{
    using enum DynXXLogLevelX;
    
    #define PRINT_ERR(rc, db) dynxxLogPrint(Error, std::string(db ? sqlite3_errmsg(db) : sqlite3_errstr(rc)))

    constexpr auto sEnableWAL = "PRAGMA journal_mode=WAL;";
}

DynXX::Core::Store::SQLite::SQLiteStore::SQLiteStore()
{
    sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    sqlite3_initialize();
}

std::weak_ptr<DynXX::Core::Store::SQLite::Connection> DynXX::Core::Store::SQLite::SQLiteStore::open(const std::string &file)
{
    return ConnPool<SQLite::Connection>::open(file, [&file]() {
        sqlite3 *db;
        if (const auto rc = sqlite3_open_v2(file.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr); rc != SQLITE_OK) [[unlikely]] {
            PRINT_ERR(rc, db);
            if (db) sqlite3_close(db);
            return std::shared_ptr<SQLite::Connection>(nullptr);
        }
        return std::make_shared<SQLite::Connection>(file, db);
    });
}

void DynXX::Core::Store::SQLite::SQLiteStore::close(const DictKeyType _id)
{
    ConnPool<SQLite::Connection>::close(_id);
}

DynXX::Core::Store::SQLite::SQLiteStore::~SQLiteStore()
{
    sqlite3_shutdown();
}

DynXX::Core::Store::SQLite::Connection::Connection(const std::string &_id, sqlite3 *db) : _id(_id), db{db}
{
    if (!this->execute(sEnableWAL)) [[unlikely]]
    {
        dynxxLogPrint(Warn, "SQLite enable WAL failed");
    }
}

bool DynXX::Core::Store::SQLite::Connection::execute(std::string_view sql) const
{
    auto lock = std::scoped_lock(this->mutex);
    if (this->db == nullptr) [[unlikely]]
    {
        dynxxLogPrint(Error, "SQLite.execute DB nullptr");
        return false;
    }
    
    const auto sqlOp = nullTerminatedCStr(sql);
    if (!sqlOp.has_value()) [[unlikely]]
    {
        dynxxLogPrint(Error, "SQLite.execute SQL not null terminated");
        return false;
    }
    
    const auto rc = sqlite3_exec(this->db, sqlOp.value(), nullptr, nullptr, nullptr);
    // dynxxLogPrintF(Debug, "SQLite.exec ret:{}", rc);
    if (rc != SQLITE_OK)
    {
        PRINT_ERR(rc, this->db);
    }
    
    return rc == SQLITE_OK;
}

std::unique_ptr<DynXX::Core::Store::SQLite::Connection::QueryResult> DynXX::Core::Store::SQLite::Connection::query(std::string_view sql) const
{
    auto lock = std::scoped_lock(this->mutex);
    if (this->db == nullptr) [[unlikely]]
    {
        dynxxLogPrint(Error, "SQLite.query DB nullptr");
        return nullptr;
    }
    sqlite3_stmt *stmt;

    const auto sqlOp = nullTerminatedCStr(sql);
    if (!sqlOp.has_value()) [[unlikely]]
    {
        dynxxLogPrint(Error, "SQLite.query SQL not null terminated");
        return nullptr;
    }

    const auto rc = sqlite3_prepare_v2(this->db, sqlOp.value(), static_cast<int>(sql.size()), &stmt, nullptr);
    // dynxxLogPrintF(Debug, "SQLite.query ret:{}", rc);
    if (rc != SQLITE_OK)
    {
        PRINT_ERR(rc, this->db);
        return nullptr;
    }

    return std::make_unique<DynXX::Core::Store::SQLite::Connection::QueryResult>(stmt);
}

DynXX::Core::Store::SQLite::Connection::~Connection()
{
    if (this->db != nullptr) [[likely]]
    {
        sqlite3_close_v2(this->db);
    }
}

DynXX::Core::Store::SQLite::Connection::QueryResult::QueryResult(sqlite3_stmt *stmt) : stmt{stmt}
{
}

bool DynXX::Core::Store::SQLite::Connection::QueryResult::readRow() const
{
    auto lock = std::unique_lock(this->mutex);
    if (this->stmt == nullptr) [[unlikely]]
    {
        dynxxLogPrint(Error, "SQLite.readRow STMT nullptr");
        return false;
    }
    const auto rc = sqlite3_step(this->stmt);
    // dynxxLogPrintF(Debug, "SQLite.step ret:{}", rc);
    if (rc != SQLITE_ROW && rc != SQLITE_DONE)
    {
        PRINT_ERR(rc, nullptr);
    }
    return rc == SQLITE_ROW;
}

std::optional<Any> DynXX::Core::Store::SQLite::Connection::QueryResult::readColumn(std::string_view column) const
{
    auto lock = std::shared_lock(this->mutex);
    if (this->stmt == nullptr) [[unlikely]]
    {
        dynxxLogPrint(Error, "SQLite.readColumn STMT nullptr");
        return std::nullopt;
    }
    auto colCount = sqlite3_column_count(this->stmt);
    for (decltype(colCount) i(0); i < colCount; i++)
    {
        const auto columnOp = nullTerminatedCStr(column);
        if (!columnOp.has_value()) [[unlikely]]
        {
            dynxxLogPrint(Error, "SQLite.readColumn column not null terminated");
            return std::nullopt;
        }
        if (strcmp(sqlite3_column_name(this->stmt, i), columnOp.value()) != 0)
        {
            continue;
        }
        switch(sqlite3_column_type(this->stmt, i))
        {
            case SQLITE_TEXT:
                return {makeStr(sqlite3_column_text(this->stmt, i))};
            case SQLITE_INTEGER:
                return {sqlite3_column_int64(this->stmt, i)};
            case SQLITE_FLOAT:
                return {sqlite3_column_double(this->stmt, i)};
            default:
                return std::nullopt;
        }
    }
    return std::nullopt;
}

std::optional<Any> DynXX::Core::Store::SQLite::Connection::QueryResult::operator[](std::string_view column) const {
    return this->readColumn(column);
}

DynXX::Core::Store::SQLite::Connection::QueryResult::~QueryResult()
{
    if (this->stmt != nullptr) [[likely]]
    {
        sqlite3_finalize(this->stmt);
    }
}

#endif
