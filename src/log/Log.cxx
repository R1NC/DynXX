#include "Log.hxx"
#include "../../include/NGenXXLog.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>

static std::function<void(const int level, const char *content)> _NGenXX_Log_callback = nullptr;

static int _NGenXX_Log_level;

void NGenXX::Log::setLevel(const int level)
{
    if (level < 0 || level > 4)
        return;
    _NGenXX_Log_level = level;
}

void NGenXX::Log::setCallback(const std::function<void(const int level, const char *content)> &callback)
{
    _NGenXX_Log_callback = callback;
}

void NGenXX::Log::print(const int level, const std::string &content)
{
    if (level < _NGenXX_Log_level)
        return;
    if (_NGenXX_Log_callback)
    {
        char *c = reinterpret_cast<char *>(malloc(strlen(content.c_str()) + 1));
        strcpy(c, content.c_str());
        _NGenXX_Log_callback(level, c);
    }
    else
    {
        std::cout << level << " | " << content << std::endl;
    }
}