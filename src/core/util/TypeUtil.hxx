#ifndef NGENXX_SRC_CORE_UTIL_TYPEUTIL_HXX_
#define NGENXX_SRC_CORE_UTIL_TYPEUTIL_HXX_

#include <NGenXXTypes.hxx>

namespace NGenXX::Core::Util::Type {
    inline const char *copyStr(const std::string &s) {
        const char *c = s.c_str();
        const auto len = s.length();
        const auto nc = mallocX<char>(len);
        std::strncpy(nc, c, len);
        return nc;
    }

    inline char *const *copyStrVector(const std::vector<std::string> &sv, const size_t strMaxLen) {
        auto size = sv.size();
        const auto sArr = mallocX<char *>(size);
        for (decltype(size) i = 0; i < size; i++) {
            sArr[i] = mallocX<char>(strMaxLen);
            std::strncpy(sArr[i], sv[i].c_str(), strMaxLen);
        }
        return sArr;
    }

    inline byte *copyBytes(const Bytes &t) {
        const auto cs = t.data();
        const auto len = t.size();
        if (cs == nullptr || len == 0) [[unlikely]] {
            return nullptr;
        }
        const auto ncs = mallocX<byte>(len);
        std::memcpy(ncs, cs, len);
        return ncs;
    }
}

#endif // NGENXX_SRC_CORE_UTIL_TYPEUTIL_HXX_
