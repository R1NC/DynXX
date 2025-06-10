#ifndef NGENXX_SRC_CORE_UTIL_CONCURRENTUTIL_HXX_
#define NGENXX_SRC_CORE_UTIL_CONCURRENTUTIL_HXX_

#if defined(__cplusplus)

#include <atomic>
#include <new>
#include <thread>
#include <mutex>
#include <functional>
#include <queue>
#include <vector>
#include <string>

namespace NGenXX::Core::Util::Concurrent
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
    static thread_local std::string cachedCurrentThreadId;
    if (cachedCurrentThreadId.empty()) 
    {
        const auto& id = std::this_thread::get_id();
        cachedCurrentThreadId = std::to_string(std::hash<std::thread::id>{}(id));
    }
    return cachedCurrentThreadId;
}


class alignas(CacheLineSize) SpinLock final
{
public:
    SpinLock();
    SpinLock(const SpinLock &) = delete;
    SpinLock &operator=(const SpinLock &) = delete;
    SpinLock(SpinLock &&) = delete;
    SpinLock &operator=(SpinLock &&) = delete;
    ~SpinLock();

    void lock();
    void unlock();

private:
    std::atomic<bool> lockFlag = {false};
};


class alignas(CacheLineSize) Executor final
{
public:
    Executor();
    explicit Executor(size_t sleepMilliSecs);
    Executor(const Executor &) = delete;
    Executor &operator=(const Executor &) = delete;
    Executor(Executor &&) = delete;
    Executor &operator=(Executor &&) = delete;
    ~Executor();

    using TaskT = std::function<void()>;

    void add(const TaskT &&task);

private:
    std::atomic<bool> active{false};
    size_t sleepMilliSecs{1uz};
    std::recursive_timed_mutex mutex;
    std::vector<std::thread> pool;
    std::queue<TaskT> queue;

    bool tryLock();
    void unlock();
    void sleep();
};

}

#endif

#endif // NGENXX_SRC_CORE_UTIL_CONCURRENTUTIL_HXX_
