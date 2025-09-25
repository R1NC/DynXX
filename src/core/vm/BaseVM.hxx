#pragma once

#include <atomic>

#include "../concurrent/Executor.hxx"

namespace DynXX::Core::VM {
    class BaseVM : public std::enable_shared_from_this<BaseVM> {
    public:
        BaseVM();

        BaseVM(const BaseVM &) = delete;

        BaseVM &operator=(const BaseVM &) = delete;

        BaseVM(BaseVM &&) = delete;

        BaseVM &operator=(BaseVM &&) = delete;

        virtual ~BaseVM();

    protected:
        mutable std::recursive_timed_mutex vmMutex;

        [[nodiscard]] bool tryLockUntil(const size_t timeoutMicroSecs);
        [[nodiscard]] bool lockAutoRetry(const size_t retryCount, const size_t sleepMicroSecs);

        void unlock();

        void sleep() const;

        void submitTask(DynXX::Core::Concurrent::TaskT &&task);
    
    private:
        std::atomic<bool> active{false};
        Concurrent::Executor executor;
    };
}
