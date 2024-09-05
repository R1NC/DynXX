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
        class DB
        {
        private:
            struct sqlite3 *db;

        public:
            /**
             * @brief Open DB
             * @param file DB file
             */
            DB(const std::string &file);

            class QueryResult
            {
            private:
                sqlite3_stmt *stmt;

            public:
                /**
                 * @warning `QueryResult` can only be constructed with `DB::execute()`
                 */
                QueryResult(sqlite3_stmt *stmt);

                /**
                 * @brief Read a row from query result
                 * @return Successful or not
                 */
                bool readRow();

                /**
                 * @brief Read text column data from query result
                 * @param column Column name
                 * @return Text column data
                 */
                std::string readColumnText(const std::string &column);

                /**
                 * @brief Read integer column data from query result
                 * @param column Column name
                 * @return Integer column data
                 */
                long long readColumnInteger(const std::string &column);

                /**
                 * @brief Read float column data from query result
                 * @param column Column name
                 * @return Float column data
                 */
                double readColumnFloat(const std::string &column);

                /**
                 * @brief Release query result
                 */
                ~QueryResult();
            };

            /**
             * @brief Execute a SQL
             * @param sql SQL
             * @return A query result, or NULL if execute failed.
             */
            QueryResult *execute(const std::string &sql);

            /**
             * @brief Release DB resource
             */
            ~DB();
        };
    }
}

#endif

#endif // NGENXX_STORE_DB_HXX_