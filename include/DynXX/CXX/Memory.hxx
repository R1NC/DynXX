#ifndef DYNXX_INCLUDE_MEMORY_HXX_
#define DYNXX_INCLUDE_MEMORY_HXX_

#if defined(_WIN32)
#include <malloc.h>
#endif

#ifdef __cplusplus

#include "Types.hxx"

// C String Utils

inline const char *dupCStr(const char *cstr, [[maybe_unused]] size_t len) {
    if (cstr == nullptr) [[unlikely]] {
        return nullptr;
    }
#if defined(_WIN32)
    return _strdup(cstr);
#else
    return strndup(cstr, len);
#endif
}

inline const char *dupStr(std::string_view sv) {
    if (sv.empty()) [[unlikely]] {
        return nullptr;
    }
    if (auto cstr = nullTerminatedCStr(sv); !cstr.has_value()) [[unlikely]] {
        std::string tmpStr{sv};
        return dupCStr(tmpStr.c_str(), tmpStr.size());
    }
    return dupCStr(sv.data(), sv.size());
}

// Memory Utils

template<MemcpyableT T>
void memcpyX(const T *src, T *dst, const std::size_t count) {
    std::memcpy(dst, src, count * sizeof(T));
}

// malloc for character types
template<CharacterT T>
T *mallocX(const size_t count = 1) {
    const auto len = count * sizeof(T) + 1;
    auto ptr = std::malloc(len);
    if (!ptr) [[unlikely]] {
        return nullptr;
    }
    std::memset(ptr, 0, len);
    return static_cast<T *>(ptr);
}

// malloc for non-character types
template<typename T>
    requires (!CharacterT<T>)
T *mallocX(const size_t count = 1) {
    size_t allocCount = count;
    if constexpr (std::is_pointer_v<T>) {
        allocCount = count + 1;
    }
    auto ptr = std::calloc(allocCount, sizeof(T));
    if (!ptr) [[unlikely]] {
        return nullptr;
    }
    return static_cast<T *>(ptr);
}

// free for non-const & non-void types
template<typename T>
    requires (!ConstT<T> && !VoidT<T>)
void freeX(T * &ptr) {
    if (!ptr) [[unlikely]] {
        return;
    }
    std::free(static_cast<RawPtr>(ptr));
    ptr = nullptr;
}

// free for non-const & void types
template<typename T>
    requires (!ConstT<T> && VoidT<T>)
void freeX(T * &ptr) {
    if (!ptr) [[unlikely]] {
        return;
    }
    std::free(ptr);
    ptr = nullptr;
}

// free for const & non-void types
template<typename T>
    requires (ConstT<T> && !VoidT<T>)
void freeX(T * &ptr) {
    if (!ptr) [[unlikely]] {
        return;
    }
    std::free(const_cast<void *>(static_cast<const void *>(ptr)));
    ptr = nullptr;
}

// free for const & void types
template<typename T>
    requires (ConstT<T> && VoidT<T>)
void freeX(T * &ptr) {
    if (!ptr) [[unlikely]] {
        return;
    }
    std::free(const_cast<void *>(ptr));
    ptr = nullptr;
}

#endif

#endif // DYNXX_INCLUDE_MEMORY_HXX_
