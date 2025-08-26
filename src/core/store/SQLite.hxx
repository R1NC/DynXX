#ifndef DYNXX_SRC_CORE_STORE_SQLITE_STORE_HXX_
#define DYNXX_SRC_CORE_STORE_SQLITE_STORE_HXX_

#if defined(__cplusplus)

#include <sqlite3.h>

#include <DynXX/CXX/Types.hxx>

#include "ConnPool.hxx"

namespace DynXX::Core::Store::SQLite {

    class Connection {
        public:
            std::string _id;

            /**
             * @warning `Connection` can only be constructed with `SQLite::connect()`
             */
            Connection() = delete;

            explicit Connection(sqlite3 *db, const std::string &_id);

            Connection(const Connection &) = delete;

            Connection &operator=(const Connection &) = delete;

            Connection(Connection &&) = delete;

            Connection &operator=(Connection &&) = delete;

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
            ~Connection();

        private:
            sqlite3 *db{nullptr};
            mutable std::mutex mutex;
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

        std::weak_ptr<SQLite::Connection> open(const std::string &file);

        void close(const std::string &_id);

        /**
         * @brief Release SQLite process
         */
        ~SQLiteStore();
    };
}

#endif

#endif // DYNXX_SRC_CORE_STORE_SQLITE_STORE_HXX_
