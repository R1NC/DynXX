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
    const char *cContent = content.c_str();
    if (_NGenXX_Log_callback)
    {
        char *c = reinterpret_cast<char *>(malloc(strlen(cContent) + 1));
        strcpy(c, content.c_str());
        _NGenXX_Log_callback(level, c);
    }
    else
    {
        constexpr char *tag = "NGENXX";
#ifdef __ANDROID__
        int priority = ANDROID_LOG_DEBUG;
        switch (level)
        {
        case NGenXXLogLevelInfo:
            priority = ANDROID_LOG_INFO;
            break;
        case NGenXXLogLevelDebug:
            priority = ANDROID_LOG_DEBUG;
            break;
        case NGenXXLogLevelWarn:
            priority = ANDROID_LOG_WARN;
            break;
        case NGenXXLogLevelError:
            priority = ANDROID_LOG_ERROR;
            break;
        case NGenXXLogLevelNone:
            return;
        }
        __android_log_print(priority, tag, "%s", cContent);
#elif defined(__OHOS__)
        LogLevel lLevel = LOG_DEBUG;
        switch (level)
        {
        case NGenXXLogLevelInfo:
            lLevel = LOG_INFO;
            break;
        case NGenXXLogLevelDebug:
            lLevel = LOG_DEBUG;
            break;
        case NGenXXLogLevelWarn:
            lLevel = LOG_WARN;
            break;
        case NGenXXLogLevelError:
            lLevel = LOG_ERROR;
            break;
        case NGenXXLogLevelNone:
            return;
        }
        OH_LOG_Print(LOG_APP, lLevel, 0xC0DE, tag, "%s", cContent);
#else
        const char *sLevel = "";
        switch (level)
        {
        case NGenXXLogLevelInfo:
            sLevel = "INFO";
            break;
        case NGenXXLogLevelDebug:
            sLevel = "DEBUG";
            break;
        case NGenXXLogLevelWarn:
            sLevel = "WARN";
            break;
        case NGenXXLogLevelError:
            sLevel = "ERROR";
            break;
        case NGenXXLogLevelNone:
            return;
        }
        std::cout << tag << "_" << sLevel << " -> " << cContent << std::endl;
#endif
    }
}