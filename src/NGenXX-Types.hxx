#ifndef NGENXX_TYPES_HXX_
#define NGENXX_TYPES_HXX_

#include "../include/NGenXXTypes.h"

#ifdef __cplusplus

#include <tuple>

namespace NGenXX
{
    typedef std::tuple<const byte *, const size> Bytes;
#define BytesEmpty {NULL, 0}
}

#endif

#endif // NGENXX_TYPES_HXX_