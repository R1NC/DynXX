#ifndef NGENXX_LOG_HXX_
#define NGENXX_LOG_HXX_

#ifdef __cplusplus

#include <string>

namespace NGenXX
{
    namespace Log
    {
        void setLevel(int level);

        void setCallback(void (*callback)(int level, const char *content));

        void print(int level, const std::string &content);
    }
}

#endif
#endif // NGENXX_LOG_HXX_