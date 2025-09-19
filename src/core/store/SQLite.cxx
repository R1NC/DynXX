#if defined(USE_DB)

#include "SQLite.hxx"

#include <DynXX/CXX/Log.hxx>

namespace
{
    using enum DynXXLogLevelX;
    
    #define PRINT_ERR(rc, db) dynxxLogPrint(Error, std::string(db ? sqlite3_errmsg(db) : sqlite3_errstr(rc)))

    constexpr auto sEnableWAL = "PRAGMA journal_mode=WAL;";
}

namespace DynXX::Core::Store::SQLite {

SQLiteStore::SQLiteStore()
{
    sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    sqlite3_initialize();
}

std::weak_ptr<Connection> SQLiteStore::open(const std::string &file)
{
    const auto cid = genCid(file);
    return ConnPool<Connection>::open(cid, [cid, &file]() {
        sqlite3 *db;
        if (const auto rc = sqlite3_open_v2(file.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr); rc != SQLITE_OK || db == nullptr) [[unlikely]] {
            PRINT_ERR(rc, db);
            if (db) sqlite3_close(db);
            return std::shared_ptr<Connection>(nullptr);
        }
        return std::make_shared<Connection>(cid, db);
    });
}

SQLiteStore::~SQLiteStore()
{
    sqlite3_shutdown();
}

Connection::Connection(const CidT cid, sqlite3 *db) : _cid(cid), db{db}
{
    if (!this->execute(sEnableWAL)) [[unlikely]]
    {
        dynxxLogPrint(Warn, "SQLite enable WAL failed");
    }
}

bool Connection::execute(std::string_view sql) const
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
    if (rc != SQLITE_OK)
    {
        PRINT_ERR(rc, this->db);
    }
    
    return rc == SQLITE_OK;
}

std::unique_ptr<Connection::QueryResult> Connection::query(std::string_view sql) const
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

    if (const auto rc = sqlite3_prepare_v2(this->db, sqlOp.value(), static_cast<int>(sql.size()), &stmt, nullptr); rc != SQLITE_OK)
    {
        PRINT_ERR(rc, this->db);
        return nullptr;
    }

    return std::make_unique<Connection::QueryResult>(stmt);
}

Connection::~Connection()
{
    if (this->db != nullptr) [[likely]]
    {
        sqlite3_close_v2(this->db);
    }
}

Connection::QueryResult::QueryResult(sqlite3_stmt *stmt) : stmt{stmt}
{
}

bool Connection::QueryResult::readRow() const
{
    auto lock = std::unique_lock(this->mutex);
    if (this->stmt == nullptr) [[unlikely]]
    {
        dynxxLogPrint(Error, "SQLite.readRow STMT nullptr");
        return false;
    }
    const auto rc = sqlite3_step(this->stmt);
    if (rc != SQLITE_ROW && rc != SQLITE_DONE)
    {
        PRINT_ERR(rc, nullptr);
    }
    return rc == SQLITE_ROW;
}

std::optional<Any> Connection::QueryResult::readColumn(std::string_view column) const
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
                {
                    const auto data = sqlite3_column_text(this->stmt, i);
                    const auto len = sqlite3_column_bytes(this->stmt, i);
                    return {makeStr(data, len)};
                }
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

std::optional<Any> Connection::QueryResult::operator[](std::string_view column) const {
    return this->readColumn(column);
}

Connection::QueryResult::~QueryResult()
{
    if (this->stmt != nullptr) [[likely]]
    {
        sqlite3_finalize(this->stmt);
    }
}

} // namespace DynXX::Core::Store::SQLite

#endif
