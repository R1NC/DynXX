#include "BaseVM.hxx"

#include <utility>

#include <NGenXXLog.hxx>
#include "../concurrent/ConcurrentUtil.hxx"

namespace
{
    constexpr auto SleepMicroSecs = 1000uz;
}

bool NGenXX::Core::VM::BaseVM::tryLock()
{
    return NGenXX::Core::Concurrent::tryLock(this->vmMutex, SleepMicroSecs);
}

void NGenXX::Core::VM::BaseVM::unlock()
{
    this->vmMutex.unlock();
}

void NGenXX::Core::VM::BaseVM::sleep()
{
    NGenXX::Core::Concurrent::sleep(SleepMicroSecs);
}

NGenXX::Core::VM::BaseVM::BaseVM()
{
    this->active = true;
}

NGenXX::Core::VM::BaseVM::~BaseVM()
{
    this->active = false;
}
