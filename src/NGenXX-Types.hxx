#ifndef NGENXX_TYPES_HXX_
#define NGENXX_TYPES_HXX_

#include "../include/NGenXXTypes.h"

#ifdef __cplusplus

#include <tuple>
#include <string>
#include <variant>

namespace NGenXX
{
    using Bytes = std::tuple<const byte *, const size>;
    #define BytesEmpty {NULL, 0}

    using Any = std::variant<long long, double, std::string>;
}

#endif

#endif // NGENXX_TYPES_HXX_