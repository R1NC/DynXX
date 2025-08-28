#ifndef DYNXX_INCLUDE_SQLITE_HXX_
#define DYNXX_INCLUDE_SQLITE_HXX_

#include "Types.hxx"
#include "../C/SQLite.h"

DynXXSQLiteConnHandle dynxxSQLiteOpen(const std::string &_id);

bool dynxxSQLiteExecute(const DynXXSQLiteConnHandle conn, std::string_view sql);

DynXXSQLiteQueryResultHandle dynxxSQLiteQueryDo(const DynXXSQLiteConnHandle conn, std::string_view sql);

bool dynxxSQLiteQueryReadRow(const DynXXSQLiteQueryResultHandle query_result);

std::optional<std::string> dynxxSQLiteQueryReadColumnText(const DynXXSQLiteQueryResultHandle query_result, std::string_view column);

std::optional<int64_t> dynxxSQLiteQueryReadColumnInteger(const DynXXSQLiteQueryResultHandle query_result, std::string_view column);

std::optional<double> dynxxSQLiteQueryReadColumnFloat(const DynXXSQLiteQueryResultHandle query_result, std::string_view column);

void dynxxSQLiteQueryDrop(const DynXXSQLiteQueryResultHandle query_result);

void dynxxSQLiteClose(const DynXXSQLiteConnHandle conn);

#endif // DYNXX_INCLUDE_SQLITE_HXX_
