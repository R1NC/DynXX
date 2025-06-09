#ifndef NGENXX_SRC_UTIL_CONCURRENT_HXX_
#define NGENXX_SRC_UTIL_CONCURRENT_HXX_

#if defined(__cplusplus)

#include <atomic>
#include <new>
#include <thread>

namespace NGenXX::Util::Concurrent
{

static constexpr size_t CacheLineSize = 
#if defined(__cpp_lib_hardware_interference_size)
    std::hardware_destructive_interference_size
#else
    64
#endif
;

class alignas(CacheLineSize) SpinLock 
{
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

private:
    std::atomic<bool> lockFlag = {false};
};

inline unsigned int countCPUCore() {
    return std::thread::hardware_concurrency();
}

inline std::string currentThreadId()
{
    static thread_local std::string cachedCurrentThreadId;
    if (cachedCurrentThreadId.empty()) 
    {
        const auto& id = std::this_thread::get_id();
        cachedCurrentThreadId = std::to_string(std::hash<std::thread::id>{}(id));
    }
    return cachedCurrentThreadId;
}

}

#endif

#endif // NGENXX_SRC_UTIL_CONCURRENT_HXX_
