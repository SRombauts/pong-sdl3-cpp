#pragma once

// Minimal random-number interface injected into Application. Modeled on IClock: a small surface (typed range helpers
// only) keeps fakes trivial and call sites readable.
//
// No generic next() returning a raw uint32_t -- gameplay code calls a typed range helper to keep call sites readable
// and avoid re-implementing range reduction at every consumer.
//
// Implementations: RandomSourceMt19937 (production, std::mt19937 seeded via constructor; makeNonDeterministicSeed()
// supplies an entropy-seeded default). Tests pin the seed directly or swap in a scripted fake.
class IRandomSource
{
public:
    IRandomSource() = default;
    virtual ~IRandomSource() = default;

    IRandomSource(const IRandomSource&) = delete;
    IRandomSource& operator=(const IRandomSource&) = delete;
    IRandomSource(IRandomSource&&) = delete;
    IRandomSource& operator=(IRandomSource&&) = delete;

    // Uniform integer in the inclusive range [lo, hi]. Precondition: lo <= hi (matches std::uniform_int_distribution).
    virtual int intInRange(int lo, int hi) = 0;

    // Uniform double in the half-open range [lo, hi). Precondition: lo <= hi (matches std::uniform_real_distribution).
    virtual double doubleInRange(double lo, double hi) = 0;
};
