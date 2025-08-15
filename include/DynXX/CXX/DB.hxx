#ifndef DYNXX_INCLUDE_DB_HXX_
#define DYNXX_INCLUDE_DB_HXX_

#include "Types.hxx"

void *dynxxStoreSqliteOpen(const std::string &_id);

bool dynxxStoreSqliteExecute(void *const conn, std::string_view sql);

void *dynxxStoreSqliteQueryDo(void *const conn, std::string_view sql);

bool dynxxStoreSqliteQueryReadRow(void *const query_result);

std::optional<std::string> dynxxStoreSqliteQueryReadColumnText(void *const query_result, std::string_view column);

std::optional<int64_t> dynxxStoreSqliteQueryReadColumnInteger(void *const query_result, std::string_view column);

std::optional<double> dynxxStoreSqliteQueryReadColumnFloat(void *const query_result, std::string_view column);

void dynxxStoreSqliteQueryDrop(void *const query_result);

void dynxxStoreSqliteClose(void *const conn);

#endif // DYNXX_INCLUDE_DB_HXX_
