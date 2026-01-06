#pragma once

#include <sqlite3.h>

#include <shared_mutex>

#include <DynXX/CXX/Types.hxx>

#include "ConnPool.hxx"

#include <memory>

namespace DynXX::Core::Store::SQLite {

    class Connection {
        public:
            /**
             * @warning `Connection` can only be constructed with `SQLite::connect()`
             */
            Connection() = delete;

            explicit Connection(CidT cid, std::string_view file);

            Connection(const Connection &) = delete;

            Connection &operator=(const Connection &) = delete;

            Connection(Connection &&) = delete;

            Connection &operator=(Connection &&) = delete;

            CidT cid() const { return this->_cid; }

            class QueryResult {
            public:
                /**
                 * @warning `QueryResult` can only be constructed with `SQLite::Connection::query()`
                 */
                QueryResult() = delete;

                explicit QueryResult(sqlite3_stmt *stmt);

                QueryResult(const QueryResult &) = delete;

                QueryResult &operator=(const QueryResult &) = delete;

                QueryResult(QueryResult &&) = delete;

                QueryResult &operator=(QueryResult &&) = delete;

                /**
                 * @brief Read a row from query result
                 * @return Successful or not
                 */
                [[nodiscard]] bool readRow() const;

                /**
                 * @brief Read text column data from QueryResult
                 * @param column Column name
                 * @return column data
                 */
                std::optional<Any> readColumn(std::string_view column) const;

                std::optional<Any> operator[](std::string_view column) const;

                /**
                 * @brief Release QueryResult
                 */
                ~QueryResult();

            private:
                sqlite3_stmt *stmt{nullptr};
                mutable std::shared_mutex mutex;
            };

            /**
             * @brief Execute SQL(s)
             * @param sql SQL
             * @return Successfull or not.
             */
            [[nodiscard]] bool execute(std::string_view sql) const;

            /**
             * @brief Query with a SQL
             * @param sql SQL
             * @return A QueryResult, or `nullptr` if execute failed.
             */
            std::unique_ptr<QueryResult> query(std::string_view sql) const;

            /**
             * @brief Release DB resource
             */
            ~Connection() = default;

        private:
            const CidT _cid{0};
            std::unique_ptr<sqlite3, void(*)(sqlite3*)> db;
            mutable std::shared_mutex mutex;
        };

    class SQLiteStore : public ConnPool<SQLite::Connection> {
    public:
        /**
         * @brief Initialize SQLite process
         */
        SQLiteStore();
        SQLiteStore(const SQLiteStore &) = delete;
        SQLiteStore &operator=(const SQLiteStore &) = delete;
        SQLiteStore(SQLiteStore &&) = delete;
        SQLiteStore &operator=(SQLiteStore &&) = delete;

        std::weak_ptr<SQLite::Connection> open(std::string_view file);

        /**
         * @brief Release SQLite process
         */
        ~SQLiteStore();
    };
}  // namespace DynXX::Core::Store::SQLite
