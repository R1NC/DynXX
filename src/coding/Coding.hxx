#ifndef NGENXX_SRC_CODING_HXX_
#define NGENXX_SRC_CODING_HXX_

#if defined(__cplusplus)

#include "../../include/NGenXXTypes.hxx"

namespace NGenXX
{
    namespace Coding
    {
        namespace Hex
        {

            const std::string bytes2str(const Bytes &bytes);

            const Bytes str2bytes(const std::string &str);
        }

        namespace Case
        {
            const std::string upper(const std::string &str);

            const std::string lower(const std::string &str);
        }

        const std::string bytes2str(const Bytes &bytes);

        const Bytes str2bytes(const std::string &str);
    }
}

#endif

#endif // NGENXX_SRC_CODING_HXX_