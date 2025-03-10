#ifndef NGENXX_INCLUDE_TYPES_HXX_
#define NGENXX_INCLUDE_TYPES_HXX_

#include "NGenXXTypes.h"
#include <cstring>

#ifdef __cplusplus

#include <cstdlib>

#include <string>
#include <vector>
#include <variant>
#include <unordered_map>
#include <limits>
#include <type_traits>

#pragma mark Pointer

static inline void *addr2ptr(const address addr)
{
    return reinterpret_cast<void *>(addr);
}

static inline address ptr2addr(const void *ptr)
{
    return reinterpret_cast<address>(ptr);
}

#pragma mark Memory

template <typename T>
concept CharacterType =
    std::is_same_v<T, char> || std::is_same_v<T, wchar_t>
#ifdef __cpp_char8_t
    || std::is_same_v<T, char8_t>
#endif
    || std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>;

// malloc for character types
template <CharacterType T>
static inline T* mallocX(size_t count)
{
    auto len = count * sizeof(T) + 1;
    auto ptr = std::malloc(len);
    if (!ptr) [[unlikely]]
    {
        return nullptr;
    }
    std::memset(ptr, 0, len);
    return static_cast<T *>(ptr);
}

// malloc for non-character types
template <typename T>
requires (!CharacterType<T>)
static inline T* mallocX(size_t count)
{
    auto ptr = std::calloc(count, sizeof(T));
    if (!ptr) [[unlikely]]
    {
        return nullptr;
    }
    return static_cast<T *>(ptr);
}


template <typename T>
concept ConstType = std::is_const_v<T>;

template <typename T>
concept VoidType = std::is_void_v<T>;

// free for non-const & non-void types
template <typename T>
requires (!ConstType<T> && !VoidType<T>)
static inline void freeX(T* &ptr)
{
    if (!ptr) [[unlikely]]
    {
        return;
    }
    std::free(static_cast<void *>(ptr));
    ptr = nullptr;
}

// free for non-const & void types
template <typename T>
requires (!ConstType<T> && VoidType<T>)
static inline void freeX(T* &ptr)
{
    if (!ptr) [[unlikely]]
    {
        return;
    }
    std::free(ptr);
    ptr = nullptr;
}

// free for const & non-void types
template <typename T>
requires (ConstType<T> && !VoidType<T>)
static inline void freeX(T* &ptr)
{
    if (!ptr) [[unlikely]]
    {
        return;
    }
    std::free(const_cast<void *>(static_cast<const void *>(ptr)));
    ptr = nullptr;
}

// free for const & void types
template <typename T>
requires (ConstType<T> && VoidType<T>)
static inline void freeX(T* &ptr)
{
    if (!ptr) [[unlikely]]
    {
        return;
    }
    std::free(const_cast<void *>(ptr));
    ptr = nullptr;
}

#pragma mark Any

using Any = std::variant<int64_t, double, std::string>;

static inline std::string Any2String(const Any &v)
{
    std::string s;
    if (std::holds_alternative<std::string>(v))
    {
        s = std::get<std::string>(v);
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


#pragma mark Bytes

using Bytes = std::vector<byte>;

static inline const Bytes wrapBytes(const byte* data, size_t len)
{
    return Bytes(data, data + len);
}

#endif

#endif // NGENXX_INCLUDE_TYPES_HXX_
