#pragma once

#include "Net.hxx"
#include "KV.hxx"
#include "SQLite.hxx"
#include "Lua.hxx"
#include "JS.hxx"
#include "Log.hxx"
#include "Json.hxx"
#include "Coding.hxx"
#include "Crypto.hxx"
#include "Zip.hxx"
#include "Device.hxx"
#include "Memory.hxx"

std::string dynxxGetVersion();

bool dynxxInit(const std::string &root);

std::optional<std::string> dynxxRootPath();

void dynxxRelease();
