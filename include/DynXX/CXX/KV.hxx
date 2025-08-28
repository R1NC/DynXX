#ifndef DYNXX_INCLUDE_KV_HXX_
#define DYNXX_INCLUDE_KV_HXX_

#include "Types.hxx"
#include "../C/KV.h"

DynXXKVConnHandle dynxxStoreKvOpen(const std::string &_id);

std::optional<std::string> dynxxStoreKvReadString(const DynXXKVConnHandle conn, std::string_view k);

bool dynxxStoreKvWriteString(const DynXXKVConnHandle conn, std::string_view k, const std::string &v);

std::optional<int64_t> dynxxStoreKvReadInteger(const DynXXKVConnHandle conn, std::string_view k);

bool dynxxStoreKvWriteInteger(const DynXXKVConnHandle conn, std::string_view k, int64_t v);

std::optional<double> dynxxStoreKvReadFloat(const DynXXKVConnHandle conn, std::string_view k);

bool dynxxStoreKvWriteFloat(const DynXXKVConnHandle conn, std::string_view k, double v);

std::vector<std::string> dynxxStoreKvAllKeys(const DynXXKVConnHandle conn);

bool dynxxStoreKvContains(const DynXXKVConnHandle conn, std::string_view k);

bool dynxxStoreKvRemove(const DynXXKVConnHandle conn, std::string_view k);

void dynxxStoreKvClear(const DynXXKVConnHandle conn);

void dynxxStoreKvClose(const DynXXKVConnHandle conn);

#endif // DYNXX_INCLUDE_KV_HXX_
