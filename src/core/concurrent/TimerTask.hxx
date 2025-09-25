#pragma once

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
