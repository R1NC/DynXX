#if defined(USE_QJS) || defined(USE_LUA)

#include "BaseVM.hxx"

#include <DynXX/CXX/Log.hxx>
#include "../concurrent/ConcurrentUtil.hxx"

namespace
{
    constexpr auto SleepMicroSecs = 1000uz;
}

namespace DynXX::Core::VM {

bool BaseVM::tryLockUntil(const size_t timeoutMicroSecs)
{
    return DynXX::Core::Concurrent::tryLockUntil(this->vmMutex, timeoutMicroSecs);
}

bool BaseVM::lockAutoRetry(const size_t retryCount, const size_t sleepMicroSecs)
{
    return DynXX::Core::Concurrent::lockAutoRetry(this->vmMutex, retryCount, sleepMicroSecs);
}

void BaseVM::unlock()
{
    this->vmMutex.unlock();
}

void BaseVM::sleep() const
{
    DynXX::Core::Concurrent::sleep(SleepMicroSecs);
}

BaseVM::BaseVM() : active(true)
{
}

BaseVM::~BaseVM()
{
    this->active = false;
}

} // namespace BaseVM

#endif
