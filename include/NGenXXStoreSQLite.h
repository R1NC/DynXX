#ifndef NGENXX_STORE_SQLITE_H_
#define NGENXX_STORE_SQLITE_H_

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief Open DB
 * @param file DB file
 * @return DB connection handle
 */
void *ngenxx_store_sqlite_open(const char *file);

/**
 * @brief Execute a SQL(s)
 * @param conn DB connection handle
 * @param sql SQL
 * @return Successfull or not
 */
bool ngenxx_store_sqlite_execute(void *conn, const char *sql);

/**
 * @brief Query with a SQL
 * @param conn DB connection handle
 * @param sql SQL
 * @return Handle of query result
 */
void *ngenxx_store_sqlite_query_do(void *conn, const char *sql);

/**
 * @brief Read a row from query result
 * @param query_result Handle of query result
 * @return Successful or not
 */
bool ngenxx_store_sqlite_query_read_row(void *query_result);

/**
 * @brief Read text column data from query result
 * @param query_result Handle of query result
 * @param column Column name
 * @return Text column data
 */
const char *ngenxx_store_sqlite_query_read_column_text(void *query_result, const char *column);

/**
 * @brief Read integer column data from query result
 * @param query_result Handle of query result
 * @param column Column name
 * @return Integer column data
 */
long long ngenxx_store_sqlite_query_read_column_integer(void *query_result, const char *column);

/**
 * @brief Read float column data from query result
 * @param query_result Handle of query result
 * @param column Column name
 * @return Float column data
 */
double ngenxx_store_sqlite_query_read_column_float(void *query_result, const char *column);

/**
 * @brief Release query result
 * @param query_result Handle of query result
 */
void ngenxx_store_sqlite_query_drop(void *query_result);

/**
 * @brief Close DB
 * @param conn DB connection handle
 */
void ngenxx_store_sqlite_close(void *conn);

#ifdef  __cplusplus
}
#endif

#endif // NGENXX_STORE_SQLITE_H_