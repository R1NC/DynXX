#if defined(USE_QJS) || defined(USE_LUA)

#include "BaseVM.hxx"

#include <DynXX/CXX/Log.hxx>
#include "../concurrent/ConcurrentUtil.hxx"

namespace
{
    constexpr auto SleepMicroSecs = 1000uz;

    using namespace DynXX::Core;
}

namespace DynXX::Core::VM {

BaseVM::BaseVM() : active(true)
{
}

bool BaseVM::tryLockUntil(const size_t timeoutMicroSecs)
{
    return Concurrent::tryLockUntil(this->vmMutex, timeoutMicroSecs);
}

bool BaseVM::lockAutoRetry(const size_t retryCount, const size_t sleepMicroSecs)
{
    return Concurrent::lockAutoRetry(this->vmMutex, retryCount, sleepMicroSecs);
}

void BaseVM::unlock()
{
    this->vmMutex.unlock();
}

void BaseVM::sleep() const
{
    Concurrent::sleep(SleepMicroSecs);
}

void BaseVM::submitTask(Concurrent::TaskT &&task)
{
    this->executor >> std::move(task);
}

BaseVM::~BaseVM()
{
    this->active = false;
}

} // namespace DynXX::Core::VM

#endif
