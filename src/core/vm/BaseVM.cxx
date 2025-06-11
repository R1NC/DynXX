#include "BaseVM.hxx"

#include <utility>

#include <NGenXXLog.hxx>
#include "../util/TimeUtil.hxx"
#include "../concurrent/ConcurrentUtil.hxx"

namespace
{
    constexpr auto SleepMilliSecs = 1uz;
}

void NGenXX::Core::VM::BaseVM::sleep()
{
    Util::Time::sleepForMilliSecs(SleepMilliSecs);
}

bool NGenXX::Core::VM::BaseVM::tryLockMutex(std::recursive_timed_mutex &mtx)
{
    const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(SleepMilliSecs);
    return mtx.try_lock_until(timeout);
}

void NGenXX::Core::VM::BaseVM::unlockMutex(std::recursive_timed_mutex &mtx)
{
    mtx.unlock();
}

NGenXX::Core::VM::BaseVM::BaseVM()
{
    this->active = true;
}

NGenXX::Core::VM::BaseVM::~BaseVM()
{
    this->active = false;
}
