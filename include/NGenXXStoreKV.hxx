#ifndef NGENXX_INCLUDE_STORE_KV_HXX_
#define NGENXX_INCLUDE_STORE_KV_HXX_

#include <string>
#include <vector>

void *const ngenxxStoreKvOpen(const std::string &_id);

const std::string ngenxxStoreKvReadString(void *const conn, const std::string &k);

bool ngenxxStoreKvWriteString(void *const conn, const std::string &k, const std::string &v);

long long ngenxxStoreKvReadInteger(void *const conn, const std::string &k);

bool ngenxxStoreKvWriteInteger(void *const conn, const std::string &k, long long v);

double ngenxxStoreKvReadFloat(void *const conn, const std::string &k);

bool ngenxxStoreKvWriteFloat(void *const conn, const std::string &k, double v);

const std::vector<std::string> ngenxxStoreKvAllKeys(void *const conn);

bool ngenxxStoreKvContains(void *const conn, const std::string &k);

void ngenxxStoreKvRemove(void *const conn, const std::string &k);

void ngenxxStoreKvClear(void *const conn);

void ngenxxStoreKvClose(void *const conn);

#endif // NGENXX_INCLUDE_STORE_KV_HXX_