#ifndef NGENXX_STORE_DB_HXX_
#define NGENXX_STORE_DB_HXX_

#ifdef __cplusplus

#include "../../include/NGenXXTypes.hxx"
#include <string>
#ifdef __APPLE__
#include <sqlite3.h>
#else
#include "../../../external/sqlite/sqlite3.h"
#endif

#include <map>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>

namespace NGenXX
{
    namespace Store
    {
        class SQLite
        {
        public:
            /**
             * @brief Initialize SQLite process
             */
            SQLite();

            class Connection
            {
            private:
                struct sqlite3 *db;
                mutable std::shared_mutex mutex;

            public:
                /**
                 * @warning `Connection` can only be constructed with `SQLite::connect()`
                 */
                Connection(sqlite3 *db);

                class QueryResult
                {
                private:
                    sqlite3_stmt *stmt;
                    mutable std::shared_mutex mutex;

                public:
                    /**
                     * @warning `QueryResult` can only be constructed with `SQLite::Connection::query()`
                     */
                    QueryResult(sqlite3_stmt *stmt);

                    /**
                     * @brief Read a row from query result
                     * @return Successful or not
                     */
                    bool readRow();

                    /**
                     * @brief Read text column data from QueryResult
                     * @param column Column name
                     * @return column data
                     */
                    Any readColumn(const std::string &column);

                    /**
                     * @brief Release QueryResult
                     */
                    ~QueryResult();
                };

                /**
                 * @brief Execute SQL(s)
                 * @param sql SQL
                 * @return Successfull or not.
                 */
                bool execute(const std::string &sql);

                /**
                 * @brief Query with a SQL
                 * @param sql SQL
                 * @return A QueryResult, or NULL if execute failed.
                 */
                QueryResult *query(const std::string &sql);

                /**
                 * @brief Release DB resource
                 */
                ~Connection();
            };

            /**
             * @brief connect DB
             * @param file DB file
             * @return A Connection handle
             */
            Connection *connect(const std::string &file);

            void closeAll();

            /**
             * @brief Release SQLite process
             */
            ~SQLite();

        private:
            std::unordered_map<std::string, Connection*> conns;
            std::mutex mutex;
        };
    }
}

#endif

#endif // NGENXX_STORE_DB_HXX_