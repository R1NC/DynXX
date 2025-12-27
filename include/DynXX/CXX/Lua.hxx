#pragma once

#include "Types.hxx"

[[nodiscard]] bool dynxxLuaLoadF(std::string_view f);

[[nodiscard]] bool dynxxLuaLoadS(std::string_view s);

[[nodiscard]] std::optional<std::string> dynxxLuaCall(std::string_view f, std::string_view ps);
