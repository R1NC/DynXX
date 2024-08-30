#include <iostream>

namespace NGenXX
{
    namespace Log
    {
        static void (*__callback)(int, const char *);

        void setLevel(int level) 
        {
            //TODO
        }

        void setCallback(void (*callback)(int level, const char *content))
        {
            __callback = callback;
        }

        void print(int level, const char *content)
        {
            //TODO
            if (__callback) {
                __callback(level, content);
            } else {
                std::cout << level << " | " << content << std::endl;
            }
        }
    }
}