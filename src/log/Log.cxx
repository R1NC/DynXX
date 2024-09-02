#include <iostream>
#include <string.h>
#include <stdlib.h>

namespace NGenXX
{
    namespace Log
    {
        static void (*__callback)(int, const char *);

        void setLevel(int level)
        {
            // TODO
        }

        void setCallback(void (*callback)(int level, const char *content))
        {
            __callback = callback;
        }

        void print(int level, const char *content)
        {
            if (content == NULL)
                return;
            // TODO
            if (__callback)
            {
                char *c = (char *)malloc(strlen(content) + 1);
                strcpy(c, content);
                __callback(level, c);
            }
            else
            {
                std::cout << level << " | " << content << std::endl;
            }
        }
    }
}