#ifndef NGENXX_INCLUDE_TYPES_HXX_
#define NGENXX_INCLUDE_TYPES_HXX_

#include "NGenXXTypes.h"

#ifdef __cplusplus

#include <string>
#include <vector>
#include <variant>

using Any = std::variant<int64_t, double, std::string>;
#define AnyEmpty {}

using Bytes = std::vector<byte>;
#define BytesEmpty {}

static inline const Bytes wrapBytes(const byte* data, const size_t len)
{
    return Bytes(data, data + len);
}

#endif

#endif // NGENXX_INCLUDE_TYPES_HXX_