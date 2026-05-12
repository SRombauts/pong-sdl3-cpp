#include "RandomSourceMt19937.h"

#include <random>

RandomSourceMt19937::RandomSourceMt19937(std::uint64_t seed) : m_engine(static_cast<std::mt19937::result_type>(seed)) {}

int RandomSourceMt19937::intInRange(int lo, int hi)
{
    std::uniform_int_distribution<int> dist(lo, hi);
    return dist(m_engine);
}

double RandomSourceMt19937::doubleInRange(double lo, double hi)
{
    std::uniform_real_distribution<double> dist(lo, hi);
    return dist(m_engine);
}

std::uint64_t makeNonDeterministicSeed()
{
    // std::random_device::operator() returns a 32-bit value on most
    // implementations. Concatenate two draws into the 64-bit seed
    // surface even though the current RandomSourceMt19937 constructor
    // truncates to mt19937::result_type (uint_fast32_t): the API is
    // already 64-bit so consumers won't churn the day we widen the
    // engine to mt19937_64 or feed a std::seed_seq.
    std::random_device rd;
    return (static_cast<std::uint64_t>(rd()) << 32) | static_cast<std::uint64_t>(rd());
}

RandomSourceMt19937 makeNonDeterministicRandomSource()
{
    return RandomSourceMt19937(makeNonDeterministicSeed());
}
