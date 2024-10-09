#ifndef NGENXX_CODING_HXX_
#define NGENXX_CODING_HXX_

#ifdef __cplusplus

#include "../NGenXX-Types.hxx"

namespace NGenXX 
{
    namespace Coding
    {
        namespace Hex 
        {

            const std::string bytes2str(const Bytes bytes);

            const Bytes str2bytes(const std::string &str);
        }
    }
}

#endif

#endif // NGENXX_CODING_HXX_