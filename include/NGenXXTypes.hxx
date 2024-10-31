#ifndef NGENXX_INCLUDE_TYPES_HXX_
#define NGENXX_INCLUDE_TYPES_HXX_

#include "NGenXXTypes.h"

#ifdef __cplusplus

#include <utility>
#include <string>
#include <variant>
#include <limits>

constexpr long long kLongLongMin = std::numeric_limits<long long>::min();
constexpr long long kLongLongMax = std::numeric_limits<long long>::max();
constexpr double kDoubleMin = std::numeric_limits<double>::min();
constexpr double kDoubleMax = std::numeric_limits<double>::max();

using Bytes = std::pair<const byte *, const size_t>;
#define BytesEmpty {NULL, 0}

using Any = std::variant<int64_t, double, std::string>;

#endif

#endif // NGENXX_INCLUDE_TYPES_HXX_