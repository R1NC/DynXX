#ifndef NGENXX_SRC_UTIL_CONCURRENT_HXX_
#define NGENXX_SRC_UTIL_CONCURRENT_HXX_

#if defined(__cplusplus)

#include <atomic>

class SpinLock {
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