#ifndef DYNXX_INCLUDE_TYPES_HXX_
#define DYNXX_INCLUDE_TYPES_HXX_

#include "../C/Types.h"

#include <string.h>

#ifdef __cplusplus

#include <cstring>
#include <string_view>
#include <string>
#include <optional>
#include <span>
#include <vector>
#include <variant>
#include <unordered_map>
#include <functional>
#include <limits>
#include <type_traits>

// Assertions

#define AssertMove(T) \
    static_assert(std::is_move_constructible_v<T>, #T " MUST BE MOVE CONSTRUCTIBLE!"); \
    static_assert(std::is_move_assignable_v<T>, #T " MUST BE MOVE ASSIGNABLE!"); \
    static_assert(std::is_nothrow_move_constructible_v<T>, #T " MUST BE NOTHROW MOVE CONSTRUCTIBLE!"); \
    static_assert(std::is_nothrow_move_assignable_v<T>, #T " MUST BE NOTHROW MOVE ASSIGNABLE!")

// Concepts

template<typename T>
concept ConstT = std::is_const_v<T>;

template<typename T>
concept VoidT = std::is_void_v<T>;

template<typename T>
concept NumberT =
        std::is_arithmetic_v<T> ||
        (std::is_enum_v<T> && std::is_convertible_v<std::underlying_type_t<T>, int>);

template<typename T>
concept IntegerT = NumberT<T> && std::integral<T>;

template<typename T>
concept FloatT = NumberT<T> && std::floating_point<T>;

template<typename T>
concept EnumT = NumberT<T> && std::is_enum_v<T>;

template<typename T>
concept CharacterT =
        std::is_same_v<T, char> || std::is_same_v<T, unsigned char> || std::is_same_v<T, signed char>
#ifdef __cpp_char8_t
        || std::is_same_v<T, char8_t>
#endif
        || std::is_same_v<T, char16_t> || std::is_same_v<T, char32_t> || std::is_same_v<T, wchar_t>;

template<typename T>
concept CStringT = std::is_same_v<T, const char*> || std::is_same_v<T, char*>;

template<typename T>
concept KeyType = std::convertible_to<T, std::string_view> || 
                  std::convertible_to<T, std::string> ||
                  std::convertible_to<T, const char*>;

template<class T, class U>
concept DerivedT = std::is_base_of_v<U, T>;

template<typename To, typename From>
concept PolymorphicConvertible = 
    std::is_polymorphic_v<From> && std::is_polymorphic_v<To> &&
    (std::is_base_of_v<To, From> || std::is_base_of_v<From, To> || requires(From* f) {
        dynamic_cast<To*>(f); 
    });

template<typename T>
concept MemcpyableT = std::is_trivially_copyable_v<T>;

template<typename T>
concept HashableT = requires(T a)
{
    { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept CompletelyComparableT = requires(T a, T b)
{
    { a <=> b } -> std::convertible_to<std::partial_ordering>;
};

template<typename T>
concept EqualityComparableT = requires(T a, T b)
{
    { a == b } -> std::convertible_to<bool>;
    { a != b } -> std::convertible_to<bool>;
};

template<typename T>
concept IterableT = requires(T a)
{
    { std::begin(a) } -> std::input_iterator;
    { std::end(a) } -> std::sentinel_for<decltype(std::begin(a))>;
};

// Utils using concept

template<typename To, typename From>
requires PolymorphicConvertible<To, From>
To* dynamicCastX(From* ptr) noexcept {
    return dynamic_cast<To*>(ptr);
}

template<HashableT T>
std::size_t getHash(const T &value) {
    return std::hash<T>{}(value);
}

template<IterableT T>
void iterate(const T &container, std::function<void(const typename T::value_type &)> &&func) {
    for (const auto &e: container) {
        std::move(func)(e);
    }
}

// Limits constants

template<NumberT T>
constexpr T MinV() {
    return (std::numeric_limits<T>::min)();
}

template<NumberT T>
constexpr T MaxV() {
    return (std::numeric_limits<T>::max)();
}

constexpr auto MinInt32 = MinV<int32_t>();
constexpr auto MaxInt32 = MaxV<int32_t>();
constexpr auto MinInt64 = MinV<int64_t>();
constexpr auto MaxInt64 = MaxV<int64_t>();
constexpr auto MinFloat32 = MinV<float>();
constexpr auto MaxFloat32 = MaxV<float>();
constexpr auto MinFloat64 = MinV<double>();
constexpr auto MaxFloat64 = MaxV<double>();
constexpr auto MinFloat128 = MinV<long double>();
constexpr auto MaxFloat128 = MaxV<long double>();

// String to Number

int32_t str2int32(const std::string &str, const int32_t defaultI = MinInt32);

int64_t str2int64(const std::string &str, const int64_t defaultI = MinInt64);

float str2float32(const std::string &str, const float defaultF = MinFloat32);

double str2float64(const std::string &str, const double defaultF = MinFloat64);

long double str2float128(const std::string &str, const long double defaultF = MinFloat128);

// Any Type

using Num = std::variant<int64_t, double>;

using Any = std::variant<int64_t, double, std::string>;

inline auto Any2String(const Any &v, const std::string &defaultS = {}) {
    if (std::holds_alternative<std::string>(v)) [[likely]] {
        return std::get<std::string>(v);
    }
    if (std::holds_alternative<int64_t>(v)) [[unlikely]] {
        return std::to_string(std::get<int64_t>(v));
    }
    if (std::holds_alternative<double>(v)) [[unlikely]] {
        return std::to_string(std::get<double>(v));
    }
    return defaultS;
}

inline auto Any2Integer(const Any &a, const int64_t defaultI = MinInt64) {
    if (!std::holds_alternative<std::string>(a)) [[likely]] {
        return std::get<int64_t>(a);
    }
    return str2int64(std::get<std::string>(a), defaultI);
}

inline auto Any2Float(const Any &a, const double defaultF = MinFloat64) {
    if (!std::holds_alternative<std::string>(a)) [[likely]] {
        return std::get<double>(a);
    }
    return str2float64(std::get<std::string>(a), defaultF);
}

// Dict Type

#if !defined(_WIN32) && defined(__cpp_lib_generic_unordered_lookup)
// Custom transparent hasher for string-like types
struct TransparentStringHash {
    using is_transparent = void;//enable transparent hasher
    template<KeyType T>
    std::size_t operator()(const T& key) const {
        return std::hash<std::string_view>{}(key);
    }
};
struct TransparentEqual {
    using is_transparent = void;
    template<typename T1, typename T2>
    bool operator()(const T1& lhs, const T2& rhs) const noexcept {
        if constexpr (CompletelyComparableT<T1> && CompletelyComparableT<T2>) {
            return (lhs <=> rhs) == 0;
        } else if constexpr (EqualityComparableT<T1> && EqualityComparableT<T2>) {
            return lhs == rhs;
        } else {
            static_assert(sizeof(T1) == 0, "Types must be comparable");
        }
    }
};
using Dict = std::unordered_map<std::string, std::string, TransparentStringHash, TransparentEqual>;
using DictAny = std::unordered_map<std::string, Any, TransparentStringHash, TransparentEqual>;
using DictKeyType = std::string_view;
#else
using Dict = std::unordered_map<std::string, std::string>;
using DictAny = std::unordered_map<std::string, Any>;
using DictKeyType = std::string&;
#endif

inline std::optional<std::string> dictAnyReadString(const DictAny &dict, const DictKeyType key) {
    auto it = dict.find(key);
    if (it == dict.end()) [[unlikely]] {
        return std::nullopt;
    }
    return {Any2String(it->second)};
}

inline auto dictAnyReadString(const DictAny &dict, const DictKeyType key, const std::string &defaultS) {
    return dictAnyReadString(dict, key).value_or(defaultS);
}

inline std::optional<int64_t> dictAnyReadInteger(const DictAny &dict, const DictKeyType key) {
    auto it = dict.find(key);
    if (it == dict.end()) [[unlikely]] {
        return std::nullopt;
    }
    return {Any2Integer(it->second)};
}

inline auto dictAnyReadInteger(const DictAny &dict, const DictKeyType key, const int64_t defaultI) {
    return dictAnyReadInteger(dict, key).value_or(defaultI);
}

inline std::optional<double> dictAnyReadFloat(const DictAny &dict, const DictKeyType key) {
    auto it = dict.find(key);
    if (it == dict.end()) [[unlikely]] {
        return std::nullopt;
    }
    return {Any2Float(it->second)};
}

inline auto dictAnyReadFloat(const DictAny &dict, const DictKeyType key, const double defaultF) {
    return dictAnyReadFloat(dict, key).value_or(defaultF);
}

// Bytes Type

using BytesView = std::span<const byte>;
using Bytes = std::vector<byte>;

inline BytesView makeBytesView(const byte *data, const size_t len) {
    if (data == nullptr || len == 0) [[unlikely]] {
        return {};
    }
    return {data, len};
}

inline Bytes makeBytes(const byte *data, const size_t len) {
    if (data == nullptr || len == 0) [[unlikely]] {
        return {};
    }
    return {data, data + len};
}

template<CharacterT T>
std::string makeStr(const T *ptr) {
    if (ptr == nullptr) [[unlikely]] {
        return {};
    }
    return {reinterpret_cast<const char *>(ptr)};
}

// C String Utils

inline std::optional<const char*> nullTerminatedCStr(std::string_view sv) {
    if (sv.empty() || !sv.back()) [[unlikely]] {
        return std::nullopt;
    }
    return sv.data();
}

#endif

#endif // DYNXX_INCLUDE_TYPES_HXX_
