#ifndef DYNXX_SRC_CORE_CONCURRENT_DAEMON_HXX_
#define DYNXX_SRC_CORE_CONCURRENT_DAEMON_HXX_

#if defined(__cplusplus)

#include <atomic>
#include <condition_variable>
#include <functional>

#include "ConcurrentUtil.hxx"

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
        Daemon() = delete;

        explicit Daemon(TaskT &&runLoop, RunChecker &&runChecker = []() { return true; }, const size_t timeoutMicroSecs = 100);

        Daemon(const Daemon &) = delete;

        Daemon &operator=(const Daemon &) = delete;

        Daemon(Daemon &&) = delete;

        Daemon &operator=(Daemon &&) = delete;

    public:

        virtual ~Daemon();

        void update(TaskT &&sth);

    private:
        mutable std::mutex mutex;
        std::condition_variable cv;

        TaskT runLoop;
        RunChecker runChecker;

#if defined(__cpp_lib_jthread)
        std::jthread thread;
#else
        std::thread thread;
        std::atomic<bool> shouldStop{false};
#endif
    };
}

#endif

#endif // DYNXX_SRC_CORE_CONCURRENT_DAEMON_HXX_
