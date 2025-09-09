#ifndef DYNXX_SRC_CORE_CONCURRENT_CONCURRENTUTIL_HXX_
#define DYNXX_SRC_CORE_CONCURRENT_CONCURRENTUTIL_HXX_

#if defined(__cplusplus)

#include <new>
#include <mutex>
#include <thread>
#include <string>
#include <chrono>
#include <functional>

namespace DynXX::Core::Concurrent {

    static constexpr size_t CacheLineSize =
#if defined(__cpp_lib_hardware_interference_size)
            std::hardware_destructive_interference_size
#else
    64
#endif
    ;


    inline unsigned int countCPUCore() {
        return std::thread::hardware_concurrency();
    }

    inline std::string currentThreadId() {
        thread_local std::string cachedCurrentThreadId;
        if (cachedCurrentThreadId.empty()) {
            const auto &id = std::this_thread::get_id();
            cachedCurrentThreadId = std::to_string(std::hash<std::thread::id>{}(id));
        }
        return cachedCurrentThreadId;
    }

    inline void sleep(const size_t microSecs) {
        std::this_thread::sleep_for(std::chrono::microseconds(microSecs));
    }

    template<typename T>
    concept TimedLockableT = requires(T mtx)
    {
        { mtx.try_lock_until(std::declval<std::chrono::time_point<std::chrono::steady_clock> >()) } -> std::convertible_to<bool>;
    };

    template<TimedLockableT T>
    [[nodiscard]] bool tryLockUntil(T &mtx, const size_t timeoutMicroSecs) {
        if (timeoutMicroSecs == 0) {
            return mtx.try_lock();
        }
        const auto timeout = std::chrono::steady_clock::now() + std::chrono::microseconds(timeoutMicroSecs);
        return mtx.try_lock_until(timeout);
    }

    template<TimedLockableT T>
    [[nodiscard]] bool lockAutoRetry(T &mtx, const size_t retryCount, const size_t sleepMicroSecs = 1uz) {
        bool locked = false;
        size_t count = 0;
        do {
            locked = mtx.try_lock();
            if (locked) {
                return true;
            }
            if (retryCount == 0) [[unlikely]] {
                return false;
            }
            if (sleepMicroSecs > 0) [[likely]] {
                sleep(sleepMicroSecs);
            }
            count++;
        } while (count < retryCount);
        return false;
    }

    template<typename T>
    concept RunnableT = requires(T t)
    {
        { t() } -> std::same_as<void>;
    };

    template<RunnableT T>
    void callOnce(T &&func) {
        static std::once_flag flag;
        std::call_once(flag, std::forward<T>(func));
    }
}

#endif

#endif // DYNXX_SRC_CORE_CONCURRENT_CONCURRENTUTIL_HXX_
