#ifndef DYNXX_INCLUDE_KV_HXX_
#define DYNXX_INCLUDE_KV_HXX_

#include "Types.hxx"

void *dynxxStoreKvOpen(const std::string &_id);

std::optional<std::string> dynxxStoreKvReadString(void *const conn, std::string_view k);

bool dynxxStoreKvWriteString(void *const conn, std::string_view k, const std::string &v);

std::optional<int64_t> dynxxStoreKvReadInteger(void *const conn, std::string_view k);

bool dynxxStoreKvWriteInteger(void *const conn, std::string_view k, int64_t v);

std::optional<double> dynxxStoreKvReadFloat(void *const conn, std::string_view k);

bool dynxxStoreKvWriteFloat(void *const conn, std::string_view k, double v);

std::vector<std::string> dynxxStoreKvAllKeys(void *const conn);

bool dynxxStoreKvContains(void *const conn, std::string_view k);

bool dynxxStoreKvRemove(void *const conn, std::string_view k);

void dynxxStoreKvClear(void *const conn);

void dynxxStoreKvClose(void *const conn);

#endif // DYNXX_INCLUDE_KV_HXX_
