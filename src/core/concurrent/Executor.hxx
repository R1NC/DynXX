#pragma once

#include <queue>
#include <vector>

#include "ConcurrentUtil.hxx"
#include "Daemon.hxx"

namespace DynXX::Core::Concurrent {

    class Worker final : public Daemon {
    public:
        Worker();

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

        ~Executor() = default;

        Executor &operator>>(TaskT &&task);

        Executor(const Executor &) = delete;
        Executor &operator=(const Executor &) = delete;
        Executor(Executor &&) = delete;
        Executor &operator=(Executor &&) = delete;

    private:
        size_t workerPoolCapacity{0uz};
        std::vector<std::unique_ptr<Worker>> workerPool;
        mutable std::mutex mutex;
        size_t workerIndex{0uz};
    };
}
