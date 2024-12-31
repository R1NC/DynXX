#ifndef NGENXX_INCLUDE_LOG_HXX_
#define NGENXX_INCLUDE_LOG_HXX_

#include <string>
#include <sstream>
#include <functional>

#if defined(USE_STD_FORMAT)
#include <format>
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

void ngenxxLogSetLevel(const NGenXXLogLevelX level);

void ngenxxLogSetCallback(const std::function<void(const int level, const char *content)> &callback);

void ngenxxLogPrint(const NGenXXLogLevelX level, const std::string &content);

#if !defined(USE_STD_FORMAT)
inline void _ngenxxLogFormatImpl(std::ostringstream &oss, const std::string &format)
{
    oss << format;
}

template <typename T, typename... Args>
inline void _ngenxxLogFormatImpl(std::ostringstream &oss, const std::string &format, T value, Args... args)
{
    auto argPos = format.find("{}");
    if (argPos != std::string::npos)
    {
        oss << format.substr(0, argPos) << value;
        _ngenxxLogFormatImpl(oss, format.substr(argPos + 2), args...);
    }
    else
    {
        oss << format;
    }
}

template <typename... Args>
inline std::string _ngenxxLogFormat(const std::string &format, Args... args)
{
    std::ostringstream oss;
    _ngenxxLogFormatImpl(oss, format, args...);
    return oss.str();
}
#endif

template <typename... Args>
inline void ngenxxLogPrintF(const NGenXXLogLevelX level, const std::string &format, Args... args)
{
#if !defined(USE_STD_FORMAT)
    auto fContent = _ngenxxLogFormat(format, args...);
#else
    auto fContent = std::vformat(format, std::make_format_args(args...));
#endif
    ngenxxLogPrint(level, fContent);
}

#endif // NGENXX_INCLUDE_LOG_HXX_