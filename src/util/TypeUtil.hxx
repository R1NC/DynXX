#ifndef NGENXX_SRC_UTIL_TYPE_HXX_
#define NGENXX_SRC_UTIL_TYPE_HXX_

#include <cstdlib>
#include <cstring>

#include <NGenXXTypes.hxx>

static inline char *copyStr(const std::string &s)
{
    const char *c = s.c_str();
    auto len = std::strlen(c);
    auto nc = mallocX<char>(len);
    std::strncpy(nc, c, len);
    return nc;
}

static inline char **copyStrVector(const std::vector<std::string> &sv, size_t strMaxLen)
{
    auto sArr = mallocX<char *>(sv.size());
    for (size_t i = 0; i < sv.size(); i++)
    {
        sArr[i] = mallocX<char>(strMaxLen);
        std::strncpy(sArr[i], sv[i].c_str(), strMaxLen);
    }
    return sArr;
}

static inline byte *copyBytes(const Bytes &t)
{
    auto cs = t.data();
    auto len = t.size();
    if (cs == nullptr || len == 0) [[unlikely]]
    {
        return nullptr;
    }
    auto ncs = mallocX<byte>(len);
    std::memcpy(static_cast<void *>(ncs), cs, len);
    return ncs;
}

#endif // NGENXX_SRC_UTIL_TYPE_HXX_