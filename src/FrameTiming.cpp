#include "FrameTiming.h"

// TDD: deliberately-wrong stubs. The header documents the intended
// contract; this implementation exists only to satisfy the linker while
// the tests in tests/FrameTimingTest.cpp drive what the real bodies must
// look like. The follow-up commit on this branch replaces both stubs.

double secondsBetween(std::uint64_t /*prevNs*/, std::uint64_t /*nowNs*/)
{
    return -1.0;
}
