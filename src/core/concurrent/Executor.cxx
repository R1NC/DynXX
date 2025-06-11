#include "Executor.hxx"

#include <NGenXXLog.hxx>

NGenXX::Core::Concurrent::Executor::Executor() : Executor(0uz, 1000uz)
{
}

NGenXX::Core::Concurrent::Executor::Executor(size_t workerPoolCapacity, size_t sleepMicroSecs) : workerPoolCapacity{workerPoolCapacity}, sleepMicroSecs{sleepMicroSecs}
{
#if !defined(__cpp_lib_jthread)
    this->active = true;
#endif

    if (this->workerPoolCapacity == 0)
    {
        this->workerPoolCapacity = countCPUCore();
    }
}

NGenXX::Core::Concurrent::Executor::~Executor()
{
    std::lock_guard lock(this->mutex);

#if !defined(__cpp_lib_jthread)
    this->active = false;
    for (auto& thread : this->workerPool)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
#endif
    
    this->workerPool.clear();
}

bool NGenXX::Core::Concurrent::Executor::tryLock()
{
	return NGenXX::Core::Concurrent::tryLockMutex(this->mutex, sleepMicroSecs);
}

void NGenXX::Core::Concurrent::Executor::unlock()
{
    this->mutex.unlock();
}

void NGenXX::Core::Concurrent::Executor::sleep()
{
    NGenXX::Core::Concurrent::sleep(this->sleepMicroSecs);
}

NGenXX::Core::Concurrent::Executor& NGenXX::Core::Concurrent::Executor::operator>>(TaskT&& task)
{
    std::lock_guard lock(this->mutex);

    this->taskQueue.emplace(std::move(task));

    if (this->workerPool.size() >= this->workerPoolCapacity)
    {
        return *this;
    }

    this->workerPool.emplace_back([this]
#if defined(__cpp_lib_jthread)
        (std::stop_token stoken) {
        while (!stoken.stop_requested())
#else
        () {
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
                ngenxxLogPrintF(NGenXXLogLevelX::Debug, "Executor task running on thread:{}", currentThreadId());
                func();
            }
            else [[unlikely]]
            {
                sleep();
            }
        }
    });
    
    ngenxxLogPrintF(NGenXXLogLevelX::Debug, "Executor created new worker, poolSize:{} poolCapacity:{} cpuCores:{}", this->workerPool.size(), this->workerPoolCapacity, countCPUCore());
    return *this;
}