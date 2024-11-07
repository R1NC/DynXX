#ifndef NGENXX_INCLUDE_TYPES_HXX_
#define NGENXX_INCLUDE_TYPES_HXX_

#include "NGenXXTypes.h"

#ifdef __cplusplus

#include <string>
#include <vector>
#include <variant>
#include <limits>

constexpr long long kLongLongMin = std::numeric_limits<long long>::min();
constexpr long long kLongLongMax = std::numeric_limits<long long>::max();
constexpr double kDoubleMin = std::numeric_limits<double>::min();
constexpr double kDoubleMax = std::numeric_limits<double>::max();

using Any = std::variant<int64_t, double, std::string>;

using Bytes = std::vector<byte>;
#define BytesEmpty {}

static inline const Bytes wrapBytes(const byte* data, const size_t len)
{
    return Bytes(data, data + len);
}

#endif

#endif // NGENXX_INCLUDE_TYPES_HXX_