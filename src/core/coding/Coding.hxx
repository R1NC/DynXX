#ifndef NGENXX_SRC_CORE_CODING_HXX_
#define NGENXX_SRC_CORE_CODING_HXX_

#if defined(__cplusplus)

#include <NGenXXTypes.hxx>

namespace NGenXX::Core::Coding {
    namespace Case {
        std::string upper(std::string_view str);

        std::string lower(std::string_view str);
    }

    namespace Hex {
        std::string bytes2str(BytesView bytes);

        Bytes str2bytes(const std::string &str);
    }

    std::string bytes2str(BytesView bytes);

    Bytes str2bytes(std::string_view str);

    std::string strTrim(std::string_view str);

    std::string strEscapeQuotes(const std::string_view& str);
}

#endif

#endif // NGENXX_SRC_CORE_CODING_HXX_
