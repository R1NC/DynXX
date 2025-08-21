#include "Lock.hxx"

void DynXX::Core::Concurrent::SpinLock::lock() 
{
    while (lockFlag.exchange(true, std::memory_order_acquire));
}

void DynXX::Core::Concurrent::SpinLock::unlock() 
{
    lockFlag.store(false, std::memory_order_release);
}
