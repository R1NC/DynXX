#pragma once

#include "Daemon.hxx"

namespace DynXX::Core::Concurrent {

    class TimerTask final : public Daemon {
    public:
        explicit TimerTask(TaskT&& task, size_t timeoutMicroSecs);
        
        ~TimerTask() override = default;
        
    private:
        TaskT userTask;
        size_t lastExecuteTime{0};
    };
}
