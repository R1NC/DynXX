#ifndef NGENXX_INCLUDE_LUA_HXX_
#define NGENXX_INCLUDE_LUA_HXX_

#include "NGenXXTypes.hxx"

bool ngenxxLuaLoadF(const std::string &f);

bool ngenxxLuaLoadS(const std::string &s);

std::optional<std::string> ngenxxLuaCall(std::string_view f, std::string_view ps);

#endif // NGENXX_INCLUDE_LUA_HXX_
