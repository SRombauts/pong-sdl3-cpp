#pragma once

#include "RandomSource.h"

#include <cstdint>
#include <random>

// Production IRandomSource backed by std::mt19937. The constructor takes an explicit 64-bit seed so tests can pin the
// sequence; makeNonDeterministicRandomSource() seeds from std::random_device for production callers that want a fresh
// sequence each launch.
//
// Non-copyable / non-movable by inheritance from IRandomSource (same shape as the IClock hierarchy). The free helper
// returns by value and relies on mandatory return-value optimization (C++17+) for prvalue returns.
class RandomSourceMt19937 : public IRandomSource
{
public:
    explicit RandomSourceMt19937(std::uint64_t seed);
    ~RandomSourceMt19937() override = default;

    int intInRange(int lo, int hi) override;
    double doubleInRange(double lo, double hi) override;

private:
    std::mt19937 m_engine;
};

// Production helper: build a RandomSourceMt19937 seeded from std::random_device. Two consecutive calls produce
// different sequences with overwhelming probability. Returns by value; the callee is non-movable, so the caller must
// consume the prvalue in place (e.g. `auto rng = makeNonDeterministicRandomSource();`).
RandomSourceMt19937 makeNonDeterministicRandomSource();

// Helper used by both makeNonDeterministicRandomSource() and Application's default-constructed random source: a 64-bit
// seed drawn from two std::random_device samples. Exposed so callers that need a unique_ptr<RandomSourceMt19937>
// (Application owns IRandomSource through one) can avoid duplicating the seeding boilerplate.
std::uint64_t makeNonDeterministicSeed();
