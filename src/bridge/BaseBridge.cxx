#include "BaseBridge.hxx"

#include <utility>

#include <NGenXXLog.hxx>
#include "../util/TimeUtil.hxx"

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

std::unique_ptr<std::thread> NGenXX::Bridge::BaseBridge::schedule(std::function<void()> &&cbk, std::recursive_timed_mutex &mtx)
{
    auto ptr = std::make_unique<std::thread>([&stopFlag = this->shouldStop, &mtx = mtx, cbk = std::move(cbk)]
    {
        while (!stopFlag)
        {
            if (tryLockMutex(mtx))
            {
                cbk();
                unlockMutex(mtx);
            }
            sleepForMilliSecs(SleepMilliSecs);
        }
    });
    ptr->detach();
    return ptr;
}

NGenXX::Bridge::BaseBridge::BaseBridge()
{
    this->shouldStop = false;
}

NGenXX::Bridge::BaseBridge::~BaseBridge()
{
    this->shouldStop = true;
}
