#ifndef DYNXX_INCLUDE_HXX_
#define DYNXX_INCLUDE_HXX_

#include "Net.hxx"
#include "KV.hxx"
#include "DB.hxx"
#include "Lua.hxx"
#include "JS.hxx"
#include "Log.hxx"
#include "Json.hxx"
#include "Coding.hxx"
#include "Crypto.hxx"
#include "Zip.hxx"
#include "Device.hxx"

std::string dynxxGetVersion();

bool dynxxInit(const std::string &root);

std::string dynxxRootPath();

void dynxxRelease();

#endif // DYNXX_INCLUDE_HXX_
