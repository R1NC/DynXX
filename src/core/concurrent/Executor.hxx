#ifndef NGENXX_SRC_CORE_CONCURRENT_EXECUTOR_HXX_
#define NGENXX_SRC_CORE_CONCURRENT_EXECUTOR_HXX_

#if defined(__cplusplus)

#include <functional>
#include <queue>
#include <vector>

#include "ConcurrentUtil.hxx"

namespace NGenXX::Core::Concurrent
{

class alignas(CacheLineSize) Executor final
{
public:
    Executor();
    explicit Executor(size_t workerPoolCapacity, size_t sleepMicroSecs);
    Executor(const Executor &) = delete;
    Executor &operator=(const Executor &) = delete;
    Executor(Executor &&) = delete;
    Executor &operator=(Executor &&) = delete;
    ~Executor();

    using TaskT = std::function<void()>;

    Executor& operator>>(TaskT&& task);

private:
    size_t workerPoolCapacity{0uz};
    size_t sleepMicroSecs{1000uz};
    std::recursive_timed_mutex mutex;
    std::queue<TaskT> taskQueue;
#if defined(__cpp_lib_jthread)
    std::vector<std::jthread> workerPool;
#else
    std::vector<std::thread> workerPool;
    std::atomic<bool> active{false};
#endif

    [[nodiscard]] bool tryLock();
    void unlock();
    void sleep();
};

}

#endif

#endif // NGENXX_SRC_CORE_CONCURRENT_EXECUTOR_HXX_
