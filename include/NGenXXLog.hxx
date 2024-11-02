#ifndef NGENXX_INCLUDE_LOG_HXX_
#define NGENXX_INCLUDE_LOG_HXX_

#include <string>
#include <functional>

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

#endif // NGENXX_INCLUDE_LOG_HXX_