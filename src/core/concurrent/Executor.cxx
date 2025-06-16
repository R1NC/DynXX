#include "Executor.hxx"

#include <NGenXXLog.hxx>

#pragma mark - Wroker

NGenXX::Core::Concurrent::Worker::Worker() : Worker(1000uz)
{
}

NGenXX::Core::Concurrent::Worker::Worker(size_t sleepMicroSecs) : sleepMicroSecs{sleepMicroSecs}
{
#if defined(__cpp_lib_jthread)
    this->thread = std::jthread([this](std::stop_token stoken) {
        while (!stoken.stop_requested())
#else
	this->active = true;
    this->thread = std::thread([this]() {
        while (active)
#endif
        {
            if (!tryLock()) [[unlikely]]
            {
                sleep();
                continue;
            }

            std::function<void()> func;
            {
                if (taskQueue.empty()) [[unlikely]]
                {
                    unlock();
                    sleep();
                    continue;
                }

                func = std::move(taskQueue.front());
                taskQueue.pop();
                unlock();
            }

            if (func) [[likely]]
            {
                ngenxxLogPrintF(NGenXXLogLevelX::Debug, "Worker@{} run task on thread:{}", reinterpret_cast<uintptr_t>(this), currentThreadId());
                func();
            }
            else [[unlikely]]
            {
                sleep();
            }
        }
    });
}

NGenXX::Core::Concurrent::Worker::~Worker()
{
    std::lock_guard lock(this->mutex);

#if !defined(__cpp_lib_jthread)
    this->active = false;
    if (thread.joinable())
    {
        thread.join();
    }
#endif
}

bool NGenXX::Core::Concurrent::Worker::tryLock()
{
	return NGenXX::Core::Concurrent::tryLock(this->mutex, sleepMicroSecs);
}

void NGenXX::Core::Concurrent::Worker::unlock()
{
    this->mutex.unlock();
}

void NGenXX::Core::Concurrent::Worker::sleep()
{
    NGenXX::Core::Concurrent::sleep(this->sleepMicroSecs);
}

NGenXX::Core::Concurrent::Worker& NGenXX::Core::Concurrent::Worker::operator>>(TaskT&& task)
{
    std::lock_guard lock(this->mutex);
    this->taskQueue.emplace(std::move(task));
	ngenxxLogPrintF(NGenXXLogLevelX::Debug, "Worker@{} taskCount:{}",  reinterpret_cast<uintptr_t>(this), this->taskQueue.size());

	return *this;
}

#pragma mark - Executor

NGenXX::Core::Concurrent::Executor::Executor() : Executor(0uz, 1000uz)
{
}

NGenXX::Core::Concurrent::Executor::Executor(size_t workerPoolCapacity, size_t sleepMicroSecs) : workerPoolCapacity{workerPoolCapacity}, sleepMicroSecs{sleepMicroSecs}
{
    if (this->workerPoolCapacity == 0)
    {
        this->workerPoolCapacity = countCPUCore();
    }
}

NGenXX::Core::Concurrent::Executor::~Executor()
{
    this->workerPool.clear();
}

NGenXX::Core::Concurrent::Executor& NGenXX::Core::Concurrent::Executor::operator>>(TaskT&& task)
{
    std::lock_guard lock(this->mutex);

    if (this->workerPool.size() < this->workerPoolCapacity)
    {
        auto worker = std::unique_ptr<Worker>(new Worker(this->sleepMicroSecs));
        this->workerPool.push_back(std::move(worker));
        ngenxxLogPrintF(NGenXXLogLevelX::Debug, "Executor created new worker, poolSize:{} poolCapacity:{} cpuCores:{}", 
                        this->workerPool.size(), this->workerPoolCapacity, countCPUCore());
    }
    
    *(this->workerPool[this->workerIndex]) >> std::move(task);
    
    this->workerIndex = (this->workerIndex + 1) % this->workerPoolCapacity;
    
    return *this;
}