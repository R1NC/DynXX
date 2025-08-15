#ifndef DYNXX_SRC_CORE_UTIL_TIMEUTIL_HXX_
#define DYNXX_SRC_CORE_UTIL_TIMEUTIL_HXX_

#if defined(__cplusplus)

#include <chrono>

namespace DynXX::Core::Util::Time
{

using namespace std::chrono;

template <typename clockT, typename durationT>
uint64_t castNow()
{
    return duration_cast<durationT>(clockT::now().time_since_epoch()).count();
}

inline uint64_t nowInMilliSecs()
{
    return castNow<system_clock, milliseconds>();
}

inline uint64_t nowInMicroSecs()
{
    return castNow<system_clock, microseconds>();
}

inline uint64_t nowInNanoSecs()
{
    return castNow<high_resolution_clock, nanoseconds>();
}

}

#endif

#endif // DYNXX_SRC_CORE_UTIL_TIMEUTIL_HXX_
