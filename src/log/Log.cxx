#include <iostream>
#include <string.h>
#include <stdlib.h>

namespace NGenXX
{
    namespace Log
    {
        static void (*__callback)(int, const char *);

        static int __level;

        void setLevel(int level)
        {
            if (level < 0 || level > 4) return;
            __level = level;
        }

        void setCallback(void (*callback)(int level, const char *content))
        {
            __callback = callback;
        }

        void print(int level, const char *content)
        {
            if (content == NULL || level > __level)
                return;
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