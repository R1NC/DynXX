#include "Log.hxx"

#include <cstring>
#include <cstdlib>

#include <mutex>

#if defined(__ANDROID__)
#include <android/log.h>
#elif defined(__OHOS__)
#include <hilog/log.h>
#else
#include <iostream>
#endif

#include <NGenXXLog.h>
#include <NGenXXTypes.hxx>

static std::function<void(const int level, const char *content)> _NGenXX_Log_callback = nullptr;
static std::mutex *_ngenxx_log_mutex = nullptr;

static int _NGenXX_Log_level = NGenXXLogLevelNone;

void NGenXX::Log::setLevel(const int level)
{
    if (level < NGenXXLogLevelDebug || level > NGenXXLogLevelNone) [[unlikely]]
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
    auto lock = std::lock_guard(*_ngenxx_log_mutex);

    if (level < _NGenXX_Log_level || level < NGenXXLogLevelDebug || level >= NGenXXLogLevelNone) [[unlikely]]
    {
        return;
    }
    auto cContent = content.c_str();
    if (_NGenXX_Log_callback)
    {
        auto len = std::strlen(cContent);
        auto c = mallocPtr<char>(len);
        std::strncpy(c, content.c_str(), len);
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
