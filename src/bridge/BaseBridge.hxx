#ifndef NGENXX_SRC_BRIDGE_BASE_HXX_
#define NGENXX_SRC_BRIDGE_BASE_HXX_

#if defined(__cplusplus)

#include <atomic>
#include <mutex>
#include <functional>
#include <thread>

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
        std::atomic<bool> shouldStop{false};

        std::unique_ptr<std::thread> schedule(std::function<void()> &&cbk, std::recursive_timed_mutex &mtx);
    };
}

#endif

#endif // NGENXX_SRC_BRIDGE_BASE_HXX_
