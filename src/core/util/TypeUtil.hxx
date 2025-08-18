#ifndef DYNXX_SRC_CORE_UTIL_TYPEUTIL_HXX_
#define DYNXX_SRC_CORE_UTIL_TYPEUTIL_HXX_

#include <DynXX/CXX/Types.hxx>
#include <algorithm>

namespace DynXX::Core::Util::Type {

    inline std::optional<const char*> nullTerminatedCStr(std::string_view sv) {
        if (sv.empty() || sv.back() != '\0') [[unlikely]] {
            return std::nullopt;
        }
        return sv.data();
    }

    inline const char *copyStr(const std::string_view s) {
        if (s.empty()) [[unlikely]] {
            return nullptr;
        }
        if (auto cstr = nullTerminatedCStr(s); cstr.has_value()) [[likely]] {
            return strdup(cstr.value());
        }
        std::string tmpStr{s};
        return strdup(tmpStr.c_str());
    }

    inline char *const *copyStrVector(const std::vector<std::string> &sv, const size_t strMaxLen) {
        auto size = sv.size();
        const auto sArr = mallocX<char *>(size);
        for (decltype(size) i = 0; i < size; i++) {
            const auto len = std::min(sv[i].size(), strMaxLen);
            sArr[i] = mallocX<char>(len);
            std::memcpy(sArr[i], sv[i].c_str(), len);
        }
        return sArr;
    }

    inline byte *copyBytes(const BytesView t) {
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

#endif // DYNXX_SRC_CORE_UTIL_TYPEUTIL_HXX_
