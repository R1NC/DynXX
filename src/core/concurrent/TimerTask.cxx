#include "TimerTask.hxx"

#include <DynXX/CXX/Log.hxx>

#include "../util/TimeUtil.hxx"

namespace {
    using enum DynXXLogLevelX;
    using namespace DynXX::Core::Util::Time;
}

DynXX::Core::Concurrent::TimerTask::TimerTask(TaskT&& task, size_t timeoutMicroSecs) :
    Daemon(
        [this]() {
            const auto addr = reinterpret_cast<uintptr_t>(this);
            try {
                this->userTask();
            } catch (const std::exception& e) {
                dynxxLogPrintF(Error, "TimerTask@{} exception: {}", addr, e.what());
            }
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