#pragma once

#include "Types.hxx"

bool dynxxLuaLoadF(std::string_view f);

bool dynxxLuaLoadS(std::string_view s);

std::optional<std::string> dynxxLuaCall(std::string_view f, std::string_view ps);
