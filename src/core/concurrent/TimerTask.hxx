#ifndef DYNXX_SRC_CORE_CONCURRENT_TIMERTASK_HXX_
#define DYNXX_SRC_CORE_CONCURRENT_TIMERTASK_HXX_

#if defined(__cplusplus)

#include "Daemon.hxx"

namespace DynXX::Core::Concurrent {

    class TimerTask final : public Daemon {
    public:
        TimerTask() = delete;
        explicit TimerTask(TaskT&& task, size_t timeoutMicroSecs);
        
        TimerTask(const TimerTask&) = delete;
        TimerTask& operator=(const TimerTask&) = delete;
        TimerTask(TimerTask&&) = delete;
        TimerTask& operator=(TimerTask&&) = delete;
        
        ~TimerTask() override = default;
        
    private:
        TaskT userTask;
        size_t lastExecuteTime{0};
    };
}

#endif

#endif // DYNXX_SRC_CORE_CONCURRENT_TIMERTASK_HXX_
