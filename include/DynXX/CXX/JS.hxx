#pragma once

#include "Types.hxx"

#include <functional>

[[nodiscard]] bool dynxxJsLoadF(std::string_view file, bool isModule = false);

[[nodiscard]] bool dynxxJsLoadS(std::string_view script, std::string_view name, bool isModule = false);

[[nodiscard]] bool dynxxJsLoadB(BytesView bytes, bool isModule = false);

[[nodiscard]] std::optional<std::string> dynxxJsCall(std::string_view func, std::string_view params, bool await = false);

void dynxxJsSetMsgCallback(const std::function<const char *(const char *msg)> &callback);
