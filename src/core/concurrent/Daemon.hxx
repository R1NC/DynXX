#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>

#if !defined(__cpp_lib_jthread)
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
        // Starts the run-loop thread. The base constructor must not start it:
        // the runLoop/runChecker lambdas may touch derived-class members 
        // that do not exist until the derived constructor body runs. 
        // Call this as the last statement of the derived constructor body.
        void start();
        void stopAndJoin();

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
        std::atomic<bool> stopped{false};
        mutable std::mutex mutex;
        std::condition_variable loopCondition;
        TaskT runLoop;
        RunChecker runChecker;
        size_t timeoutMicroSecs{100UZ};

#if defined(__cpp_lib_jthread)
        std::jthread thread;
#else
        std::thread thread;
        std::atomic<bool> shouldStop{false};
#endif
    };
}  // namespace DynXX::Core::Concurrent
