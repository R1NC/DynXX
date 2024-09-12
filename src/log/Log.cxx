#include "Log.hxx"
#include "../../include/NGenXXLog.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>

static void (*_NGenXX_Log_callback)(int, const char *);

static int _NGenXX_Log_level;

void NGenXX::Log::setLevel(int level)
{
    if (level < 0 || level > 4)
        return;
    _NGenXX_Log_level = level;
}

void NGenXX::Log::setCallback(void (*callback)(int level, const char *content))
{
    _NGenXX_Log_callback = callback;
}

void NGenXX::Log::print(int level, const char *content)
{
    if (content == NULL || level < _NGenXX_Log_level)
        return;
    if (_NGenXX_Log_callback)
    {
        char *c = (char *)malloc(strlen(content) + 1);
        strcpy(c, content);
        _NGenXX_Log_callback(level, c);
    }
    else
    {
        std::cout << level << " | " << content << std::endl;
    }
}