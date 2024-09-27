#ifndef NGENXX_STORE_DB_HXX_
#define NGENXX_STORE_DB_HXX_

#ifdef __cplusplus

#include "../NGenXX-Types.hxx"
#include <string>
#ifdef __APPLE__
#include <sqlite3.h>
#else
#include "../../../external/sqlite/sqlite3.h"
#endif

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

            public:
                /**
                 * @warning `Connection` can only be constructed with `SQLite::connect()`
                 */
                Connection(sqlite3 *db);

                class QueryResult
                {
                private:
                    sqlite3_stmt *stmt;

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

            /**
             * @brief Release SQLite process
             */
            ~SQLite();
        };
    }
}

#endif

#endif // NGENXX_STORE_DB_HXX_