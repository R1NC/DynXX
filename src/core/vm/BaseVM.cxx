#if defined(USE_QJS) || defined(USE_LUA)

#include "BaseVM.hxx"

#include <DynXX/CXX/Log.hxx>
#include "../concurrent/ConcurrentUtil.hxx"

namespace
{
    constexpr auto SleepMicroSecs = 1000uz;
}

bool DynXX::Core::VM::BaseVM::tryLockUntil(const size_t timeoutMicroSecs)
{
    return DynXX::Core::Concurrent::tryLockUntil(this->vmMutex, timeoutMicroSecs);
}

bool DynXX::Core::VM::BaseVM::lockAutoRetry(const size_t retryCount, const size_t sleepMicroSecs)
{
    return DynXX::Core::Concurrent::lockAutoRetry(this->vmMutex, retryCount, sleepMicroSecs);
}

void DynXX::Core::VM::BaseVM::unlock()
{
    this->vmMutex.unlock();
}

void DynXX::Core::VM::BaseVM::sleep() const
{
    DynXX::Core::Concurrent::sleep(SleepMicroSecs);
}

DynXX::Core::VM::BaseVM::BaseVM() : active(true)
{
}

DynXX::Core::VM::BaseVM::~BaseVM()
{
    this->active = false;
}

#endif
