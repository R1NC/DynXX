#ifndef NGENXX_SRC_BRIDGE_BASE_HXX_
#define NGENXX_SRC_BRIDGE_BASE_HXX_

#if defined(__cplusplus)

#include <atomic>
#include <mutex>
#include <functional>
#include <thread>
#include <queue>
#include <vector>

namespace NGenXX::Bridge
{
    class BaseBridge {
    public:
        BaseBridge();
        BaseBridge(const BaseBridge &) = delete;
        BaseBridge &operator=(const BaseBridge &) = delete;
        BaseBridge(BaseBridge &&) = delete;
        BaseBridge &operator=(BaseBridge &&) = delete;

        static bool tryLockMutex(std::recursive_timed_mutex &mtx);
        static void unlockMutex(std::recursive_timed_mutex &mtx);

        virtual ~BaseBridge();

    protected:
        std::atomic<bool> active{false};
        std::recursive_timed_mutex vmMutex;

        void sleep();

        void enqueueTask(const std::function<void()> &&taskF);

    private:
        std::recursive_timed_mutex threadPoolMutex;
        std::vector<std::thread> threadPool;
        std::queue<std::function<void()>> taskQueue;
    };
}

#endif

#endif // NGENXX_SRC_BRIDGE_BASE_HXX_
