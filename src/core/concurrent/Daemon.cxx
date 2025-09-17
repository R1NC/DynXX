#include "Daemon.hxx"

#include <DynXX/CXX/Log.hxx>

namespace {
    using enum DynXXLogLevelX;
}

namespace DynXX::Core::Concurrent {

Daemon::Daemon(TaskT &&runLoop, RunChecker &&runChecker, const size_t timeoutMicroSecs) :
 runLoop(std::move(runLoop)), runChecker(std::move(runChecker))
{
    this->thread = 
#if defined(__cpp_lib_jthread)
        std::jthread(
#else
        std::thread(
#endif
        [this, timeout = timeoutMicroSecs]
#if defined(__cpp_lib_jthread)
        (std::stop_token stoken) {
        while (!stoken.stop_requested())
#else
        () {
        while (!shouldStop.load())
#endif
        {
            auto lock = std::unique_lock(this->mutex);
            // Wait for stop signal
            this->cv.wait_for(lock, std::chrono::microseconds(timeout), [this
#if defined(__cpp_lib_jthread)
                , &stoken
#endif
                ]() {
                return this->runChecker() ||
#if defined(__cpp_lib_jthread)
                stoken.stop_requested()
#else
                shouldStop.load()
#endif
            ;});
            
            if (
#if defined(__cpp_lib_jthread)
                stoken.stop_requested()
#else
                shouldStop.load()
#endif
            ) { break; }

            this->runLoop();

            lock.unlock();
        }
    });
}

void Daemon::update(TaskT &&sth)
{
    {
        auto lock = std::scoped_lock(this->mutex);
        std::move(sth)();
    }
    
    this->cv.notify_one();
}

Daemon::~Daemon()
{
#if defined(__cpp_lib_jthread)
    // jthread automatically handles stop request and join
#else
    this->update([this]() {
        this->shouldStop = true;
    });
    
    if (this->thread.joinable()) [[likely]] 
    {
        this->thread.join();
    }
#endif
}

} // namespace DynXX::Core::Concurrent