#ifndef NGENXX_SRC_CORE_CONCURRENT_LOCK_HXX_
#define NGENXX_SRC_CORE_CONCURRENT_LOCK_HXX_

#if defined(__cplusplus)

#include <atomic>

#include "ConcurrentUtil.hxx"

namespace NGenXX::Core::Concurrent
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

#endif // NGENXX_SRC_CORE_CONCURRENT_LOCK_HXX_
