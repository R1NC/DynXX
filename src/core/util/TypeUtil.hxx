#pragma once

#include <DynXX/CXX/Types.hxx>
#include <algorithm>

namespace DynXX::Core::Util::Type {

    inline const char **dupCharsArray(const std::vector<std::string> &sv, const size_t strMaxLen) {
        auto size = sv.size();
        const auto sArr = mallocX<char *>(size);
        for (decltype(size) i = 0; i < size; i++) {
            const auto len = std::min(sv[i].size(), strMaxLen);
            sArr[i] = mallocX<char>(len);
            std::memcpy(sArr[i], sv[i].c_str(), len);
        }
        return const_cast<const char **>(sArr);
    }

    inline byte *dupBytes(BytesView t) {
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
