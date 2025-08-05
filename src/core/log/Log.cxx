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
    constexpr size_t MAX_LEN = 1023;

    void stdLogPrint(int level, std::string_view content)
    {
#if defined(__ANDROID__)
        __android_log_print(level, TAG, "%.*s", static_cast<int>(content.length()), content.data());
#elif defined(__OHOS__)
        OH_LOG_Print(LOG_APP, static_cast<LogLevel>(level), 0xC0DE, TAG, "%{public}.*s", static_cast<int>(content.length()), content.data());
#elif defined(__APPLE__)
        _ngenxx_log_apple(content.data());
#elif defined(__EMSCRIPTEN__)
        EM_ASM({
            var tag = UTF8ToString($0);
            var level = $1;
            var msg = UTF8ToString($2);
            var txt = tag + "_" + level + " -> " + msg;
            console.log(txt);
        }, tag, level, ch);
#else
        std::cout << TAG << "_" << level << " -> " << content << std::endl;
#endif
    }

#if defined(USE_SPDLOG)
    void spdLogPrepare() 
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

    void spdLogPrint(const int level, std::string_view content)
    {
        spdLogPrepare();
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

void NGenXX::Core::Log::setLevel(int level)
{
    if (level < NGenXXLogLevelDebug || level > NGenXXLogLevelNone) [[unlikely]]
    {
        return;
    }

#if defined(USE_SPDLOG)
    spdLogPrepare();
#endif

    auto lock = std::lock_guard(_mutex);
    _level = level;
}

void NGenXX::Core::Log::setCallback(const std::function<void(int level, const char *content)> &callback)
{
    auto lock = std::lock_guard(_mutex);
    _callback = callback;
}

void NGenXX::Core::Log::print(int level, std::string_view content)
{
    auto lock = std::lock_guard(_mutex);

    if (level < _level || level < NGenXXLogLevelDebug || level >= NGenXXLogLevelNone) [[unlikely]]
    {
        return;
    }

    logPrintInBlocks(level, content);
}
