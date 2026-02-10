#pragma once

#include <bit>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include <DynXX/CXX/Types.hxx>

namespace DynXX::Core::Util::Mem {

// Pointer cast

template<typename T = void>
T *addr2ptr(const address addr) {
    if (addr == 0) [[unlikely]] {
        return nullptr;
    }
    return std::bit_cast<T *>(addr);
}

template<typename T = void>
address ptr2addr(const T *ptr) {
    if (ptr == nullptr) [[unlikely]] {
        return 0;
    }
    return std::bit_cast<address>(ptr);
}

// Pointer cache

template<typename T>
class PtrCache final {
    public:
        PtrCache() = default;
        PtrCache(const PtrCache &) = delete;
        PtrCache &operator=(const PtrCache &) = delete;
        PtrCache(PtrCache &&) = delete;
        PtrCache &operator=(PtrCache &&) = delete;
        ~PtrCache() = default;

        address add(std::unique_ptr<T> ptr) {
            const auto lock = std::scoped_lock(mutex);
            const auto addr = ptr2addr(ptr.get());
            this->cache.emplace(addr, std::move(ptr));
            return addr;
        }

        void remove(address handle) {
            const auto lock = std::scoped_lock(mutex);
            this->cache.erase(handle);
        }

        bool contains(address handle) const {
            const auto lock = std::shared_lock(mutex);
            return this->cache.contains(handle);
        }

        T* get(address handle) const {
            const auto lock = std::shared_lock(mutex);
            if (auto it = this->cache.find(handle); it != this->cache.end()) {
                return it->second.get();
            }
            return nullptr;
        }

    private:
        std::unordered_map<address, std::unique_ptr<T>> cache;
        mutable std::shared_mutex mutex;
};

}  // namespace DynXX::Core::Util::Mem
