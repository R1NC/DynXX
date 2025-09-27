#include "Daemon.hxx"

#include <DynXX/CXX/Log.hxx>

namespace {
    using enum DynXXLogLevelX;
}

namespace DynXX::Core::Concurrent {

Daemon::Daemon(TaskT &&runLoop, RunChecker &&runChecker, size_t timeoutMicroSecs)
{
    this->thread = 
#if defined(__cpp_lib_jthread)
        std::jthread
#else
        std::thread
#endif
        ([timeout = std::chrono::microseconds(timeoutMicroSecs), 
            loop = std::move(runLoop), 
            checker = std::move(runChecker),
            &mtx = this->mutex, 
            &cv = this->loopCondition
#if !defined(__cpp_lib_jthread)
        , &stopFlag = this->shouldStop
#endif
        ](
#if defined(__cpp_lib_jthread)
        std::stop_token stoken
#endif
        ) 
#if defined(__cpp_lib_move_only_function)
        mutable 
#endif
        { 
        while (
#if defined(__cpp_lib_jthread)
        !stoken.stop_requested()
#else
        !stopFlag.load()
#endif
        ) {
            auto lock = std::unique_lock(mtx);
            // Wait for stop signal
            cv.wait_for(lock, timeout, [&checker, 
#if defined(__cpp_lib_jthread)
                &stoken
#else
                &stopFlag
#endif
                ]() {
                return checker() ||
#if defined(__cpp_lib_jthread)
                stoken.stop_requested()
#else
                stopFlag.load()
#endif
            ;});
            
            if (
#if defined(__cpp_lib_jthread)
                stoken.stop_requested()
#else
                stopFlag.load()
#endif
            ) { break; }

            loop();

            lock.unlock();
        }
    });
}

Daemon::~Daemon()
{
#if defined(__cpp_lib_jthread)
    // jthread automatically handles stop request and join
#else
    this->update([&stopFlag = this->shouldStop]() {
        stopFlag.store(true);
    });
    
    if (this->thread.joinable()) [[likely]] 
    {
        this->thread.join();
    }
#endif
}

} // namespace DynXX::Core::Concurrent