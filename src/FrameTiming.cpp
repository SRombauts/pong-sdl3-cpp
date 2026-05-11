#include "FrameTiming.h"

namespace
{
constexpr double kNanosecondsPerSecond = 1.0e9;
}

double secondsBetween(std::uint64_t prevNs, std::uint64_t nowNs)
{
    if (nowNs <= prevNs)
    {
        return 0.0;
    }
    return static_cast<double>(nowNs - prevNs) / kNanosecondsPerSecond;
}
