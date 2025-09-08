#if defined(USE_QJS) || defined(USE_LUA)

#include "BaseVM.hxx"

#include <DynXX/CXX/Log.hxx>
#include "../concurrent/ConcurrentUtil.hxx"

namespace
{
    constexpr auto SleepMicroSecs = 1000uz;
}

bool DynXX::Core::VM::BaseVM::tryLock(size_t timeoutMicroSecs)
{
    return DynXX::Core::Concurrent::tryLock(this->vmMutex, timeoutMicroSecs);
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
