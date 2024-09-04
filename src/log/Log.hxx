#ifndef NGENXX_LOG_HXX_
#define NGENXX_LOG_HXX_

#ifdef __cplusplus

namespace NGenXX
{
    namespace Log
    {
        /**
         *
         */
        void setLevel(int level);

        /**
         *
         */
        void setCallback(void (*callback)(int level, const char *content));

        /**
         *
         */
        void print(int level, const char *content);
    }
}

#endif
#endif // NGENXX_LOG_HXX_