#if defined(USE_DB)

#include "SQLite.hxx"

#include <DynXX/CXX/Log.hxx>

namespace
{
    using enum DynXXLogLevelX;
    using namespace DynXX::Core::Store::SQLite;

    constexpr auto sEnableWAL = "PRAGMA journal_mode=WAL;";

    constexpr auto PRINT_ERR = [](auto rc, auto db) {
        dynxxLogPrint(Error, std::string(db != nullptr ? sqlite3_errmsg(db) : sqlite3_errstr(rc)));
    };

    std::shared_ptr<sqlite3> createDB(std::string_view file) {
        std::shared_ptr<sqlite3> db{nullptr};
        const auto flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
        const auto fileS = std::string(file);
        const auto filename = fileS.c_str();

#if defined(__cpp_lib_out_ptr)
        int rc = sqlite3_open_v2(filename, std::out_ptr(db, Connection::DBDeleter{}), flags, nullptr);
#else
        sqlite3* raw{nullptr};
        int rc = sqlite3_open_v2(filename, &raw, flags, nullptr);
        db.reset(raw);
#endif

        if (rc != SQLITE_OK) [[unlikely]] {
            PRINT_ERR(rc, db.get());
        }
        return db;
    }

    std::unique_ptr<sqlite3_stmt, Connection::QueryResult::StatementDeleter> createStatement(const std::shared_ptr<sqlite3> &db, std::string_view sql) {
        std::unique_ptr<sqlite3_stmt, Connection::QueryResult::StatementDeleter> stmt{nullptr};
        const auto sqlOp = nullTerminatedCStr(sql);
        if (!sqlOp.has_value()) [[unlikely]] {
            dynxxLogPrint(Error, "SQLite.createStmt SQL not null terminated");
            return stmt;
        }

        const auto dbPtr = db.get();
        const auto zSql = sqlOp.value();
        const auto nByte = static_cast<int>(sql.size());

#if defined(__cpp_lib_out_ptr)
        const auto rc = sqlite3_prepare_v2(dbPtr, zSql, nByte, std::out_ptr(stmt), nullptr);
#else
        sqlite3_stmt *raw{nullptr};
        const auto rc = sqlite3_prepare_v2(dbPtr, zSql, nByte, &raw, nullptr);
        stmt.reset(raw);
#endif

        if (rc != SQLITE_OK) [[unlikely]] {
            PRINT_ERR(rc, db.get());
            return nullptr;
        }

        return stmt;
    }
}

namespace DynXX::Core::Store::SQLite {

SQLiteStore::SQLiteStore()
{
    sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    sqlite3_initialize();
}

std::weak_ptr<Connection> SQLiteStore::open(std::string_view file)
{
    if (file.empty()) [[unlikely]]
    {
        dynxxLogPrint(Error, "SQLite.open file empty");
        return {};
    }
    const auto cid = genCid(file);
    return ConnPool<Connection>::open(cid, [cid, file]() {
        return std::make_shared<Connection>(cid, file);
    });
}

SQLiteStore::~SQLiteStore()
{
    sqlite3_shutdown();
}

Connection::Connection(CidT cid, std::string_view file) : _cid(cid), db(std::make_shared<sqlite3>(createDB(file), Connection::DBDeleter()))
{
    if (!this->execute(sEnableWAL)) [[unlikely]]
    {
        dynxxLogPrint(Warn, "SQLite enable WAL failed");
    }
}

bool Connection::execute(std::string_view sql) const
{
    const auto lock = std::scoped_lock(this->mutex);
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
    
    const auto rc = sqlite3_exec(this->db.get(), sqlOp.value(), nullptr, nullptr, nullptr);
    if (rc != SQLITE_OK)
    {
        PRINT_ERR(rc, this->db.get());
    }
    
    return rc == SQLITE_OK;
}

std::unique_ptr<Connection::QueryResult> Connection::query(std::string_view sql) const
{
    const auto lock = std::scoped_lock(this->mutex);
    return std::make_unique<Connection::QueryResult>(createStatement(this->db, sql));
}

Connection::QueryResult::QueryResult(std::unique_ptr<sqlite3_stmt, StatementDeleter> stmt) : stmt{std::move(stmt)}
{
}

bool Connection::QueryResult::readRow() const
{
    const auto lock = std::unique_lock(this->mutex);
    const auto stmt = this->stmt.get();
    if (stmt == nullptr) [[unlikely]]
    {
        dynxxLogPrint(Error, "SQLite.readRow STMT nullptr");
        return false;
    }
    const auto rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW && rc != SQLITE_DONE)
    {
        PRINT_ERR(rc, nullptr);
    }
    return rc == SQLITE_ROW;
}

std::optional<Any> Connection::QueryResult::readColumn(std::string_view column) const
{
    const auto lock = std::shared_lock(this->mutex);
    const auto stmt = this->stmt.get();
    if (stmt == nullptr) [[unlikely]]
    {
        dynxxLogPrint(Error, "SQLite.readColumn STMT nullptr");
        return std::nullopt;
    }
    const auto colCount = sqlite3_column_count(stmt);
    for (int i(0); i < colCount; i++)
    {
        const auto columnOp = nullTerminatedCStr(column);
        if (!columnOp.has_value()) [[unlikely]]
        {
            dynxxLogPrint(Error, "SQLite.readColumn column not null terminated");
            return std::nullopt;
        }
        if (strcmp(sqlite3_column_name(stmt, i), columnOp.value()) != 0)
        {
            continue;
        }
        switch(sqlite3_column_type(stmt, i))
        {
            case SQLITE_TEXT:
                {
                    const auto data = sqlite3_column_text(stmt, i);
                    const auto len = sqlite3_column_bytes(stmt, i);
                    return {makeStr(data, len)};
                }
            case SQLITE_INTEGER:
                return {sqlite3_column_int64(stmt, i)};
            case SQLITE_FLOAT:
                return {sqlite3_column_double(stmt, i)};
            default:
                return std::nullopt;
        }
    }
    return std::nullopt;
}

std::optional<Any> Connection::QueryResult::operator[](std::string_view column) const {
    return this->readColumn(column);
}

} // namespace DynXX::Core::Store::SQLite

#endif
