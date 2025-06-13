#ifndef NGENXX_SRC_CORE_CONCURRENT_CONCURRENTUTIL_HXX_
#define NGENXX_SRC_CORE_CONCURRENT_CONCURRENTUTIL_HXX_

#if defined(__cplusplus)

#include <new>
#include <mutex>
#include <thread>
#include <string>
#include <chrono>
#include <functional>

namespace NGenXX::Core::Concurrent
{

using namespace std::chrono;

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
    std::this_thread::sleep_for(microseconds(microSecs));
}

template<typename T>
concept TimedLockableT = requires(T mtx)
{
    {mtx.try_lock_until(std::declval<time_point<steady_clock>>()) } -> std::convertible_to<bool>;
};

template<TimedLockableT T>
[[nodiscard]] bool tryLock(T& mtx, const size_t microSecs)
{
    const auto timeout = steady_clock::now() + microseconds(microSecs);
    return mtx.try_lock_until(timeout);
}

template <typename T>
concept RunnableT = requires(T t) 
{
    { t() } -> std::same_as<void>;
};

template <RunnableT T>
inline void callOnce(T&& func) 
{
    static std::once_flag flag;
    std::call_once(flag, std::forward<T>(func));
}

}

#endif

#endif // NGENXX_SRC_CORE_CONCURRENT_CONCURRENTUTIL_HXX_
