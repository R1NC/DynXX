#include "Executor.hxx"

#include <DynXX/CXX/Log.hxx>

namespace {
    using enum DynXXLogLevelX;
}

// - Daemon

DynXX::Core::Concurrent::Daemon::Daemon(TaskT &&runLoop, RunChecker &&runChecker) : runLoop{std::move(runLoop)}, runChecker{std::move(runChecker)}
{
    #if defined(__cpp_lib_jthread)
    this->thread = std::jthread([this](std::stop_token stoken) {
        while (!stoken.stop_requested())
#else
    this->thread = std::thread([this]() {
        while (!shouldStop.load())
#endif
        {
            auto lock = std::unique_lock(this->mutex);
            // Wait for stop signal
            this->cv.wait(lock, [this
#if defined(__cpp_lib_jthread)
                , &stoken
#endif
                ]() {
                return this->runChecker() ||
#if defined(__cpp_lib_jthread)
                stoken.stop_requested()
#else
                shouldStop.load()
#endif
                ;
            });
            
            if (
#if defined(__cpp_lib_jthread)
                stoken.stop_requested()
#else
                shouldStop.load()
#endif
            ) {
                break;
            }

            this->runLoop();

            lock.unlock();
        }
    });
}

void DynXX::Core::Concurrent::Daemon::update(TaskT &&sth)
{
    {
        auto lock = std::scoped_lock(this->mutex);
        sth();
    }
    
    this->cv.notify_one();
}

DynXX::Core::Concurrent::Daemon::~Daemon()
{
#if defined(__cpp_lib_jthread)
    // jthread automatically handles stop request and join
#else
    {
        auto lock = std::scoped_lock(this->mutex);
        this->shouldStop = true;
    }
    this->notify();
    
    if (this->thread.joinable()) [[likely]] 
    {
        this->thread.join();
    }
#endif
}

// - Worker

DynXX::Core::Concurrent::Worker::Worker() : 
 Daemon([this]() {
    auto lock = std::scoped_lock(this->mutex);
    if (taskQueue.empty()) [[unlikely]] 
    {
        return;
    }

    auto func = std::move(taskQueue.front());
    taskQueue.pop();

    if (func) [[likely]] 
    {
        dynxxLogPrintF(Debug, "Worker@{} run task on thread:{}", reinterpret_cast<uintptr_t>(this), currentThreadId());
        func();
    }
}, [this]() {
    auto lock = std::scoped_lock(this->mutex);
    return !this->taskQueue.empty();
})
{
}

DynXX::Core::Concurrent::Worker::~Worker()
{
}

DynXX::Core::Concurrent::Worker& DynXX::Core::Concurrent::Worker::operator>>(TaskT&& task)
{
    this->update([&mutex = this->mutex, task = std::move(task), &queue = this->taskQueue, addr = reinterpret_cast<uintptr_t>(this)]() {
        auto lock = std::scoped_lock(mutex);
        queue.emplace(std::move(task));
        dynxxLogPrintF(Debug, "Worker@{} taskCount:{}", addr, queue.size());
    });
    
    return *this;
}

// - Executor

DynXX::Core::Concurrent::Executor::Executor() : Executor(0uz)
{
}

DynXX::Core::Concurrent::Executor::Executor(size_t workerPoolCapacity) : workerPoolCapacity{workerPoolCapacity}
{
    if (this->workerPoolCapacity == 0)
    {
        this->workerPoolCapacity = countCPUCore();
    }
    this->workerPool.reserve(this->workerPoolCapacity);
}

DynXX::Core::Concurrent::Executor::~Executor()
{
    this->workerPool.clear();
}

DynXX::Core::Concurrent::Executor& DynXX::Core::Concurrent::Executor::operator>>(TaskT&& task)
{
    auto lock = std::scoped_lock(this->mutex);

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