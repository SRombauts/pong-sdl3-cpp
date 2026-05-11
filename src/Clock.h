#pragma once

#include <cstdint>

// Minimal monotonic clock injected into Application. Single method on
// purpose: helpers like sleep() / nowSeconds() would expand the
// test-doubles surface without earning their keep. Seconds conversion
// lives in FrameTiming::secondsBetween().
//
// Implementations: SdlTicksClock (production, wraps SDL_GetTicksNS),
// FakeClock (tests, scripted via setNow / advance).
class IClock
{
public:
    IClock() = default;
    virtual ~IClock() = default;

    IClock(const IClock&) = delete;
    IClock& operator=(const IClock&) = delete;
    IClock(IClock&&) = delete;
    IClock& operator=(IClock&&) = delete;

    // Monotonic nanosecond timestamp. FrameTiming::secondsBetween()
    // defends against the degenerate non-monotonic case.
    virtual std::uint64_t now() const = 0;
};
