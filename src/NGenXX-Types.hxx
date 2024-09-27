#ifndef NGENXX_TYPES_HXX_
#define NGENXX_TYPES_HXX_

#include "../include/NGenXXTypes.h"

#ifdef __cplusplus

#include <tuple>
#include <string>
#include <variant>

namespace NGenXX
{
    constexpr long long kLongLongMin = std::numeric_limits<long long>::min();
    constexpr long long kLongLongMax = std::numeric_limits<long long>::max();
    constexpr double kDoubleMin = std::numeric_limits<double>::min();
    constexpr double kDoubleMax = std::numeric_limits<double>::max();

    using Bytes = std::tuple<const byte *, const size>;
#define BytesEmpty {NULL, 0}

    using Any = std::variant<long long, double, std::string>;
}

#endif

#endif // NGENXX_TYPES_HXX_