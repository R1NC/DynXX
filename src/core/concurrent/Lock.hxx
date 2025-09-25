#pragma once

#include <atomic>

#include "ConcurrentUtil.hxx"

namespace DynXX::Core::Concurrent
{

class alignas(CacheLineSize) SpinLock final
{
public:
    SpinLock() = default;
    SpinLock(const SpinLock &) = delete;
    SpinLock &operator=(const SpinLock &) = delete;
    SpinLock(SpinLock &&) = delete;
    SpinLock &operator=(SpinLock &&) = delete;
    ~SpinLock() = default;

    void lock();
    void unlock();

private:
    std::atomic<bool> lockFlag = {false};
};

}
