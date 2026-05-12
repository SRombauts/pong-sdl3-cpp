#pragma once

// Minimal random-number interface injected into Application. Modeled on
// IClock: the surface stays intentionally small (typed range helpers
// only) so test fakes are trivial and call sites read like English.
//
// We deliberately do NOT expose a generic next() returning a raw
// uint32_t -- gameplay code should call a typed range helper, which
// keeps the call sites readable and avoids re-implementing range
// reduction at every consumer.
//
// Implementations: RandomSourceMt19937 (production, std::mt19937 seeded
// via constructor argument; a non-deterministic seed helper lives next
// to it for production use). Tests can either pin the seed on
// RandomSourceMt19937 directly or, once consumers exist, swap in a
// scripted fake.
class IRandomSource
{
public:
    IRandomSource() = default;
    virtual ~IRandomSource() = default;

    IRandomSource(const IRandomSource&) = delete;
    IRandomSource& operator=(const IRandomSource&) = delete;
    IRandomSource(IRandomSource&&) = delete;
    IRandomSource& operator=(IRandomSource&&) = delete;

    // Uniform integer in the inclusive range [lo, hi]. Precondition:
    // lo <= hi (matches std::uniform_int_distribution).
    virtual int intInRange(int lo, int hi) = 0;

    // Uniform double in the half-open range [lo, hi). Precondition:
    // lo <= hi (matches std::uniform_real_distribution).
    virtual double doubleInRange(double lo, double hi) = 0;
};
