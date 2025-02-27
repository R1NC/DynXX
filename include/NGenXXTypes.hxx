#ifndef NGENXX_INCLUDE_TYPES_HXX_
#define NGENXX_INCLUDE_TYPES_HXX_

#include "NGenXXTypes.h"

#ifdef __cplusplus

#include <cstdlib>

#include <string>
#include <vector>
#include <variant>
#include <unordered_map>
#include <limits>

static inline void *addr2ptr(const address addr)
{
    return reinterpret_cast<void *>(addr);
}

static inline address ptr2addr(const void *ptr)
{
    return reinterpret_cast<address>(ptr);
}

template <typename T>
static inline T* mallocPtr(const size_t count)
{
    auto len = count * sizeof(T) + 1;
    auto ptr = static_cast<T *>(std::malloc(len));
    std::memset(ptr, 0, len);
    return ptr;
}

template <typename T>
static inline void freePtr(T* &ptr)
{
    if (!ptr) [[unlikely]]
    {
        return;
    }
    std::free(const_cast<void *>(static_cast<const void *>(ptr)));
    ptr = nullptr;
}

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

using Dict = std::unordered_map<std::string, std::string>;
using DictAny = std::unordered_map<std::string, Any>;


using Bytes = std::vector<byte>;
#define BytesEmpty {}

static inline const Bytes wrapBytes(const byte* data, const size_t len)
{
    return Bytes(data, data + len);
}

#endif

#endif // NGENXX_INCLUDE_TYPES_HXX_