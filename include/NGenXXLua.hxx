#ifndef NGENXX_INCLUDE_LUA_HXX_
#define NGENXX_INCLUDE_LUA_HXX_

#include <string>

bool ngenxxLuaLoadF(const std::string &f);

bool ngenxxLuaLoadS(const std::string &s);

const std::string ngenxxLuaCall(const std::string &f, const std::string &ps);

#endif //  NGENXX_INCLUDE_LUA_HXX_