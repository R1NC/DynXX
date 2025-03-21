#ifndef NGENXX_INCLUDE_LOG_HXX_
#define NGENXX_INCLUDE_LOG_HXX_

#include <string>
#include <functional>

#if defined(USE_STD_FORMAT)
#include <format>
#else
#include <tuple>
#include <sstream>
#endif

enum class NGenXXLogLevelX : int
{
    Debug = 3,
    Info,
    Warn,
    Error,
    Fatal,
    None
};

void ngenxxLogSetLevel(NGenXXLogLevelX level);

void ngenxxLogSetCallback(const std::function<void(int level, const char *content)> &callback);

void ngenxxLogPrint(NGenXXLogLevelX level, const std::string &content);

#if !defined(USE_STD_FORMAT)
template <typename... Args>
inline std::string _ngenxxLogFormat(const std::string &format, Args... args)
{
    std::ostringstream oss;
    
    auto formatWithArgs = [&oss, &format](auto... xArgs) {
        auto tmpFormat = format;
        ((oss << tmpFormat.substr(0, tmpFormat.find("{}")) << xArgs, tmpFormat.erase(0, tmpFormat.find("{}") + 2)), ...);
        oss << tmpFormat;
    };
    std::apply(formatWithArgs, std::make_tuple(args...));
    
    return oss.str();
}
#endif

template <typename... Args>
inline void ngenxxLogPrintF(NGenXXLogLevelX level, const std::string &format, Args... args)
{
#if !defined(USE_STD_FORMAT)
    auto fContent = _ngenxxLogFormat(format, args...);
#else
    auto fContent = std::vformat(format, std::make_format_args(args...));
#endif
    ngenxxLogPrint(level, fContent);
}

#endif // NGENXX_INCLUDE_LOG_HXX_