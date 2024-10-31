#ifndef NGENXX_UTIL_TYPE_H_
#define NGENXX_UTIL_TYPE_H_

#include <string>
#include <vector>
#include <tuple>
#include <cstring>
#include "../../include/NGenXXTypes.hxx"

static inline const char *copyStr(std::string s)
{
    const char *c = s.c_str();
    char *nc = reinterpret_cast<char *>(malloc(strlen(c) + 1));
    strcpy(nc, c);
    return nc;
}

static inline const char **copyStrVector(std::vector<std::string> sv, const size_t strMaxLen)
{
    char **sArr = reinterpret_cast<char **>(malloc(sizeof(char *) * sv.size() + 1));
    for (int i = 0; i < sv.size(); i++)
    {
        sArr[i] = reinterpret_cast<char *>(malloc(sizeof(char) * strMaxLen + 1));
        strcpy(sArr[i], sv[i].c_str());
    }
    return const_cast<const char **>(sArr);
}

static inline const byte *copyBytes(Bytes t)
{
    auto [cs, len] = t;
    if (cs == NULL || len <= 0)
        return NULL;
    const byte *ncs = reinterpret_cast<byte *>(malloc(len + 1));
    std::memset(reinterpret_cast<void *>(const_cast<byte *>(ncs)), 0, len + 1);
    std::memcpy(reinterpret_cast<void *>(const_cast<byte *>(ncs)), cs, len);
    return ncs;
}

static inline const Bytes trimBytes(Bytes bytes)
{
    auto [data, len] = bytes;
    auto fixedLen = len;
    for (auto i = len - 1; i > 0; i--)
    {
        if (data[i] <= 0)
            fixedLen--;
    }
    return {data, fixedLen};
}

#endif // NGENXX_UTIL_TYPE_H_