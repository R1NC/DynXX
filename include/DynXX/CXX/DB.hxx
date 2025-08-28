#ifndef DYNXX_INCLUDE_DB_HXX_
#define DYNXX_INCLUDE_DB_HXX_

#include "Types.hxx"
#include "../C/DB.h"

DynXXDBConnHandle dynxxStoreSqliteOpen(const std::string &_id);

bool dynxxStoreSqliteExecute(const DynXXDBConnHandle conn, std::string_view sql);

DynXXDBQueryResultHandle dynxxStoreSqliteQueryDo(const DynXXDBConnHandle conn, std::string_view sql);

bool dynxxStoreSqliteQueryReadRow(const DynXXDBQueryResultHandle query_result);

std::optional<std::string> dynxxStoreSqliteQueryReadColumnText(const DynXXDBQueryResultHandle query_result, std::string_view column);

std::optional<int64_t> dynxxStoreSqliteQueryReadColumnInteger(const DynXXDBQueryResultHandle query_result, std::string_view column);

std::optional<double> dynxxStoreSqliteQueryReadColumnFloat(const DynXXDBQueryResultHandle query_result, std::string_view column);

void dynxxStoreSqliteQueryDrop(const DynXXDBQueryResultHandle query_result);

void dynxxStoreSqliteClose(const DynXXDBConnHandle conn);

#endif // DYNXX_INCLUDE_DB_HXX_
