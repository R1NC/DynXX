#ifndef NGENXX_INCLUDE_DB_HXX_
#define NGENXX_INCLUDE_DB_HXX_

#include "NGenXXTypes.hxx"

void *ngenxxStoreSqliteOpen(const std::string &_id);

bool ngenxxStoreSqliteExecute(void *const conn, std::string_view sql);

void *ngenxxStoreSqliteQueryDo(void *const conn, std::string_view sql);

bool ngenxxStoreSqliteQueryReadRow(void *const query_result);

std::optional<std::string> ngenxxStoreSqliteQueryReadColumnText(void *const query_result, std::string_view column);

std::optional<int64_t> ngenxxStoreSqliteQueryReadColumnInteger(void *const query_result, std::string_view column);

std::optional<double> ngenxxStoreSqliteQueryReadColumnFloat(void *const query_result, std::string_view column);

void ngenxxStoreSqliteQueryDrop(void *const query_result);

void ngenxxStoreSqliteClose(void *const conn);

#endif // NGENXX_INCLUDE_DB_HXX_
