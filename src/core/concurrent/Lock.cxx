#include "Lock.hxx"

NGenXX::Core::Concurrent::SpinLock::SpinLock()
{
}

NGenXX::Core::Concurrent::SpinLock::~SpinLock()
{
}

void NGenXX::Core::Concurrent::SpinLock::lock() 
{
    while (lockFlag.exchange(true, std::memory_order_acquire));
}

void NGenXX::Core::Concurrent::SpinLock::unlock() 
{
    lockFlag.store(false, std::memory_order_release);
}
