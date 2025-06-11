#ifndef NGENXX_SRC_CORE_CONCURRENT_CONCURRENTUTIL_HXX_
#define NGENXX_SRC_CORE_CONCURRENT_CONCURRENTUTIL_HXX_

#if defined(__cplusplus)

#include <atomic>
#include <new>
#include <thread>
#include <string>

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

template<typename F>
void callOnce(F&& func) 
{
    static std::once_flag flag;
    std::call_once(flag, std::forward<F>(func));
}

}

#endif

#endif // NGENXX_SRC_CORE_CONCURRENT_CONCURRENTUTIL_HXX_
