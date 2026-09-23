#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <stdexcept>

#include "../src/core/concurrent/Executor.hxx"

class DynXXConcurrentTestSuite : public ::testing::Test {};

namespace {
    using DynXX::Core::Concurrent::Executor;
    using DynXX::Core::Concurrent::TaskT;
    using DynXX::Core::Concurrent::Worker;
}

TEST_F(DynXXConcurrentTestSuite, ExecutorIgnoresEmptyTask) {
    Executor executor(1);
    EXPECT_NO_THROW(executor >> TaskT{});
}

TEST_F(DynXXConcurrentTestSuite, WorkerSurvivesThrowingTasks) {
    Worker worker;

    std::mutex mtx;
    std::condition_variable cv;
    auto finished = false;

    // Every task throws a different exception type so each catch clause is reached;
    // the worker must stay alive and keep draining the queue afterwards.
    worker >> TaskT{[]() {
        const std::function<void()> empty;
        empty();
    }};
    worker >> TaskT{[]() { throw std::runtime_error("dynxx-worker-test"); }};
    worker >> TaskT{[]() { throw 42; }};
    worker >> TaskT{[&mtx, &cv, &finished]() {
        const auto lock = std::scoped_lock(mtx);
        finished = true;
        cv.notify_one();
    }};

    auto lock = std::unique_lock(mtx);
    EXPECT_TRUE(cv.wait_for(lock, std::chrono::seconds(10), [&]() { return finished; }));
}
