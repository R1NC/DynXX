#include "ConcurrentUtil.hxx"

#include "TimeUtil.hxx"
#include <NGenXXLog.hxx>

#pragma mark - SpinLock

NGenXX::Core::Util::Concurrent::SpinLock::SpinLock()
{
}

NGenXX::Core::Util::Concurrent::SpinLock::~SpinLock()
{
}

void NGenXX::Core::Util::Concurrent::SpinLock::lock() 
{
    while (lockFlag.exchange(true, std::memory_order_acquire));
}

void NGenXX::Core::Util::Concurrent::SpinLock::unlock() 
{
    lockFlag.store(false, std::memory_order_release);
}

#pragma mark - Executor

NGenXX::Core::Util::Concurrent::Executor::Executor() : sleepMilliSecs(1uz)
{
}

NGenXX::Core::Util::Concurrent::Executor::Executor(size_t sleepMilliSecs) : sleepMilliSecs(sleepMilliSecs)
{
    this->active = true;
}

NGenXX::Core::Util::Concurrent::Executor::~Executor()
{
    this->active = false;
    std::lock_guard lock(this->mutex);
    for (auto& thread : this->pool)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

bool NGenXX::Core::Util::Concurrent::Executor::tryLock()
{
    const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(this->sleepMilliSecs);
    return this->mutex.try_lock_until(timeout);
}

void NGenXX::Core::Util::Concurrent::Executor::unlock()
{
    this->mutex.unlock();
}

void NGenXX::Core::Util::Concurrent::Executor::sleep()
{
    Util::Time::sleepForMilliSecs(this->sleepMilliSecs);
}

void NGenXX::Core::Util::Concurrent::Executor::add(const std::function<void()> &&task)
{
    std::lock_guard lock(this->mutex);

    this->queue.emplace(std::move(task));

    const auto cpuCores = Util::Concurrent::countCPUCore();
    if (this->pool.size() >= cpuCores)
    {
        return;
    }

    this->pool.emplace_back([this] {
        while (active)
        {
            if (!tryLock())
            {
                sleep();
                continue;
            }

            std::function<void()> func;
            {
                if (queue.empty())
                {
                    unlock();
                    sleep();
                    continue;
                }

                func = std::move(queue.front());
                queue.pop();
                unlock();
            }

            if (func)
            {
                ngenxxLogPrintF(NGenXXLogLevelX::Debug, "Executor task running on thread:{}", NGenXX::Core::Util::Concurrent::currentThreadId());
                func();
            }
            else
            {
                sleep();
            }
        }
    });

    ngenxxLogPrintF(NGenXXLogLevelX::Debug, "Executor created new thread, poolSize:{} cpuCoreCount:{}", this->pool.size(), cpuCores);
}