#pragma once

#include <new>
#include <thread>
#include <string>
#include <chrono>
#include <functional>

namespace DynXX::Core::Concurrent {

    static constexpr auto CacheLineSize =
#if defined(__cpp_lib_hardware_interference_size)
            std::hardware_destructive_interference_size
#else
    64uz
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

    inline void sleep(size_t microSecs) {
        std::this_thread::sleep_for(std::chrono::microseconds(microSecs));
    }

    template<typename T>
    concept TimedLockableT = requires(T mtx)
    {
        { mtx.try_lock_until(std::declval<std::chrono::time_point<std::chrono::steady_clock> >()) } -> std::convertible_to<bool>;
    };

    template<TimedLockableT T>
    [[nodiscard]] bool tryLockUntil(T &mtx, size_t timeoutMicroSecs) {
        if (timeoutMicroSecs == 0) {
            return mtx.try_lock();
        }
        const auto timeout = std::chrono::steady_clock::now() + std::chrono::microseconds(timeoutMicroSecs);
        return mtx.try_lock_until(timeout);
    }

    template<TimedLockableT T>
    [[nodiscard]] bool lockAutoRetry(T &mtx, size_t retryCount, size_t sleepMicroSecs = 1uz) {
        auto locked = false;
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
}
