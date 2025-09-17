#include "Lock.hxx"

namespace DynXX::Core::Concurrent {

void SpinLock::lock() 
{
    while (lockFlag.exchange(true, std::memory_order_acquire));
}

void SpinLock::unlock() 
{
    lockFlag.store(false, std::memory_order_release);
}

} // namespace DynXX::Core::Concurrent
