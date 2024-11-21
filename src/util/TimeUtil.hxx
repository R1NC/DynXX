#ifndef NGENXX_SRC_UTIL_TIME_HXX_
#define NGENXX_SRC_UTIL_TIME_HXX_

#ifdef __cplusplus

#include <chrono>

using namespace std::chrono;

template<typename clockT, typename durationT>
static inline const uint64_t castNow()
{
    return duration_cast<durationT>(clockT::now().time_since_epoch()).count();
}

static inline const uint64_t nowInMicroSecs()
{
    return castNow<system_clock, microseconds>();
}

static inline const uint64_t nowInNanoSecs()
{
    return castNow<high_resolution_clock, nanoseconds>();
}

#endif

#endif // NGENXX_SRC_UTIL_TIME_HXX_