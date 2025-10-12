#pragma once

#include <DynXX/CXX/Types.hxx>

namespace DynXX::Core::Coding {
    namespace Case {
        std::string upper(std::string_view str);

        std::string lower(std::string_view str);
    }

    namespace Hex {
        std::string bytes2str(BytesView bytes);

        Bytes str2bytes(std::string_view str);
    }

    std::string bytes2str(BytesView bytes);

    Bytes str2bytes(std::string_view str);

    std::string strTrim(std::string_view str);

    std::string strEscapeQuotes(std::string_view str);
}
