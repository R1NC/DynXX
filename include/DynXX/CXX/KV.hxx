#ifndef DYNXX_INCLUDE_KV_HXX_
#define DYNXX_INCLUDE_KV_HXX_

#include "Types.hxx"
#include "../C/KV.h"

DynXXKVConnHandle dynxxKVOpen(const std::string &_id);

std::optional<std::string> dynxxKVReadString(const DynXXKVConnHandle conn, std::string_view k);

bool dynxxKVWriteString(const DynXXKVConnHandle conn, std::string_view k, const std::string &v);

std::optional<int64_t> dynxxKVReadInteger(const DynXXKVConnHandle conn, std::string_view k);

bool dynxxKVWriteInteger(const DynXXKVConnHandle conn, std::string_view k, int64_t v);

std::optional<double> dynxxKVReadFloat(const DynXXKVConnHandle conn, std::string_view k);

bool dynxxKVWriteFloat(const DynXXKVConnHandle conn, std::string_view k, double v);

std::vector<std::string> dynxxKVAllKeys(const DynXXKVConnHandle conn);

bool dynxxKVContains(const DynXXKVConnHandle conn, std::string_view k);

bool dynxxKVRemove(const DynXXKVConnHandle conn, std::string_view k);

void dynxxKVClear(const DynXXKVConnHandle conn);

void dynxxKVClose(const DynXXKVConnHandle conn);

#endif // DYNXX_INCLUDE_KV_HXX_
