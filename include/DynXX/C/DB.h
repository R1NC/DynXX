#ifndef DYNXX_INCLUDE_DB_H_
#define DYNXX_INCLUDE_DB_H_

#include "Types.h"

EXTERN_C_BEGIN

typedef void* DynXXDBConnHandle;
typedef void* DynXXDBQueryResultHandle;

/**
 * @brief Open DB
 * @param _id DB id
 * @return DB connection handle
 */
DynXXDBConnHandle dynxx_store_sqlite_open(const char *_id);

/**
 * @brief Execute a SQL(s)
 * @param conn DB connection handle
 * @param sql SQL
 * @return Successfull or not
 */
bool dynxx_store_sqlite_execute(const DynXXDBConnHandle conn, const char *sql);

/**
 * @brief Query with a SQL
 * @param conn DB connection handle
 * @param sql SQL
 * @return Handle of query result
 */
DynXXDBQueryResultHandle dynxx_store_sqlite_query_do(const DynXXDBConnHandle conn, const char *sql);

/**
 * @brief Read a row from query result
 * @param query_result Handle of query result
 * @return Successful or not
 */
bool dynxx_store_sqlite_query_read_row(const DynXXDBQueryResultHandle query_result);

/**
 * @brief Read text column data from query result
 * @param query_result Handle of query result
 * @param column Column name
 * @return Text column data
 */
const char *dynxx_store_sqlite_query_read_column_text(const DynXXDBQueryResultHandle query_result, const char *column);

/**
 * @brief Read integer column data from query result
 * @param query_result Handle of query result
 * @param column Column name
 * @return Integer column data
 */
int64_t dynxx_store_sqlite_query_read_column_integer(const DynXXDBQueryResultHandle query_result, const char *column);

/**
 * @brief Read float column data from query result
 * @param query_result Handle of query result
 * @param column Column name
 * @return Float column data
 */
double dynxx_store_sqlite_query_read_column_float(const DynXXDBQueryResultHandle query_result, const char *column);

/**
 * @brief Release query result
 * @param query_result Handle of query result
 */
void dynxx_store_sqlite_query_drop(const DynXXDBQueryResultHandle query_result);

/**
 * @brief Close DB
 * @param conn DB connection handle
 */
void dynxx_store_sqlite_close(const DynXXDBConnHandle conn);

EXTERN_C_END

#endif // DYNXX_INCLUDE_DB_H_
