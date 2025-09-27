#pragma once

#include <atomic>

#include "../concurrent/Executor.hxx"

namespace DynXX::Core::VM {
    class BaseVM : public std::enable_shared_from_this<BaseVM> {
    public:
        BaseVM();

        virtual ~BaseVM();

        BaseVM(const BaseVM &) = delete;
        BaseVM &operator=(const BaseVM &) = delete;
        BaseVM(BaseVM &&) = delete;
        BaseVM &operator=(BaseVM &&) = delete;

    protected:
        mutable std::recursive_timed_mutex vmMutex;

        [[nodiscard]] bool tryLockUntil(size_t timeoutMicroSecs);
        [[nodiscard]] bool lockAutoRetry(size_t retryCount, size_t sleepMicroSecs);

        void unlock();

        void sleep() const;

        void submitTask(DynXX::Core::Concurrent::TaskT &&task);
    
    private:
        std::atomic<bool> active{false};
        Concurrent::Executor executor;
    };
}
