#ifndef ENGINEXX_LOG_H_
#define ENGINEXX_LOG_H_

#ifdef __cplusplus

namespace EngineXX
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
#endif // ENGINEXX_LOG_H_