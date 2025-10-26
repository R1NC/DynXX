#pragma once

#include <chrono>

namespace DynXX::Core::Util::Time
{

template <typename clockT, typename durationT>
uint64_t castNow()
{
    return std::chrono::duration_cast<durationT>(clockT::now().time_since_epoch()).count();
}

inline uint64_t nowInMilliSecs()
{
    return castNow<std::chrono::system_clock, std::chrono::milliseconds>();
}

inline uint64_t nowInMicroSecs()
{
    return castNow<std::chrono::system_clock, std::chrono::microseconds>();
}

inline uint64_t nowInNanoSecs()
{
    return castNow<std::chrono::high_resolution_clock, std::chrono::nanoseconds>();
}

}  // namespace DynXX::Core::Util::Time
