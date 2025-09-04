#ifndef DYNXX_SRC_CORE_VM_BASEVM_HXX_
#define DYNXX_SRC_CORE_VM_BASEVM_HXX_

#if defined(__cplusplus)

#include <atomic>

#include "../concurrent/Executor.hxx"

namespace DynXX::Core::VM {
    class BaseVM {
    public:
        BaseVM();

        BaseVM(const BaseVM &) = delete;

        BaseVM &operator=(const BaseVM &) = delete;

        BaseVM(BaseVM &&) = delete;

        BaseVM &operator=(BaseVM &&) = delete;

        virtual ~BaseVM();

    protected:
        std::atomic<bool> active{false};
        mutable std::recursive_timed_mutex vmMutex;
        Concurrent::Executor executor;

        [[nodiscard]] bool tryLock();

        void unlock();

        void sleep() const;
    };
}

#endif

#endif // DYNXX_SRC_CORE_VM_BASEVM_HXX_
