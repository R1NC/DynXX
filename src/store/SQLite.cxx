#include "SQLite.hxx"

#include <cstring>

#include <NGenXXLog.hxx>

#define PRINT_ERR(rc, db) ngenxxLogPrint(NGenXXLogLevelX::Error, std::string(db ? sqlite3_errmsg(db) : sqlite3_errstr(rc)))

NGenXX::Store::SQLite::SQLite()
{
    sqlite3_config(SQLITE_CONFIG_SERIALIZED);
    sqlite3_initialize();
}

std::weak_ptr<NGenXX::Store::SQLite::Connection> NGenXX::Store::SQLite::connect(const std::string &file)
{
    auto lock = std::lock_guard(this->mutex);
    if (this->conns.contains(file))
    {
        return this->conns.at(file);
    }
    sqlite3 *db;
    const auto rc = sqlite3_open_v2(file.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    // ngenxxLogPrintF(NGenXXLogLevelX::Debug, "SQLite.open ret:{}", rc);
    if (rc != SQLITE_OK) [[unlikely]]
    {
        PRINT_ERR(rc, db);
        return {};
    }
    this->conns.emplace(file, std::make_shared<Connection>(db));
    return this->conns.at(file);
}

void NGenXX::Store::SQLite::close(const std::string &file)
{
    auto lock = std::lock_guard(this->mutex);
    if (this->conns.contains(file))
    {
        this->conns.at(file).reset();
        this->conns.erase(file);
    }
}

void NGenXX::Store::SQLite::closeAll()
{
    auto lock = std::lock_guard(this->mutex);
    for (auto &[_, v] : this->conns)
    {
        v.reset();
    }
    this->conns.clear();
}

NGenXX::Store::SQLite::~SQLite()
{
    sqlite3_shutdown();
}

NGenXX::Store::SQLite::Connection::Connection(sqlite3 *db) : db{db}
{
    if (constexpr auto sEnableWAL = "PRAGMA journal_mode=WAL;"; !this->execute(sEnableWAL)) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Warn, "SQLite enable WAL failed");
    }
}

bool NGenXX::Store::SQLite::Connection::execute(const std::string &sql) const
{
    auto lock = std::lock_guard(this->mutex);
    if (this->db == nullptr) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "SQLite.execute DB nullptr");
        return false;
    }
    const auto rc = sqlite3_exec(this->db, sql.c_str(), nullptr, nullptr, nullptr);
    // ngenxxLogPrintF(NGenXXLogLevelX::Debug, "SQLite.exec ret:{}", rc);
    if (rc != SQLITE_OK)
    {
        PRINT_ERR(rc, this->db);
    }
    return rc == SQLITE_OK;
}

std::unique_ptr<NGenXX::Store::SQLite::Connection::QueryResult> NGenXX::Store::SQLite::Connection::query(const std::string &sql) const
{
    auto lock = std::lock_guard(this->mutex);
    if (this->db == nullptr) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "SQLite.query DB nullptr");
        return nullptr;
    }
    sqlite3_stmt *stmt;
    const auto rc = sqlite3_prepare_v2(this->db, sql.c_str(), -1, &stmt, nullptr);
    // ngenxxLogPrintF(NGenXXLogLevelX::Debug, "SQLite.query ret:{}", rc);
    if (rc != SQLITE_OK)
    {
        PRINT_ERR(rc, this->db);
        return nullptr;
    }
    return std::make_unique<NGenXX::Store::SQLite::Connection::QueryResult>(stmt);
}

NGenXX::Store::SQLite::Connection::~Connection()
{
    if (this->db != nullptr) [[likely]]
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
    if (this->stmt == nullptr) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "SQLite.readRow STMT nullptr");
        return false;
    }
    const auto rc = sqlite3_step(this->stmt);
    // ngenxxLogPrintF(NGenXXLogLevelX::Debug, "SQLite.step ret:{}", rc);
    if (rc != SQLITE_ROW && rc != SQLITE_DONE)
    {
        PRINT_ERR(rc, nullptr);
    }
    return rc == SQLITE_ROW;
}

std::optional<Any> NGenXX::Store::SQLite::Connection::QueryResult::readColumn(const std::string &column) const
{
    auto lock = std::shared_lock(this->mutex);
    if (this->stmt == nullptr) [[unlikely]]
    {
        ngenxxLogPrint(NGenXXLogLevelX::Error, "SQLite.readColumn STMT nullptr");
        return std::nullopt;
    }
    auto colCount = sqlite3_column_count(this->stmt);
    for (decltype(colCount) i(0); i < colCount; i++)
    {
        if (strcmp(sqlite3_column_name(this->stmt, i), column.c_str()) != 0)
        {
            continue;
        }
        switch(sqlite3_column_type(this->stmt, i))
        {
            case SQLITE_TEXT:
                return std::make_optional(wrapStr(sqlite3_column_text(this->stmt, i)));
            case SQLITE_INTEGER:
                return sqlite3_column_int64(this->stmt, i);
            case SQLITE_FLOAT:
                return sqlite3_column_double(this->stmt, i);
            default:
                return std::nullopt;
        }
    }
    return std::nullopt;
}

std::optional<Any> NGenXX::Store::SQLite::Connection::QueryResult::operator[](const std::string &column) const {
    return this->readColumn(column);
}

NGenXX::Store::SQLite::Connection::QueryResult::~QueryResult()
{
    if (this->stmt != nullptr) [[likely]]
    {
        sqlite3_finalize(this->stmt);
    }
}
