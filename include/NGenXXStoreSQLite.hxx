#ifndef NGENXX_INCLUDE_STORE_SQLITE_HXX_
#define NGENXX_INCLUDE_STORE_SQLITE_HXX_

void *const ngenxxStoreSqliteOpen(const std::string &_id);

bool ngenxxStoreSqliteExecute(void *const conn, const std::string &sql);

void *const ngenxxStoreSqliteQueryDo(void *const conn, const std::string &sql);

bool ngenxxStoreSqliteQueryReadRow(void *const query_result);

const std::string ngenxxStoreSqliteQueryReadColumnText(void *const query_result, const std::string &column);

long long ngenxxStoreSqliteQueryReadColumnInteger(void *const query_result, const std::string &column);

double ngenxxStoreSqliteQueryReadColumnFloat(void *const query_result, const std::string &column);

void ngenxxStoreSqliteQueryDrop(void *const query_result);

void ngenxxStoreSqliteClose(void *const conn);

#endif // NGENXX_INCLUDE_STORE_SQLITE_HXX_