#ifndef DYNXX_SRC_CORE_CODING_HXX_
#define DYNXX_SRC_CORE_CODING_HXX_

#if defined(__cplusplus)

#include <DynXX/CXX/Types.hxx>

namespace DynXX::Core::Coding {
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

#endif // DYNXX_SRC_CORE_CODING_HXX_
