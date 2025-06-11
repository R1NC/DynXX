#include "Log.hxx"

#include <cstring>

#include <mutex>
#include <iostream>

#if defined(__ANDROID__)
#include <android/log.h>
#elif defined(__OHOS__)
#include <hilog/log.h>
#endif

#if defined(USE_SPDLOG)
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#endif

#include <NGenXXLog.h>
#include <NGenXXTypes.hxx>
#if defined(USE_SPDLOG)
#include <NGenXX.hxx>
#endif

#include "../concurrent/ConcurrentUtil.hxx"

#if defined(__APPLE__)
void _ngenxx_log_apple(const char*);
#endif

namespace
{
    int _level = NGenXXLogLevelNone;
    std::function<void(int level, const char *content)> _callback = nullptr;
    std::mutex _mutex;

    constexpr auto TAG = "NGENXX";

    void stdLogPrint(int level, const std::string &content)
    {
        auto cContent = content.c_str();
#if defined(__ANDROID__)
        __android_log_print(level, TAG, "%s", cContent);
#elif defined(__OHOS__)
        OH_LOG_Print(LOG_APP, static_cast<LogLevel>(level), 0xC0DE, TAG, "%{public}s", cContent);
#elif defined(__APPLE__)
        _ngenxx_log_apple(cContent);
#else
        std::cout << TAG << "_" << level << " -> " << cContent << std::endl;
#endif
    }

#if defined(USE_SPDLOG)
    void spdlogPrepare() 
    {
        NGenXX::Core::Concurrent::callOnce([]() {
            try 
            {
                const auto& logger = spdlog::daily_logger_mt(
                    "ngenxx_spdlog",
                    ngenxxRootPath() + "/log.txt",
                    0,
                    0 
                );
                spdlog::set_default_logger(logger);
                spdlog::set_pattern("[%Y-%m-%d_%H:%M:%S.%e] [%l] [%t] %v");
                switch(_level)
                {
                    case NGenXXLogLevelDebug:
                        spdlog::set_level(spdlog::level::debug);
                        break;
                    case NGenXXLogLevelInfo:
                        spdlog::set_level(spdlog::level::info);
                        break;
                    case NGenXXLogLevelWarn:
                        spdlog::set_level(spdlog::level::warn);
                        break;
                    case NGenXXLogLevelError:
                        spdlog::set_level(spdlog::level::err);
                        break;
                    default:
                        spdlog::set_level(spdlog::level::off);
                }
                spdlog::flush_on(spdlog::level::debug);
            }
            catch (const spdlog::spdlog_ex& ex) 
            {
                std::cerr << "SpdLog init failed: " << ex.what() << std::endl;
            }
        });
    }

    void spdlogPrint(const int level, const std::string &content)
    {
        spdlogPrepare();
        if (level == NGenXXLogLevelDebug)
        {
            spdlog::debug(content);
        }
        else if (level == NGenXXLogLevelInfo)
        {
            spdlog::info(content);
        }
        else if (level == NGenXXLogLevelWarn)
        {
            spdlog::warn(content);
        }
        else if (level == NGenXXLogLevelError)
        {
            spdlog::error(content);
        }
    }
#endif
}

void NGenXX::Core::Log::setLevel(int level)
{
    if (level < NGenXXLogLevelDebug || level > NGenXXLogLevelNone) [[unlikely]]
    {
        return;
    }
    _level = level;
#if defined(USE_SPDLOG)
    spdlogPrepare();
#endif
}

void NGenXX::Core::Log::setCallback(const std::function<void(int level, const char *content)> &callback)
{
    _callback = callback;
}

void NGenXX::Core::Log::print(int level, const std::string &content)
{
    auto lock = std::lock_guard(_mutex);

    if (level < _level || level < NGenXXLogLevelDebug || level >= NGenXXLogLevelNone) [[unlikely]]
    {
        return;
    }

    if (_callback)
    {
        _callback(level, content.c_str());
    }
    else
    {
        stdLogPrint(level, content);
    }
    
#if defined(USE_SPDLOG)
    spdlogPrint(level, content);
#endif
}
