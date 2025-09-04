#ifndef DYNXX_SRC_CORE_CONCURRENT_EXECUTOR_HXX_
#define DYNXX_SRC_CORE_CONCURRENT_EXECUTOR_HXX_

#if defined(__cplusplus)

#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>
#include <vector>

#include "ConcurrentUtil.hxx"

namespace DynXX::Core::Concurrent {
    using TaskT = std::
#if defined(__cpp_lib_move_only_function)
    move_only_function
#else
    function
#endif
    <void()>;

    using RunChecker = std::function<bool()>;

    class
#if !defined(__cpp_lib_jthread)
        alignas(CacheLineSize)
#endif
        Daemon {
    public:
        Daemon() = delete;

        explicit Daemon(TaskT &&runLoop, RunChecker &&runChecker = []() { return true; });

        Daemon(const Daemon &) = delete;

        Daemon &operator=(const Daemon &) = delete;

        Daemon(Daemon &&) = delete;

        Daemon &operator=(Daemon &&) = delete;

        virtual ~Daemon();

        void update(TaskT &&sth);

    private:
        mutable std::mutex mutex;
        std::condition_variable cv;

        TaskT runLoop;
        RunChecker runChecker;

#if defined(__cpp_lib_jthread)
        std::jthread thread;
#else
        std::thread thread;
        std::atomic<bool> shouldStop{false};
#endif
    };

    class Worker final : public Daemon {
    public:
        Worker();

        Worker(const Worker &) = delete;

        Worker &operator=(const Worker &) = delete;

        Worker(Worker &&) = delete;

        Worker &operator=(Worker &&) = delete;

        ~Worker() = default;

        Worker &operator>>(TaskT &&task);

    private:
        std::queue<TaskT> taskQueue;
        mutable std::mutex mutex;
    };

    class Executor final {
    public:
        Executor();

        explicit Executor(size_t workerPoolCapacity);

        Executor(const Executor &) = delete;

        Executor &operator=(const Executor &) = delete;

        Executor(Executor &&) = delete;

        Executor &operator=(Executor &&) = delete;

        ~Executor();

        Executor &operator>>(TaskT &&task);

    private:
        size_t workerPoolCapacity{0uz};
        std::vector<std::unique_ptr<Worker>> workerPool;
        mutable std::mutex mutex;
        size_t workerIndex{0uz};
    };
}

#endif

#endif // DYNXX_SRC_CORE_CONCURRENT_EXECUTOR_HXX_
