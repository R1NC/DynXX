#ifndef NGENXX_SRC_LOG_HXX_
#define NGENXX_SRC_LOG_HXX_

#if defined(__cplusplus)

#include <string>
#include <functional>

namespace NGenXX
{
    namespace Log
    {
        void setLevel(const int level);

        void setCallback(const std::function<void(const int level, const char *content)> &callback);

        void print(const int level, const std::string &content);
    }
}

#endif
#endif // NGENXX_SRC_LOG_HXX_