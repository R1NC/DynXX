#pragma once

#include "Types.hxx"

#include <functional>

bool dynxxJsLoadF(std::string_view file, bool isModule);

bool dynxxJsLoadS(std::string_view script, std::string_view name, bool isModule);

bool dynxxJsLoadB(BytesView bytes, bool isModule);

std::optional<std::string> dynxxJsCall(std::string_view func, std::string_view params, bool await);

void dynxxJsSetMsgCallback(const std::function<const char *(const char *msg)> &callback);
