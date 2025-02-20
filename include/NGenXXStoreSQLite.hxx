#ifndef NGENXX_INCLUDE_STORE_SQLITE_HXX_
#define NGENXX_INCLUDE_STORE_SQLITE_HXX_

#include <string>

void *ngenxxStoreSqliteOpen(const std::string &_id);

bool ngenxxStoreSqliteExecute(void *const conn, const std::string &sql);

void *ngenxxStoreSqliteQueryDo(void *const conn, const std::string &sql);

bool ngenxxStoreSqliteQueryReadRow(void *const query_result);

std::string ngenxxStoreSqliteQueryReadColumnText(void *const query_result, const std::string &column);

int64_t ngenxxStoreSqliteQueryReadColumnInteger(void *const query_result, const std::string &column);

double ngenxxStoreSqliteQueryReadColumnFloat(void *const query_result, const std::string &column);

void ngenxxStoreSqliteQueryDrop(void *const query_result);

void ngenxxStoreSqliteClose(void *const conn);

#endif // NGENXX_INCLUDE_STORE_SQLITE_HXX_