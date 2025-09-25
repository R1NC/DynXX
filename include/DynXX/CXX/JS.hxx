#pragma once

#include "Types.hxx"

#include <functional>

bool dynxxJsLoadF(const std::string &file, bool isModule);

bool dynxxJsLoadS(const std::string &script, const std::string &name, bool isModule);

bool dynxxJsLoadB(const Bytes &bytes, bool isModule);

std::optional<std::string> dynxxJsCall(std::string_view func, std::string_view params, bool await);

void dynxxJsSetMsgCallback(const std::function<const char *(const char *msg)> &callback);
