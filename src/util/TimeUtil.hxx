#ifndef NGENXX_SRC_UTIL_TIME_HXX_
#define NGENXX_SRC_UTIL_TIME_HXX_

#ifdef __cplusplus

#include <chrono>

static inline const uint64_t nowInNanoSecs()
{
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    return duration_cast<nanoseconds>(now.time_since_epoch()).count();
}

#endif

#endif // NGENXX_SRC_UTIL_TIME_HXX_