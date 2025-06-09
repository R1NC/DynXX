#include "BaseBridge.hxx"

#include <utility>

#include <NGenXXLog.hxx>
#include "../util/TimeUtil.hxx"
#include "../util/ConcurrentUtil.hxx"

namespace
{
    constexpr auto SleepMilliSecs = 1uz;
}

bool NGenXX::Bridge::BaseBridge::tryLockMutex(std::recursive_timed_mutex &mtx)
{
    const auto timeout = steady_clock::now() + milliseconds(SleepMilliSecs);
    return mtx.try_lock_until(timeout);
}

void NGenXX::Bridge::BaseBridge::unlockMutex(std::recursive_timed_mutex &mtx)
{
    mtx.unlock();
}

void NGenXX::Bridge::BaseBridge::sleep()
{
    sleepForMilliSecs(SleepMilliSecs);
}

void NGenXX::Bridge::BaseBridge::enqueueTask(const std::function<void()> &&taskF)
{
    std::lock_guard lock(this->threadPoolMutex);

    this->taskQueue.emplace(std::move(taskF));

    auto cpuCores = countCPUCore();
    if (this->threadPool.size() >= cpuCores)
    {
        return;
    }

    this->threadPool.emplace_back([this] {
        while (active)
        {
            if (!tryLockMutex(threadPoolMutex))
            {
                sleep();
                continue;
            }

            std::function<void()> func;
            {
                if (taskQueue.empty())
                {
                    unlockMutex(threadPoolMutex);
                    sleep();
                    continue;
                }

                func = std::move(taskQueue.front());
                taskQueue.pop();
                unlockMutex(threadPoolMutex);
            }

            if (func)
            {
                ngenxxLogPrintF(NGenXXLogLevelX::Debug, "Bridge task running on thread:{}", currentThreadId());
                func();
            }
            else
            {
                sleep();
            }
        }
    });

    ngenxxLogPrintF(NGenXXLogLevelX::Debug, "Bridge created new thread, poolSize:{} cpuCoreCount:{}", this->threadPool.size(), cpuCores);
}

NGenXX::Bridge::BaseBridge::BaseBridge()
{
    this->active = true;
}

NGenXX::Bridge::BaseBridge::~BaseBridge()
{
    this->active = false;

    std::lock_guard lock(this->threadPoolMutex);
    for (auto& thread : this->threadPool) 
    {
        if (thread.joinable()) 
        {
            thread.join();
        }
    }
}
