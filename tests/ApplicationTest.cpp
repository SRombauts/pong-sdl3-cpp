#include "Application.h"

#include "ClockFake.h"

#include <doctest/doctest.h>

#include <memory>

// Proves ${PONG_SRC} is correctly compiled into the test binary by linking against Application's constructor and
// destructor in Application.cpp. If the shared source list drifts and Application.cpp stops being passed to the test
// target, this translation unit fails to link rather than silently skipping coverage.
//
// init() is deliberately skipped -- exercising the linker symbols is the whole point, and bringing up SDL video here
// would require a display and expand the test's failure modes beyond this guard's scope.
TEST_CASE("Application constructs and destructs without touching SDL")
{
    Application app("Pong test", 800, 600);
}

// End-to-end coverage of the IClock injection: drive tickFrameClock() through a ClockFake and verify the dt sequence.
TEST_CASE("Application::tickFrameClock drives dtSeconds via the injected IClock")
{
    // Application owns the ClockFake; keep a raw observer pointer here to drive it via advance() / setNow().
    auto clock = std::make_unique<ClockFake>(1'000'000'000ULL);
    ClockFake* fake = clock.get();
    Application app("Pong test", 800, 600, std::move(clock));

    // Skip init() (needs SDL video); prime m_lastTickNs the same way init() does: one discarded tickFrameClock().
    (void)app.tickFrameClock();

    CHECK(app.tickFrameClock() == doctest::Approx(0.0));

    fake->advance(1'000'000'000ULL);
    CHECK(app.tickFrameClock() == doctest::Approx(1.0));

    fake->advance(500'000'000ULL);
    CHECK(app.tickFrameClock() == doctest::Approx(0.5));

    // Re-check zero-advance after real advances: catches a clock accidentally wired to a constant.
    CHECK(app.tickFrameClock() == doctest::Approx(0.0));

    // Backward clock: secondsBetween() clamps to 0.0 so tickFrameClock can't feed a negative dt into update().
    fake->setNow(0ULL);
    CHECK(app.tickFrameClock() == doctest::Approx(0.0));
}
