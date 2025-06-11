#include "Executor.hxx"

#include "../util/TimeUtil.hxx"
#include <NGenXXLog.hxx>

NGenXX::Core::Concurrent::Executor::Executor() : Executor(0uz, 1uz)
{
}

NGenXX::Core::Concurrent::Executor::Executor(size_t workerPoolCapacity, size_t sleepMilliSecs) : workerPoolCapacity{workerPoolCapacity}, sleepMilliSecs{sleepMilliSecs}
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
    const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(this->sleepMilliSecs);
    return this->mutex.try_lock_until(timeout);
}

void NGenXX::Core::Concurrent::Executor::unlock()
{
    this->mutex.unlock();
}

void NGenXX::Core::Concurrent::Executor::sleep()
{
    Util::Time::sleepForMilliSecs(this->sleepMilliSecs);
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
            if (!tryLock())
            {
                sleep();
                continue;
            }

            std::function<void()> func;
            {
                if (taskQueue.empty())
                {
                    unlock();
                    sleep();
                    continue;
                }

                func = std::move(taskQueue.front());
                taskQueue.pop();
                unlock();
            }

            if (func)
            {
                ngenxxLogPrintF(NGenXXLogLevelX::Debug, "Executor task running on thread:{}", currentThreadId());
                func();
            }
            else
            {
                sleep();
            }
        }
    });

    ngenxxLogPrintF(NGenXXLogLevelX::Debug, "Executor created new worker, poolSize:{} poolCapacity:{} cpuCores:{}", this->workerPool.size(), this->workerPoolCapacity, countCPUCore());

	return *this;
}