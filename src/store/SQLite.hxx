#ifndef NGENXX_STORE_DB_HXX_
#define NGENXX_STORE_DB_HXX_

#ifdef __cplusplus

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

                class ExecuteResult
                {
                private:
                    struct sqlite3 *db;
                    sqlite3_stmt *stmt;

                public:
                    /**
                     * @warning `ExcuteResult` can only be constructed with `SQLite::Connection::execute()`
                     */
                    ExecuteResult(struct sqlite3 *db, sqlite3_stmt *stmt);

                    /**
                     * @brief Read a row from query result
                     * @return Successful or not
                     */
                    bool readRow();

                    /**
                     * @brief Read text column data from Exexcute result
                     * @param column Column name
                     * @return Text column data
                     */
                    std::string readColumnText(const std::string &column);

                    /**
                     * @brief Read integer column data from Exexcute result
                     * @param column Column name
                     * @return Integer column data
                     */
                    long long readColumnInteger(const std::string &column);

                    /**
                     * @brief Read float column data from Exexcute result
                     * @param column Column name
                     * @return Float column data
                     */
                    double readColumnFloat(const std::string &column);

                    /**
                     * @brief Release Exexcute result
                     */
                    ~ExecuteResult();
                };

                /**
                 * @brief Execute a SQL
                 * @param sql SQL
                 * @return A Exexcute result, or NULL if execute failed.
                 */
                ExecuteResult *execute(const std::string &sql);

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