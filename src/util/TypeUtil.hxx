#ifndef NGENXX_UTIL_TYPE_H_
#define NGENXX_UTIL_TYPE_H_

#include <string>
#include <tuple>

static inline const char *str2charp(std::string s)
{
    const char *c = s.c_str();
    char *nc = (char *)malloc(strlen(c) + 1);
    strcpy(nc, c);
    return nc;
}

static inline const unsigned char *copyBytes(std::tuple<const unsigned char *, const unsigned int> t)
{
    const unsigned char *cs = std::get<0>(t);
    int len = std::get<1>(t);
    if (cs == NULL || len <= 0) return NULL;
    const unsigned char *ncs = (unsigned char *)malloc(len + 1);
    memset((void *)ncs, 0, len + 1);
    memcpy((void *)ncs, cs, len);
    return ncs;
}

#endif // NGENXX_UTIL_TYPE_H_