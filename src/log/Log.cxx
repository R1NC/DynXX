#include "Log.hxx"

#include <cstring>

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

#if defined(__APPLE__)
void _ngenxx_log_apple(const char*);
#endif

static std::function<void(int level, const char *content)> _NGenXX_Log_callback = nullptr;
static std::unique_ptr<std::mutex> _ngenxx_log_mutex = nullptr;

static int _NGenXX_Log_level = NGenXXLogLevelNone;

void NGenXX::Log::setLevel(int level)
{
    if (level < NGenXXLogLevelDebug || level > NGenXXLogLevelNone) [[unlikely]]
    {
        return;
    }
    _NGenXX_Log_level = level;
}

void NGenXX::Log::setCallback(const std::function<void(int level, const char *content)> &callback)
{
    _NGenXX_Log_callback = callback;
    if (callback == nullptr)
    {
        _ngenxx_log_mutex.reset();
    }
}

void NGenXX::Log::print(int level, const std::string &content)
{
    if (level < _NGenXX_Log_level || level < NGenXXLogLevelDebug || level >= NGenXXLogLevelNone) [[unlikely]]
    {
        return;
    }

    if (!_ngenxx_log_mutex)
    {
        _ngenxx_log_mutex = std::make_unique<std::mutex>();
    }
    auto lock = std::lock_guard(*_ngenxx_log_mutex.get());

    auto cContent = content.c_str();
    if (_NGenXX_Log_callback)
    {
        auto len = std::strlen(cContent);
        auto c = mallocX<char>(len);
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
#elif defined(__APPLE__)
        _ngenxx_log_apple(cContent);
#else
        std::cout << tag << "_" << level << " -> " << cContent << std::endl;
#endif
    }
}
