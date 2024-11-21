#ifndef NGENXX_SRC_UTIL_TIME_HXX_
#define NGENXX_SRC_UTIL_TIME_HXX_

#ifdef __cplusplus

#include <chrono>

static inline const uint64_t timeStampInMicroSecs()
{
    using namespace std::chrono;
    return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
}

#endif

#endif // NGENXX_SRC_UTIL_TIME_HXX_