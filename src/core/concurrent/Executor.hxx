#ifndef NGENXX_SRC_CORE_CONCURRENT_EXECUTOR_HXX_
#define NGENXX_SRC_CORE_CONCURRENT_EXECUTOR_HXX_

#if defined(__cplusplus)

#include <atomic>
#include <functional>
#include <queue>
#include <vector>

#include "ConcurrentUtil.hxx"

namespace NGenXX::Core::Concurrent {
    using TaskT = std::function<void()>;

    class
#if !defined(__cpp_lib_jthread)
            alignas(CacheLineSize)
#endif
            Worker final {
    public:
        Worker();

        explicit Worker(size_t sleepMicroSecs);

        Worker(const Worker &) = delete;

        Worker &operator=(const Worker &) = delete;

        Worker(Worker &&) = delete;

        Worker &operator=(Worker &&) = delete;

        ~Worker();

        Worker &operator>>(TaskT &&task);

    private:
        size_t sleepMicroSecs{1000uz};
        std::timed_mutex mutex;
        std::queue<TaskT> taskQueue;
#if defined(__cpp_lib_jthread)
        std::jthread thread;
#else
        std::thread thread;
        std::atomic<bool> active{false};
#endif

        [[nodiscard]] bool tryLock();

        void unlock();

        void sleep();
    };

    class Executor final {
    public:
        Executor();

        explicit Executor(size_t workerPoolCapacity, size_t sleepMicroSecs);

        Executor(const Executor &) = delete;

        Executor &operator=(const Executor &) = delete;

        Executor(Executor &&) = delete;

        Executor &operator=(Executor &&) = delete;

        ~Executor();

        Executor &operator>>(TaskT &&task);

    private:
        size_t workerPoolCapacity{0uz};
        size_t sleepMicroSecs{1000uz};
        std::vector<std::unique_ptr<Worker>> workerPool;
        std::mutex mutex;
        unsigned workerIndex{0uz};
    };
}

#endif

#endif // NGENXX_SRC_CORE_CONCURRENT_EXECUTOR_HXX_
