#include "ClockFake.h"

ClockFake::ClockFake(std::uint64_t initialNs) : m_now(initialNs) {}

std::uint64_t ClockFake::now() const
{
    return m_now;
}

void ClockFake::setNow(std::uint64_t ns)
{
    m_now = ns;
}

void ClockFake::advance(std::uint64_t ns)
{
    m_now += ns;
}
