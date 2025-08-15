#ifndef DYNXX_SRC_CORE_CONCURRENT_LOCK_HXX_
#define DYNXX_SRC_CORE_CONCURRENT_LOCK_HXX_

#if defined(__cplusplus)

#include <atomic>

#include "ConcurrentUtil.hxx"

namespace DynXX::Core::Concurrent
{

class alignas(CacheLineSize) SpinLock final
{
public:
    SpinLock();
    SpinLock(const SpinLock &) = delete;
    SpinLock &operator=(const SpinLock &) = delete;
    SpinLock(SpinLock &&) = delete;
    SpinLock &operator=(SpinLock &&) = delete;
    ~SpinLock();

    void lock();
    void unlock();

private:
    std::atomic<bool> lockFlag = {false};
};

}

#endif

#endif // DYNXX_SRC_CORE_CONCURRENT_LOCK_HXX_
