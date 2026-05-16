#include "RandomSourceMt19937.h"

#include <doctest/doctest.h>

#include <cstdint>

// Determinism guard: two instances seeded with the same value must produce byte-identical sequences. This is the
// contract gameplay tests will rely on to pin a serve angle or AI noise to a known outcome.
TEST_CASE("RandomSourceMt19937: same seed yields the same sequence on independent instances")
{
    constexpr std::uint64_t seed = 0xDEADBEEFCAFEBABEULL;
    RandomSourceMt19937 a(seed);
    RandomSourceMt19937 b(seed);

    for (int i = 0; i < 64; ++i)
    {
        CHECK(a.intInRange(-1000, 1000) == b.intInRange(-1000, 1000));
    }
    for (int i = 0; i < 64; ++i)
    {
        CHECK(a.doubleInRange(-1.0, 1.0) == b.doubleInRange(-1.0, 1.0));
    }
}

// intInRange contract: inclusive on both ends, never escapes the requested band. The bounds-hit checks turn the
// range size down to nine values so 1000 samples are statistically certain to land on both extremes; if either
// flag never flips, the implementation is almost certainly off-by-one.
TEST_CASE("RandomSourceMt19937::intInRange respects inclusive bounds")
{
    RandomSourceMt19937 rng(42);
    constexpr int lo = -3;
    constexpr int hi = 5;
    bool sawLo = false;
    bool sawHi = false;
    for (int i = 0; i < 1000; ++i)
    {
        const int v = rng.intInRange(lo, hi);
        CHECK(v >= lo);
        CHECK(v <= hi);
        if (v == lo)
        {
            sawLo = true;
        }
        if (v == hi)
        {
            sawHi = true;
        }
    }
    CHECK(sawLo);
    CHECK(sawHi);
}

// intInRange degenerate case: lo == hi must always return lo; a regression here betrays a faulty range translation.
TEST_CASE("RandomSourceMt19937::intInRange with lo == hi returns the constant")
{
    RandomSourceMt19937 rng(42);
    for (int i = 0; i < 16; ++i)
    {
        CHECK(rng.intInRange(7, 7) == 7);
    }
}

// doubleInRange contract: half-open. The upper bound must NEVER be returned. A loose `<=` implementation would
// silently break downstream code that uses the sample as an array index.
TEST_CASE("RandomSourceMt19937::doubleInRange respects half-open bounds")
{
    RandomSourceMt19937 rng(42);
    constexpr double lo = -1.5;
    constexpr double hi = 2.5;
    for (int i = 0; i < 1000; ++i)
    {
        const double v = rng.doubleInRange(lo, hi);
        CHECK(v >= lo);
        CHECK(v < hi);
    }
}

// Non-deterministic helper: two consecutive calls must produce distinct sequences. Statistical, not strict; with
// 32+ bits of entropy per seed the probability of an accidental match is vanishingly small.
TEST_CASE("makeNonDeterministicRandomSource produces different sequences across calls")
{
    auto a = makeNonDeterministicRandomSource();
    auto b = makeNonDeterministicRandomSource();

    bool allEqual = true;
    for (int i = 0; i < 8; ++i)
    {
        if (a.intInRange(0, 1'000'000) != b.intInRange(0, 1'000'000))
        {
            allEqual = false;
            break;
        }
    }
    CHECK_FALSE(allEqual);
}
