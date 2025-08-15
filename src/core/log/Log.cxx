#include "Log.hxx"

#include <cstring>

#include <mutex>
#include <iostream>

#if defined(__ANDROID__)
#include <android/log.h>
#elif defined(__OHOS__)
#include <hilog/log.h>
#elif defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

#if defined(USE_SPDLOG)
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#endif

#include <DynXX/C/Log.h>
#include <DynXX/CXX/Types.hxx>
#if defined(USE_SPDLOG)
#include <DynXX/CXX/DynXX.hxx>
#endif

#include "../concurrent/ConcurrentUtil.hxx"

#if defined(__APPLE__)
void _dynxx_log_apple(const char*);
#endif

namespace
{
    int _level = DynXXLogLevelNone;
    std::function<void(int level, const char *content)> _callback = nullptr;
    std::mutex _mutex;

    constexpr auto TAG = "DYNXX";
    constexpr size_t MAX_LEN = 1023;

    void stdLogPrint(int level, std::string_view content)
    {
#if defined(__ANDROID__)
        __android_log_print(level, TAG, "%.*s", static_cast<int>(content.length()), content.data());
#elif defined(__OHOS__)
        OH_LOG_Print(LOG_APP, static_cast<LogLevel>(level), 0xC0DE, TAG, "%{public}.*s", static_cast<int>(content.length()), content.data());
#elif defined(__APPLE__)
        _dynxx_log_apple(content.data());
#elif defined(__EMSCRIPTEN__)
        EM_ASM({
            var tag = UTF8ToString($0);
            var level = $1;
            var msg = UTF8ToString($2);
            var txt = tag + "_" + level + " -> " + msg;
            console.log(txt);
        }, TAG, level, content.data());
#else
        std::cout << TAG << "_" << level << " -> " << content << std::endl;
#endif
    }

#if defined(USE_SPDLOG)
    void spdLogPrepare() 
    {
        DynXX::Core::Concurrent::callOnce([]() {
            try 
            {
                const auto& logger = spdlog::daily_logger_mt(
                    "dynxx_spdlog",
                    dynxxRootPath() + "/log.txt",
                    0,
                    0 
                );
                spdlog::set_default_logger(logger);
                spdlog::set_pattern("[%Y-%m-%d_%H:%M:%S.%e] [%l] [%t] %v");
                switch(_level)
                {
                    case DynXXLogLevelDebug:
                        spdlog::set_level(spdlog::level::debug);
                        break;
                    case DynXXLogLevelInfo:
                        spdlog::set_level(spdlog::level::info);
                        break;
                    case DynXXLogLevelWarn:
                        spdlog::set_level(spdlog::level::warn);
                        break;
                    case DynXXLogLevelError:
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

    void spdLogPrint(const int level, std::string_view content)
    {
        spdLogPrepare();
        if (level == DynXXLogLevelDebug)
        {
            spdlog::debug(content);
        }
        else if (level == DynXXLogLevelInfo)
        {
            spdlog::info(content);
        }
        else if (level == DynXXLogLevelWarn)
        {
            spdlog::warn(content);
        }
        else if (level == DynXXLogLevelError)
        {
            spdlog::error(content);
        }
    }
#endif

    void logPrint(int level, std::string_view content)
    {
        if (_callback)
        {
            _callback(level, content.data());
        }
        else
        {
            stdLogPrint(level, content);
        }
    
#if defined(USE_SPDLOG)
        spdLogPrint(level, content);
#endif
    }

    void logPrintInBlocks(int level, std::string_view content)
    {
        if (content.length() <= MAX_LEN) [[likely]]
        {
            logPrint(level, content);
        }
        else
        {
            const auto totalLen = content.length();
            const auto blockCount = (totalLen + MAX_LEN - 1) / MAX_LEN;
            const auto blockCountStr = std::to_string(blockCount);

            std::string blockBuffer;
            blockBuffer.reserve(MAX_LEN + 50);
            
            for (size_t i = 0; i < blockCount; ++i)
            {
                blockBuffer.clear();
                
                const auto start = i * MAX_LEN;
                const auto blockLen = std::min(MAX_LEN, totalLen - start);
                
                blockBuffer.append("[");
                blockBuffer.append(std::to_string(i + 1));
                blockBuffer.append("/");
                blockBuffer.append(blockCountStr);
                blockBuffer.append("] ");
                blockBuffer.append(content, start, blockLen);
                
                logPrint(level, blockBuffer);
            }
        }
    }
}

void DynXX::Core::Log::setLevel(int level)
{
    if (level < DynXXLogLevelDebug || level > DynXXLogLevelNone) [[unlikely]]
    {
        return;
    }

#if defined(USE_SPDLOG)
    spdLogPrepare();
#endif

    auto lock = std::lock_guard(_mutex);
    _level = level;
}

void DynXX::Core::Log::setCallback(const std::function<void(int level, const char *content)> &callback)
{
    auto lock = std::lock_guard(_mutex);
    _callback = callback;
}

void DynXX::Core::Log::print(int level, std::string_view content)
{
    auto lock = std::lock_guard(_mutex);

    if (level < _level || level < DynXXLogLevelDebug || level >= DynXXLogLevelNone) [[unlikely]]
    {
        return;
    }

    logPrintInBlocks(level, content);
}
