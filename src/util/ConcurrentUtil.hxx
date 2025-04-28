#ifndef NGENXX_SRC_UTIL_CONCURRENT_HXX_
#define NGENXX_SRC_UTIL_CONCURRENT_HXX_

#if defined(__cplusplus)

#include <atomic>
#include <new>

#if defined(__cpp_lib_hardware_interference_size)
    static constexpr size_t CacheLineSize = std::hardware_destructive_interference_size;
#else
    static constexpr size_t CacheLineSize = 64;
#endif

class alignas(CacheLineSize) SpinLock {
private:
    std::atomic<bool> lockFlag = {false};

public:
    SpinLock() = default;
    SpinLock(const SpinLock &) = delete;
    SpinLock &operator=(const SpinLock &) = delete;
    SpinLock(SpinLock &&) = delete;
    SpinLock &operator=(SpinLock &&) = delete;
    ~SpinLock() = default;

    void lock() 
    {
        while (lockFlag.exchange(true, std::memory_order_acquire));
    }

    void unlock() 
    {
        lockFlag.store(false, std::memory_order_release);
    }
};

#endif

#endif // NGENXX_SRC_UTIL_CONCURRENT_HXX_