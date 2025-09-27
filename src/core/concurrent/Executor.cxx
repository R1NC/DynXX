#include "Executor.hxx"

#include <DynXX/CXX/Log.hxx>

namespace {
    using enum DynXXLogLevelX;
}

// - Worker

namespace DynXX::Core::Concurrent {

Worker::Worker() : 
 Daemon([this]() {
    TaskT func;
    {
        const auto lock = std::scoped_lock(this->mutex);
        if (taskQueue.empty()) [[unlikely]] 
        {
            return;
        }
        func = std::move(taskQueue.front());
        taskQueue.pop();
    }

    if (func) [[likely]] 
    {
        dynxxLogPrintF(Debug, "Worker@{} run task on thread:{}", reinterpret_cast<uintptr_t>(this), currentThreadId());
        func();
    }
}, [this]() {
    const auto lock = std::scoped_lock(this->mutex);
    return !this->taskQueue.empty();
})
{
}

Worker& Worker::operator>>(TaskT&& task)
{
    this->update([&mtx = this->mutex, tsk = std::move(task), &queue = this->taskQueue, addr = reinterpret_cast<uintptr_t>(this)]() mutable {
        const auto lock = std::scoped_lock(mtx);
        queue.emplace(std::move(tsk));
        dynxxLogPrintF(Debug, "Worker@{} taskCount:{}", addr, queue.size());
    });
    
    return *this;
}

// - Executor

Executor::Executor() : Executor(0uz)
{
}

Executor::Executor(size_t workerPoolCapacity) : workerPoolCapacity{workerPoolCapacity}
{
    if (this->workerPoolCapacity == 0)
    {
        this->workerPoolCapacity = countCPUCore();
    }
    this->workerPool.reserve(this->workerPoolCapacity);
}

Executor& Executor::operator>>(TaskT&& task)
{
    const auto lock = std::scoped_lock(this->mutex);

    if (this->workerPool.size() < this->workerPoolCapacity)
    {
        auto worker = std::make_unique<Worker>();
        this->workerPool.emplace_back(std::move(worker));
        dynxxLogPrintF(Debug, "Executor created new worker, poolSize:{} poolCapacity:{} cpuCores:{}", 
                        this->workerPool.size(), this->workerPoolCapacity, countCPUCore());
    }
    
    *(this->workerPool.at(this->workerIndex)) >> std::move(task);
    
    this->workerIndex = (this->workerIndex + 1) % this->workerPoolCapacity;
    
    return *this;
}

} // namespace DynXX::Core::Concurrent