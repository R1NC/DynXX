#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

#if !defined(__cpp_lib_jthread)
#include <atomic>
#include "ConcurrentUtil.hxx"
#endif

#include <DynXX/CXX/Types.hxx>

namespace DynXX::Core::Concurrent {
    using TaskT = std::
#if defined(__cpp_lib_move_only_function)
    move_only_function
#else
    function
#endif
    <void()>;

    using RunChecker = std::function<bool()>;

    class
#if !defined(__cpp_lib_jthread)
        alignas(CacheLineSize)
#endif
        Daemon {
    protected:
        explicit Daemon(TaskT &&runLoop, RunChecker &&runChecker = []() { return true; }, size_t timeoutMicroSecs = 100UZ);

        template<RunnableT T>
        void update(T &&f) {
            {
                const auto lock = std::scoped_lock(this->mutex);
                std::invoke(std::forward<T>(f));
            }
            this->loopCondition.notify_one();
        }

    public:
        Daemon() = delete;
        Daemon(const Daemon &) = delete;
        Daemon &operator=(const Daemon &) = delete;
        Daemon(Daemon &&) = delete;
        Daemon &operator=(Daemon &&) = delete;
        virtual ~Daemon();

    private:
        mutable std::mutex mutex;
        std::condition_variable loopCondition;

#if defined(__cpp_lib_jthread)
        std::jthread thread;
#else
        std::thread thread;
        std::atomic<bool> shouldStop{false};
#endif
    };
}  // namespace DynXX::Core::Concurrent
