#pragma once

#include "Clock.h"

#include <cstdint>

// Test-only IClock returning scripted values. Lives in ${PONG_SRC} so
// any TEST_CASE can use it without extra CMake plumbing.
class ClockFake : public IClock
{
public:
    ClockFake() = default;
    explicit ClockFake(std::uint64_t initialNs);
    ~ClockFake() override = default;

    std::uint64_t now() const override;

    // Replace the scripted timestamp with an absolute value.
    void setNow(std::uint64_t ns);

    // Tick forward by a relative amount.
    void advance(std::uint64_t ns);

private:
    std::uint64_t m_now = 0;
};
