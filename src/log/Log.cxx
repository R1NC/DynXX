#include "Log.hxx"
#include <NGenXXLog.h>

#include <string.h>
#include <stdlib.h>

#if defined(__ANDROID__)
#include <android/log.h>
#elif defined(__OHOS__)
#include <hilog/log.h>
#else
#include <iostream>
#endif

#include <mutex>

static std::function<void(const int level, const char *content)> _NGenXX_Log_callback = nullptr;
static std::mutex *_ngenxx_log_mutex = nullptr;

static int _NGenXX_Log_level = NGenXXLogLevelNone;

void NGenXX::Log::setLevel(const int level)
{
    if (level < NGenXXLogLevelDebug || level > NGenXXLogLevelNone)
    {
        return;
    }
    _NGenXX_Log_level = level;
}

void NGenXX::Log::setCallback(const std::function<void(const int level, const char *content)> &callback)
{
    _NGenXX_Log_callback = callback;
}

void NGenXX::Log::print(const int level, const std::string &content)
{
    if (_ngenxx_log_mutex == nullptr)
    {
        _ngenxx_log_mutex = new std::mutex();
    }
    const std::lock_guard<std::mutex> lock(*_ngenxx_log_mutex);

    if (level < _NGenXX_Log_level || level < NGenXXLogLevelDebug || level >= NGenXXLogLevelNone)
    {
        return;
    }
    const char *cContent = content.c_str();
    if (_NGenXX_Log_callback)
    {
        char *c = reinterpret_cast<char *>(malloc(strlen(cContent) + 1));
        strcpy(c, content.c_str());
        _NGenXX_Log_callback(level, c);
    }
    else
    {
        static const std::string tag = "NGENXX";
#if defined(__ANDROID__)
        __android_log_print(level, tag.c_str(), "%s", cContent);
#elif defined(__OHOS__)
        OH_LOG_Print(LOG_APP, static_cast<LogLevel>(level), 0xC0DE, tag.c_str(), "%s", cContent);
#else
        std::cout << tag << "_" << level << " -> " << cContent << std::endl;
#endif
    }
}
