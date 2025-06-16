#ifndef NGENXX_INCLUDE_LUA_HXX_
#define NGENXX_INCLUDE_LUA_HXX_

#include <string>
#include <optional>

bool ngenxxLuaLoadF(const std::string &f);

bool ngenxxLuaLoadS(const std::string &s);

std::optional<std::string> ngenxxLuaCall(const std::string &f, const std::string &ps);

#endif // NGENXX_INCLUDE_LUA_HXX_
