#pragma once

#include <cstdio>
#include <functional>

#include "Macro.hxx"
#include "Types.hxx"

#if defined(USE_STD_FORMAT)
#include <format>
#else
#include <tuple>
#include <sstream>
#endif

enum class DynXXLogLevelX : uint8_t {
    Debug = 3,
    Info,
    Warn,
    Error,
    Fatal,
    None
};

void dynxxLogSetLevel(DynXXLogLevelX level);

void dynxxLogSetCallback(const std::function<void(int level, const char *content)> &callback);

void dynxxLogPrint(DynXXLogLevelX level, std::string_view content);

#if !defined(USE_STD_FORMAT)
template <typename... Args>
std::string dynxxLogFormatT(std::string_view format, Args... args)
{
    std::ostringstream oss;
    
    auto formatWithArgs = [&oss, &format](auto... xArgs) {
        std::string tmpFormat{format};
        constexpr auto flag = "{}";
        ((oss << tmpFormat.substr(0, tmpFormat.find(flag)) << xArgs, tmpFormat.erase(0, tmpFormat.find(flag) + 2)), ...);
        oss << tmpFormat;
    };
    std::apply(formatWithArgs, std::make_tuple(args...));
    
    return oss.str();
}
#endif

template<typename... Args>
void dynxxLogPrintF(DynXXLogLevelX level, std::string_view format, Args... args) {
    auto fContent =
#if !defined(USE_STD_FORMAT)
    dynxxLogFormatT(format, args...)
#else
    std::vformat(std::string{format}, std::make_format_args(args...))
#endif
    ;
    dynxxLogPrint(level, fContent);
}
