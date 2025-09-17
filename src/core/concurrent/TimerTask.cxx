#include "TimerTask.hxx"

#include <DynXX/CXX/Log.hxx>

#include "../util/TimeUtil.hxx"

namespace {
    using enum DynXXLogLevelX;
    using namespace DynXX::Core::Util::Time;
}

namespace DynXX::Core::Concurrent {

TimerTask::TimerTask(TaskT&& task, size_t timeoutMicroSecs) :
    Daemon(
        [this]() {
            this->userTask();
            this->lastExecuteTime = nowInMicroSecs();
        },
        [this, timeoutMicroSecs]() {
            const auto now = nowInMicroSecs();
            return now - this->lastExecuteTime >= timeoutMicroSecs;
        },
        timeoutMicroSecs
    ),
    userTask(std::move(task))
{
    this->lastExecuteTime = nowInMicroSecs();
}

} // namespace DynXX::Core::Concurrent