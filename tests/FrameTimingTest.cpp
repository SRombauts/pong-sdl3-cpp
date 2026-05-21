#include "FrameTiming.h"

#include <doctest/doctest.h>

#include <cstdint>

TEST_CASE("FrameTiming::secondsBetween: equal timestamps yield zero")
{
    CHECK(FrameTiming::secondsBetween(0ULL, 0ULL) == doctest::Approx(0.0));
    CHECK(FrameTiming::secondsBetween(42ULL, 42ULL) == doctest::Approx(0.0));
}

TEST_CASE("FrameTiming::secondsBetween: one-second delta")
{
    constexpr std::uint64_t oneSecondNs = 1'000'000'000ULL;
    CHECK(FrameTiming::secondsBetween(0ULL, oneSecondNs) == doctest::Approx(1.0));
    CHECK(FrameTiming::secondsBetween(123'456ULL, 123'456ULL + oneSecondNs) == doctest::Approx(1.0));
}

TEST_CASE("FrameTiming::secondsBetween: sub-millisecond delta")
{
    // 500 microseconds = 500_000 ns = 5.0e-4 seconds.
    CHECK(FrameTiming::secondsBetween(0ULL, 500'000ULL) == doctest::Approx(5.0e-4));
}

TEST_CASE("FrameTiming::secondsBetween: now < prev defensive case")
{
    // Backward clock: clamp to 0.0 rather than feed a negative dt.
    CHECK(FrameTiming::secondsBetween(1'000'000ULL, 500'000ULL) == doctest::Approx(0.0));
    CHECK(FrameTiming::secondsBetween(1'000'000'000ULL, 0ULL) == doctest::Approx(0.0));
}
