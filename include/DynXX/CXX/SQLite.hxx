#pragma once

#include "Types.hxx"
#include "../C/SQLite.h"

DynXXSQLiteConnHandle dynxxSQLiteOpen(std::string_view _id);

bool dynxxSQLiteExecute(DynXXSQLiteConnHandle conn, std::string_view sql);

DynXXSQLiteQueryResultHandle dynxxSQLiteQueryDo(DynXXSQLiteConnHandle conn, std::string_view sql);

bool dynxxSQLiteQueryReadRow(DynXXSQLiteQueryResultHandle query_result);

std::optional<std::string> dynxxSQLiteQueryReadColumnText(DynXXSQLiteQueryResultHandle query_result, std::string_view column);

std::optional<int64_t> dynxxSQLiteQueryReadColumnInteger(DynXXSQLiteQueryResultHandle query_result, std::string_view column);

std::optional<double> dynxxSQLiteQueryReadColumnFloat(DynXXSQLiteQueryResultHandle query_result, std::string_view column);

void dynxxSQLiteQueryDrop(DynXXSQLiteQueryResultHandle query_result);

void dynxxSQLiteClose(DynXXSQLiteConnHandle conn);
