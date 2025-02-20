#ifndef NGENXX_SRC_CODING_HXX_
#define NGENXX_SRC_CODING_HXX_

#if defined(__cplusplus)

#include <NGenXXTypes.hxx>

namespace NGenXX
{
    namespace Coding
    {
        namespace Hex
        {

            std::string bytes2str(const Bytes &bytes);

            Bytes str2bytes(const std::string &str);
        }

        namespace Case
        {
            std::string upper(const std::string &str);

            std::string lower(const std::string &str);
        }

        std::string bytes2str(const Bytes &bytes);

        Bytes str2bytes(const std::string &str);
    }
}

#endif

#endif // NGENXX_SRC_CODING_HXX_