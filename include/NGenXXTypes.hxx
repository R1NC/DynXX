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
#include <functional>
#include <limits>
#include <iostream>
#include <type_traits>

#pragma mark Concepts

template <typename T>
concept ConstT = std::is_const_v<T>;
    
template <typename T>
concept VoidT = std::is_void_v<T>;

template <typename T>
concept CharacterT =
    std::is_same_v<T, char> || std::is_same_v<T, wchar_t>
#ifdef __cpp_char8_t
    || std::is_same_v<T, char8_t>
#endif
    || std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t>;

template<class T, class U>
concept DerivedT = std::is_base_of<U, T>::value;

template<typename T>
concept PolymorphicT = std::is_polymorphic_v<T>;

template<typename T>
concept MemcpyableT = std::is_trivially_copyable_v<T>;

template<typename T>
concept HashableT = requires(T a) 
{
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept Comparable = requires(T a, T b) 
{
    { a < b } -> std::convertible_to<bool>;
};

template<typename T>
concept Iterable = requires(T a) 
{
    { std::begin(a) } -> std::input_iterator;
    { std::end(a) } -> std::sentinel_for<decltype(std::begin(a))>;
};

#pragma mark Utils using concept

template<PolymorphicT T>
T* dynamicCastX(void* ptr) 
{
    return dynamic_cast<T *>(ptr);
}

template<HashableT T>
std::size_t getHash(const T& value) 
{
    return std::hash<T>{}(value);
}

template<Iterable T>
void iterate(const T& container, std::function<void(const typename T::value_type&)> &&func) 
{
    for (const auto& e : container)
    {
        std::move(func)(e);
    }
}

template<MemcpyableT T>
void memcpyX(const T* src, T* dst, std::size_t count) 
{
    std::memcpy(dst, src, count * sizeof(T));
}

// malloc for character types
template <CharacterT T>
static inline T* mallocX(size_t count = 1)
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
requires (!CharacterT<T>)
static inline T* mallocX(size_t count = 1)
{
    auto ptr = std::calloc(count, sizeof(T));
    if (!ptr) [[unlikely]]
    {
        return nullptr;
    }
    return static_cast<T *>(ptr);
}

// free for non-const & non-void types
template <typename T>
requires (!ConstT<T> && !VoidT<T>)
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
requires (!ConstT<T> && VoidT<T>)
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
requires (ConstT<T> && !VoidT<T>)
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
requires (ConstT<T> && VoidT<T>)
static inline void freeX(T* &ptr)
{
    if (!ptr) [[unlikely]]
    {
        return;
    }
    std::free(const_cast<void *>(ptr));
    ptr = nullptr;
}

#pragma mark Pointer transform

template <typename T = void>
static inline T *addr2ptr(const address addr)
{
    return reinterpret_cast<T *>(addr);
}

template <typename T = void>
static inline address ptr2addr(const T *ptr)
{
    return reinterpret_cast<address>(ptr);
}

#pragma mark Any Type

constexpr auto MinInt64 = std::numeric_limits<int64_t>::min();
constexpr auto MaxInt64 = std::numeric_limits<int64_t>::max();
constexpr auto MinDouble = std::numeric_limits<double>::min();
constexpr auto MaxDouble = std::numeric_limits<double>::max();

using Any = std::variant<int64_t, double, std::string>;

static inline std::string Any2String(const Any &v, const std::string &defaultS = {})
{
    if (std::holds_alternative<std::string>(v))
    {
        return std::get<std::string>(v);
    }
    return defaultS;
}

static inline int64_t Any2Integer(const Any &v, int64_t defaultI = MinInt64)
{
    if (!std::holds_alternative<std::string>(v))
    {
        return std::get<int64_t>(v);
    }
    return defaultI;
}

static inline double Any2Float(const Any &v, double defaultF = MinDouble)
{
    if (!std::holds_alternative<std::string>(v))
    {
        return std::get<double>(v);
    }
    return defaultF;
}

#pragma mark Dict Type

using Dict = std::unordered_map<std::string, std::string>;
using DictAny = std::unordered_map<std::string, Any>;

static inline std::string dictAnyReadString(const DictAny &dict, const std::string &key, const std::string &defaultS = {})
{
    return dict.find(key) == dict.end() ? defaultS : Any2String(dict.at(key), defaultS);
}

static inline int64_t dictAnyReadInteger(const DictAny &dict, const std::string &key, int64_t defaultI = MinInt64)
{
    return dict.find(key) == dict.end() ? defaultI : Any2Integer(dict.at(key), defaultI);
}

static inline double dictAnyReadFloat(const DictAny &dict, const std::string &key, double defaultF = MinDouble)
{
    return dict.find(key) == dict.end() ? defaultF : Any2Float(dict.at(key), defaultF);
}

#pragma mark Bytes Type

using Bytes = std::vector<byte>;

static inline const Bytes wrapBytes(const byte* data, size_t len)
{
    return Bytes(data, data + len);
}

template <typename T>
std::string wrapStr(const T* ptr) 
{
    if (ptr == nullptr) 
    {
        return {};
    }
    return std::string(reinterpret_cast<const char*>(ptr));
}

#endif

#endif // NGENXX_INCLUDE_TYPES_HXX_
