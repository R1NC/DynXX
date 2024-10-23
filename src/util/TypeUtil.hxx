#ifndef NGENXX_UTIL_TYPE_H_
#define NGENXX_UTIL_TYPE_H_

#include <string>
#include <tuple>
#include <cstring>
#include "../NGenXX-Types.hxx"

static inline const char *str2charp(std::string s)
{
    const char *c = s.c_str();
    char *nc = reinterpret_cast<char *>(malloc(strlen(c) + 1));
    strcpy(nc, c);
    return nc;
}

static inline const byte *copyBytes(NGenXX::Bytes t)
{
    auto [cs, len] = t;
    if (cs == NULL || len <= 0) return NULL;
    const byte *ncs = reinterpret_cast<byte *>(malloc(len + 1));
    std::memset(reinterpret_cast<void *>(const_cast<byte *>(ncs)), 0, len + 1);
    std::memcpy(reinterpret_cast<void *>(const_cast<byte *>(ncs)), cs, len);
    return ncs;
}

static inline const NGenXX::Bytes trimBytes(NGenXX::Bytes bytes)
{
    auto [data, len] = bytes;
    auto fixedLen = len;
    for (auto i = len - 1; i > 0; i--)
    {
        if (data[i] <= 0) fixedLen--;
    }
    return {data, fixedLen};
}

#endif // NGENXX_UTIL_TYPE_H_