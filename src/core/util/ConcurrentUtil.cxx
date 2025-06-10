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

NGenXX::Core::Util::Concurrent::Executor::Executor() : Executor(1uz)
{
}

NGenXX::Core::Util::Concurrent::Executor::Executor(size_t sleepMilliSecs) : sleepMilliSecs(sleepMilliSecs)
{
#if !defined(__cpp_lib_jthread)
    this->active = true;
#endif
}

NGenXX::Core::Util::Concurrent::Executor::~Executor()
{
    std::lock_guard lock(this->mutex);

#if !defined(__cpp_lib_jthread)
    this->active = false;
    for (auto& thread : this->pool)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
#endif
    
    pool.clear();
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

NGenXX::Core::Util::Concurrent::Executor& NGenXX::Core::Util::Concurrent::Executor::operator>>(TaskT&& task)
{
    std::lock_guard lock(this->mutex);

    this->queue.emplace(std::move(task));

    const auto cpuCores = Util::Concurrent::countCPUCore();
    if (this->pool.size() >= cpuCores)
    {
        return *this;
    }

    this->pool.emplace_back([this]
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

	return *this;
}