#include "Log.hxx"
#include "../../include/NGenXXLog.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>

#ifdef __ANDROID__
#include <android/log.h>
#elif defined(__OHOS__)
#include <hilog/log.h>
#endif

static std::function<void(const int level, const char *content)> _NGenXX_Log_callback = nullptr;

static int _NGenXX_Log_level = NGenXXLogLevelNone;

void NGenXX::Log::setLevel(const int level)
{
    if (level < NGenXXLogLevelDebug || level > NGenXXLogLevelNone)
        return;
    _NGenXX_Log_level = level;
}

void NGenXX::Log::setCallback(const std::function<void(const int level, const char *content)> &callback)
{
    _NGenXX_Log_callback = callback;
}

void NGenXX::Log::print(const int level, const std::string &content)
{
    if (level < _NGenXX_Log_level || level < NGenXXLogLevelDebug || level >= NGenXXLogLevelNone)
        return;
    const char *cContent = content.c_str();
    if (_NGenXX_Log_callback)
    {
        char *c = reinterpret_cast<char *>(malloc(strlen(cContent) + 1));
        strcpy(c, content.c_str());
        _NGenXX_Log_callback(level, c);
    }
    else
    {
        const char *tag = "NGENXX";
#ifdef __ANDROID__
        __android_log_print(level, tag, "%s", cContent);
#elif defined(__OHOS__)
        OH_LOG_Print(LOG_APP, static_cast<LogLevel>(level), 0xC0DE, tag, "%s", cContent);
#else
        std::cout << tag << "_" << level << " -> " << cContent << std::endl;
#endif
    }
}