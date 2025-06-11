#ifndef NGENXX_SRC_CORE_VM_BASEVM_HXX_
#define NGENXX_SRC_CORE_VM_BASEVM_HXX_

#if defined(__cplusplus)

#include "../concurrent/Executor.hxx"

namespace NGenXX::Core::VM
{
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
        std::recursive_timed_mutex vmMutex;
        Concurrent::Executor executor;

        [[nodiscard]] bool tryLock();
        void unlock();
        void sleep();
    };
}

#endif

#endif // NGENXX_SRC_CORE_VM_BASEVM_HXX_
