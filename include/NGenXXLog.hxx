#ifndef NGENXX_INCLUDE_LOG_HXX_
#define NGENXX_INCLUDE_LOG_HXX_

#include <functional>

#include "NGenXXMacro.hxx"
#include "NGenXXTypes.hxx"

#if defined(USE_STD_FORMAT)
#include <format>
#else
#include <tuple>
#include <sstream>
#endif

enum class NGenXXLogLevelX : int {
    Debug = 3,
    Info,
    Warn,
    Error,
    Fatal,
    None
};

void ngenxxLogSetLevel(NGenXXLogLevelX level);

void ngenxxLogSetCallback(const std::function<void(int level, const char *content)> &callback);

void ngenxxLogPrint(NGenXXLogLevelX level, const std::string_view &content);

#if !defined(USE_STD_FORMAT)
template <typename... Args>
inline std::string _ngenxxLogFormat(const std::string &format, Args... args)
{
    std::ostringstream oss;
    
    auto formatWithArgs = [&oss, &format](auto... xArgs) {
        auto tmpFormat = format;
        constexpr auto flag = "{}";
        ((oss << tmpFormat.substr(0, tmpFormat.find(flag)) << xArgs, tmpFormat.erase(0, tmpFormat.find(flag) + 2)), ...);
        oss << tmpFormat;
    };
    std::apply(formatWithArgs, std::make_tuple(args...));
    
    return oss.str();
}
#endif

template<typename... Args>
void ngenxxLogPrintF(NGenXXLogLevelX level, const std::string &format, Args... args) {
    auto fContent =
#if !defined(USE_STD_FORMAT)
    _ngenxxLogFormat(format, args...)
#else
                    std::vformat(format, std::make_format_args(args...))
#endif
            ;
    ngenxxLogPrint(level, fContent);
}

#endif // NGENXX_INCLUDE_LOG_HXX_
