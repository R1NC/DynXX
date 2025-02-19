#ifndef NGENXX_INCLUDE_TYPES_HXX_
#define NGENXX_INCLUDE_TYPES_HXX_

#include "NGenXXTypes.h"

#ifdef __cplusplus

#include <string>
#include <vector>
#include <variant>
#include <limits>

using Any = std::variant<int64_t, double, std::string>;
#define AnyEmpty {}

static inline std::string Any2String(const Any &v)
{
    std::string s;
    if (std::holds_alternative<std::string>(v))
    {
        s = std::move(std::get<std::string>(v));
    }
    return s;
}

static inline int64_t Any2Integer(const Any &v)
{
    if (!std::holds_alternative<std::string>(v))
    {
        return std::get<int64_t>(v);
    }
    return std::numeric_limits<int64_t>::min();
}

static inline double Any2Float(const Any &v)
{
    if (!std::holds_alternative<std::string>(v))
    {
        return std::get<double>(v);
    }
    return std::numeric_limits<double>::min();
}



using Bytes = std::vector<byte>;
#define BytesEmpty {}

static inline const Bytes wrapBytes(const byte* data, const size_t len)
{
    return Bytes(data, data + len);
}

#endif

#endif // NGENXX_INCLUDE_TYPES_HXX_