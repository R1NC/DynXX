#pragma once

#include "Types.hxx"

bool dynxxLuaLoadF(const std::string &f);

bool dynxxLuaLoadS(const std::string &s);

std::optional<std::string> dynxxLuaCall(std::string_view f, std::string_view ps);