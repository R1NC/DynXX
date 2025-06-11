#ifndef NGENXX_SRC_CORE_CONCURRENT_CONCURRENTUTIL_HXX_
#define NGENXX_SRC_CORE_CONCURRENT_CONCURRENTUTIL_HXX_

#if defined(__cplusplus)

#include <new>
#include <atomic>
#include <mutex>
#include <thread>
#include <string>
#include <chrono>

namespace NGenXX::Core::Concurrent
{

static constexpr size_t CacheLineSize = 
#if defined(__cpp_lib_hardware_interference_size)
    std::hardware_destructive_interference_size
#else
    64
#endif
;


inline unsigned int countCPUCore() {
    return std::thread::hardware_concurrency();
}

inline std::string currentThreadId()
{
    thread_local std::string cachedCurrentThreadId;
    if (cachedCurrentThreadId.empty()) 
    {
        const auto& id = std::this_thread::get_id();
        cachedCurrentThreadId = std::to_string(std::hash<std::thread::id>{}(id));
    }
    return cachedCurrentThreadId;
}

inline void sleep(const size_t microSecs)
{
    std::this_thread::sleep_for(std::chrono::microseconds(microSecs));
}

template<typename T>
concept TimedLockableT = requires(T mtx)
{
    {mtx.try_lock_until(std::declval<std::chrono::time_point<std::chrono::steady_clock>>()) } -> std::convertible_to<bool>;
};

template<TimedLockableT T>
bool tryLockMutex(T& mtx, const size_t microSecs)
{
    const auto timeout = std::chrono::steady_clock::now() + std::chrono::microseconds(microSecs);
    return mtx.try_lock_until(timeout);
}

template<typename F>
void callOnce(F&& func) 
{
    static std::once_flag flag;
    std::call_once(flag, std::forward<F>(func));
}

}

#endif

#endif // NGENXX_SRC_CORE_CONCURRENT_CONCURRENTUTIL_HXX_
