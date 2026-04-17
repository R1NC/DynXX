#include "Executor.hxx"

#include <bit>

#include <DynXX/CXX/Log.hxx>

#include "ConcurrentUtil.hxx"

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
        dynxxLogPrintF(Debug, "Worker@{} run task on thread:{}", std::bit_cast<uintptr_t>(this), currentThreadId());
        try
        {
            func();
        }
        catch (const std::bad_function_call &e)
        {
            dynxxLogPrintF(Error, "Worker bad_function_call: {}", e.what());
        }
        catch (const std::exception &e)
        {
            dynxxLogPrintF(Error, "Worker task exception: {}", e.what());
        }
        catch (...)
        {
            dynxxLogPrint(Error, "Worker task exception: unknown");
        }
    }
}, [this]() {
    const auto lock = std::scoped_lock(this->mutex);
    return !this->taskQueue.empty();
})
{
}

Worker::~Worker()
{
    this->stopAndJoin();
}

Worker& Worker::operator>>(TaskT&& task)
{
    this->update([&mtx = this->mutex, tsk = std::move(task), &queue = this->taskQueue, addr = std::bit_cast<uintptr_t>(this)]() mutable {
        const auto lock = std::scoped_lock(mtx);
        queue.emplace(std::move(tsk));
        dynxxLogPrintF(Debug, "Worker@{} taskCount:{}", addr, queue.size());
    });
    
    return *this;
}

// - Executor

Executor::Executor() : Executor(0UZ)
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
    if (!task) [[unlikely]]
    {
        dynxxLogPrint(Error, "Executor ignored empty task");
        return *this;
    }

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
