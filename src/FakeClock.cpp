#include "FakeClock.h"

FakeClock::FakeClock(std::uint64_t initialNs) : m_now(initialNs) {}

std::uint64_t FakeClock::now() const
{
    return m_now;
}

void FakeClock::setNow(std::uint64_t ns)
{
    m_now = ns;
}

void FakeClock::advance(std::uint64_t ns)
{
    m_now += ns;
}
