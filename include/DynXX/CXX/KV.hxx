#pragma once

#include "../C/KV.h"
#include "Types.hxx"

DynXXKVConnHandle dynxxKVOpen(const std::string &_id);

std::optional<std::string> dynxxKVReadString(DynXXKVConnHandle conn, std::string_view k);

bool dynxxKVWriteString(DynXXKVConnHandle conn, std::string_view k, const std::string &v);

std::optional<int64_t> dynxxKVReadInteger(DynXXKVConnHandle conn, std::string_view k);

bool dynxxKVWriteInteger(DynXXKVConnHandle conn, std::string_view k, int64_t v);

std::optional<double> dynxxKVReadFloat(DynXXKVConnHandle conn, std::string_view k);

bool dynxxKVWriteFloat(DynXXKVConnHandle conn, std::string_view k, double v);

std::vector<std::string> dynxxKVAllKeys(DynXXKVConnHandle conn);

bool dynxxKVContains(DynXXKVConnHandle conn, std::string_view k);

bool dynxxKVRemove(DynXXKVConnHandle conn, std::string_view k);

void dynxxKVClear(DynXXKVConnHandle conn);

void dynxxKVClose(DynXXKVConnHandle conn);
