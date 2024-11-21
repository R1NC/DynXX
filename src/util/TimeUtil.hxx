#ifndef NGENXX_SRC_UTIL_TIME_HXX_
#define NGENXX_SRC_UTIL_TIME_HXX_

#ifdef __cplusplus

#include <chrono>

using namespace std::chrono;

template<typename timePointT, typename durationT>
static inline const uint64_t castNow(timePointT now)
{
    return duration_cast<durationT>(now.time_since_epoch()).count();
}

static inline const uint64_t nowInMicroSecs()
{
    return castNow<system_clock::time_point, microseconds>(system_clock::now());
}

static inline const uint64_t nowInNanoSecs()
{
    return castNow<high_resolution_clock::time_point, nanoseconds>(high_resolution_clock::now());
}

#endif

#endif // NGENXX_SRC_UTIL_TIME_HXX_