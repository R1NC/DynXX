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

#if defined(__APPLE__)
void _dynxx_log_apple(const char*);
#endif

namespace
{
    using enum DynXXLogLevelX;

    DynXXLogLevelX _level = None;
    std::function<void(int level, const char *content)> _callback = nullptr;
    std::mutex _mutex;

    constexpr auto TAG = "DYNXX";
    constexpr auto MAX_LEN = 1023uz;

    bool isDebug() {
        return _level == Debug;
    }

    void prepareStdIO() {
        std::ios::sync_with_stdio(false);
        std::cin.tie(nullptr);
    }

    void stdLogPrint(DynXXLogLevelX level, std::string_view content)
    {
        const auto iLevel = underlying(level);
#if defined(__ANDROID__)
        __android_log_print(iLevel, TAG, "%.*s", static_cast<int>(content.length()), content.data());
#elif defined(__OHOS__)
        OH_LOG_Print(LOG_APP, static_cast<LogLevel>(iLevel), 0xC0DE, TAG, "%{public}.*s", static_cast<int>(content.length()), content.data());
#elif defined(__APPLE__)
        _dynxx_log_apple(content.data());
#elif defined(__EMSCRIPTEN__)
        EM_ASM({
            var tag = UTF8ToString($0);
            var level = $1;
            var msg = UTF8ToString($2);
            var txt = tag + "_" + level + " -> " + msg;
            console.log(txt);
        }, TAG, iLevel, content.data());
#else
        std::cout << TAG << "_" << iLevel << " -> " << content << std::endl;
#endif
    }

#if defined(USE_SPDLOG)
    void spdLogSetLevel(DynXXLogLevelX level)
    {
        switch(level) {
            case Debug:
                spdlog::set_level(spdlog::level::debug);
                break;
            case Info:
                spdlog::set_level(spdlog::level::info);
                break;
            case Warn:
                spdlog::set_level(spdlog::level::warn);
                break;
            case Error:
                spdlog::set_level(spdlog::level::err);
                break;
            default:
                spdlog::set_level(spdlog::level::off);
        }
    }

    void spdLogPrepare() 
    {
        const auto rootPath = dynxxRootPath();
        if (!rootPath.has_value()) [[unlikely]] {
            return;
        }

        static std::once_flag flag;
        std::call_once(flag, [&dir = rootPath.value()]() {
            try 
            {
                const auto& logger = spdlog::daily_logger_mt(
                    "dynxx_spdlog",
                    dir + "/log.txt",
                    0,
                    0 
                );
                spdlog::set_default_logger(logger);
                spdlog::set_pattern("[%Y-%m-%d_%H:%M:%S.%e] [%l] [%t] %v");
                spdlog::flush_on(isDebug() ? spdlog::level::debug : spdlog::level::warn);
            }
            catch (const spdlog::spdlog_ex& ex) 
            {
                std::cerr << "SpdLog init failed: " << ex.what() << std::endl;
            }
        });

        spdLogSetLevel(_level);
    }

    void spdLogPrint(DynXXLogLevelX level, std::string_view content)
    {
        spdLogPrepare();
        switch(level) {
            case Debug:
                spdlog::debug(content);
                break;
            case Info:
                spdlog::info(content);
                break;
            case Warn:
                spdlog::warn(content);
                break;
            case Error:
                spdlog::error(content);
                break;
            default:
                break;
        }
    }
#endif

    void logPrint(DynXXLogLevelX level, std::string_view content)
    {
        if (_callback)
        {
            _callback(underlying(level), content.data());
        }
        else
        {
            stdLogPrint(level, content);
        }
    
#if defined(USE_SPDLOG)
        spdLogPrint(level, content);
#endif
    }

    void logPrintInBlocks(DynXXLogLevelX level, std::string_view content)
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

namespace DynXX::Core::Log {

void setLevel(DynXXLogLevelX level)
{
    if (level < Debug || level > None) [[unlikely]]
    {
        return;
    }

#if defined(USE_SPDLOG)
    spdLogPrepare();
#endif

    const auto lock = std::scoped_lock(_mutex);
    _level = level;
}

void setCallback(const std::function<void(int level, const char *content)> &callback)
{
    const auto lock = std::scoped_lock(_mutex);
    _callback = callback;
}

void print(DynXXLogLevelX level, std::string_view content)
{
    static std::once_flag flag;
    std::call_once(flag, []() {
        prepareStdIO();
    });

    const auto lock = std::scoped_lock(_mutex);

    if (level < _level || level < Debug || level >= None) [[unlikely]]
    {
        return;
    }

    logPrintInBlocks(level, content);
}

} // namespace DynXX::Core::Log
