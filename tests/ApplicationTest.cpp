#include "Application.h"

#include <doctest/doctest.h>

// This test exists primarily to prove that ${PONG_SRC} is correctly compiled
// into the test binary: it links against Application's constructor and
// destructor, which live in Application.cpp. If the shared source list ever
// drifts and Application.cpp stops being passed to the test target, this
// translation unit fails to link rather than silently skipping coverage.
//
// Note: init() is deliberately not called -- exercising the linker symbols is
// the whole point, and bringing up real SDL video here would require a display
// and would expand the test's failure modes well beyond what this guard is
// meant to cover.
TEST_CASE("Application constructs and destructs without touching SDL")
{
    Application app("Pong test", 800, 600);
}
