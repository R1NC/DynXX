#pragma once

#if defined(_WIN32)
#include <malloc.h>
#endif

#include <memory>
#if defined(__cpp_lib_ranges)
#include <ranges>
#endif

#include "Types.hxx"
#include "Macro.hxx"

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
void memcpyX(const T *src, T *dst, size_t count) {
    std::memcpy(dst, src, count * sizeof(T));
}

// malloc for character types
template<CharacterT T>
T *mallocX(size_t count = 1) {
    const auto len = count * sizeof(T) + 1;
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc)
    auto ptr = std::malloc(len);
    if (ptr == nullptr) [[unlikely]] {
        return nullptr;
    }
    std::memset(ptr, 0, len);
    return static_cast<T *>(ptr);
}

// malloc for non-character types
template<typename T>
    requires (!CharacterT<T>)
T *mallocX(size_t count = 1) {
    auto allocCount = count;
    if constexpr (std::is_pointer_v<T>) {
        allocCount = count + 1;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc)
    auto ptr = std::calloc(allocCount, sizeof(T));
    if (ptr == nullptr) [[unlikely]] {
        return nullptr;
    }
    return static_cast<T *>(ptr);
}

// free for non-const & non-void types
template<typename T>
    requires (!ConstT<T> && !VoidT<T>)
void freeX(T * &ptr) {
    if (ptr == nullptr) [[unlikely]] {
        return;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
    std::free(static_cast<RawPtr>(ptr));
    ptr = nullptr;
}

// free for non-const & void types
template<typename T>
    requires (!ConstT<T> && VoidT<T>)
void freeX(T * &ptr) {
    if (ptr == nullptr) [[unlikely]] {
        return;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc)
    std::free(ptr);
    ptr = nullptr;
}

// free for const & non-void types
template<typename T>
    requires (ConstT<T> && !VoidT<T>)
void freeX(T * &ptr) {
    if (ptr == nullptr) [[unlikely]] {
        return;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc,cppcoreguidelines-pro-type-const-cast)
    std::free(const_cast<void *>(static_cast<const void *>(ptr)));
    ptr = nullptr;
}

// free for const & void types
template<typename T>
    requires (ConstT<T> && VoidT<T>)
void freeX(T * &ptr) {
    if (ptr == nullptr) [[unlikely]] {
        return;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory,cppcoreguidelines-no-malloc)
    std::free(const_cast<void *>(ptr));
    ptr = nullptr;
}

template<InRangeT Src, OutRangeT Dst>
requires std::indirectly_writable<
    std::ranges::iterator_t<Dst>,
    std::ranges::range_reference_t<Src>
>
void copyRange(const Src& src, Dst& dst, size_t len) {
    if (len == 0) [[unlikely]] {
        return;
    }
    len = std::min({len, src.size()});
    dst.resize(len);
#if defined(__cpp_lib_ranges)
    std::ranges::copy(src | std::views::take(len), dst.begin());
#else
    std::copy_n(src.begin(), len, dst.begin());
#endif
}

// Smart Pointers

struct FreeDeleter {
    void operator()(RawPtr p) const noexcept {
        freeX(p);
    }
};
using AutoFreePtr = std::unique_ptr<void, FreeDeleter>;

// Classes

class DynXXStaticOnly {
    public:
        DynXXStaticOnly() = delete;
        DynXXStaticOnly(const DynXXStaticOnly& other) = delete;
        DynXXStaticOnly& operator=(const DynXXStaticOnly& other) = delete;
        DynXXStaticOnly(DynXXStaticOnly&& other) = delete;
        DynXXStaticOnly& operator=(DynXXStaticOnly&& other) = delete;
        void* operator new(std::size_t) = delete;
        void* operator new[](std::size_t) = delete;
        void operator delete(void*) noexcept = delete;
        void operator delete[](void*) noexcept = delete;
        ~DynXXStaticOnly() = delete;
};

class DynXXStackOnly {
public:
    DynXXStackOnly() = default;
    DynXXStackOnly(const DynXXStackOnly& other) = default;
    DynXXStackOnly& operator=(const DynXXStackOnly& other) = default;
    DynXXStackOnly(DynXXStackOnly&& other) noexcept = default;
    DynXXStackOnly& operator=(DynXXStackOnly&& other) noexcept = default;
    virtual ~DynXXStackOnly() = delete;
    void* operator new(size_t) = delete;
    void* operator new[](size_t) = delete;
    void operator delete(void*) = delete;
    void operator delete[](void*) = delete;
};

class DynXXHeapOnly {
public:
    DynXXHeapOnly(const DynXXHeapOnly& other) = delete;
    DynXXHeapOnly& operator=(const DynXXHeapOnly& other) = delete;
    DynXXHeapOnly(DynXXHeapOnly&& other) = delete;
    DynXXHeapOnly& operator=(DynXXHeapOnly&& other) = delete;
    virtual ~DynXXHeapOnly() = default;

protected:
    explicit DynXXHeapOnly() = default;
};

template <typename Derived>
class DynXXSmartHeapOnly : public DynXXHeapOnly {
public:
    DynXXSmartHeapOnly(const DynXXSmartHeapOnly&) = delete;
    DynXXSmartHeapOnly& operator=(const DynXXSmartHeapOnly&) = delete;
    DynXXSmartHeapOnly(DynXXSmartHeapOnly&&) = delete;
    DynXXSmartHeapOnly& operator=(DynXXSmartHeapOnly&&) = delete;

    template<typename... Args>
    static std::shared_ptr<Derived> makeUnique(Args&&... args) {
        return std::make_unique<Derived>(Token{}, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static std::shared_ptr<Derived> makeShared(Args&&... args) {
        return std::make_shared<Derived>(Token{}, std::forward<Args>(args)...);
    }

protected:
    struct Token {
        explicit Token() = default;
    };
    explicit DynXXSmartHeapOnly(Token token) : DynXXHeapOnly(), token(token) {}
private:
    NO_UNIQUE_ADDRESS Token token;
};
