#ifndef NGENXX_SRC_CODING_HXX_
#define NGENXX_SRC_CODING_HXX_

#if defined(__cplusplus)

#include <string_view>

#include <NGenXXTypes.hxx>

namespace NGenXX::Coding
    {
        namespace Case
        {
            std::string upper(const std::string_view &str);

            std::string lower(const std::string_view &str);
        }
        
        namespace Hex
        {
            std::string bytes2str(const Bytes &bytes);

            Bytes str2bytes(const std::string &str);
        }

        std::string bytes2str(const Bytes &bytes);

        Bytes str2bytes(const std::string_view &str);

        std::string strTrim(const std::string_view &str);
}

#endif

#endif // NGENXX_SRC_CODING_HXX_