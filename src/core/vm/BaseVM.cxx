#include "BaseVM.hxx"

#include <utility>

#include <NGenXXLog.hxx>
#include "../concurrent/ConcurrentUtil.hxx"

namespace
{
    constexpr auto SleepMicroSecs = 1000uz;
}

void NGenXX::Core::VM::BaseVM::sleep()
{
    NGenXX::Core::Concurrent::sleep(SleepMicroSecs);
}

bool NGenXX::Core::VM::BaseVM::tryLock()
{
    NGenXX::Core::Concurrent::tryLockMutex(this->vmMutex, SleepMicroSecs);
}

void NGenXX::Core::VM::BaseVM::unlock()
{
    this->vmMutex.unlock();
}

NGenXX::Core::VM::BaseVM::BaseVM()
{
    this->active = true;
}

NGenXX::Core::VM::BaseVM::~BaseVM()
{
    this->active = false;
}
