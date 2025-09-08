#ifndef DYNXX_SRC_CORE_CONCURRENT_EXECUTOR_HXX_
#define DYNXX_SRC_CORE_CONCURRENT_EXECUTOR_HXX_

#if defined(__cplusplus)

#include <queue>
#include <vector>

#include "ConcurrentUtil.hxx"
#include "Daemon.hxx"

namespace DynXX::Core::Concurrent {

    class Worker final : public Daemon {
    public:
        Worker();

        Worker(const Worker &) = delete;

        Worker &operator=(const Worker &) = delete;

        Worker(Worker &&) = delete;

        Worker &operator=(Worker &&) = delete;

        ~Worker() override = default;

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
