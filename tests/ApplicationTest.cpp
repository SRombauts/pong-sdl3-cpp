#include "Application.h"

#include "FakeClock.h"

#include <doctest/doctest.h>

#include <memory>

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

// End-to-end coverage of the IClock injection: drive tickFrameClock()
// through a FakeClock and verify the dt sequence.
TEST_CASE("Application::tickFrameClock drives dtSeconds via the injected IClock")
{
    // Application owns the FakeClock through m_clock; keep a raw
    // observer pointer here to drive it via advance() / setNow().
    auto clock = std::make_unique<FakeClock>(1'000'000'000ULL);
    FakeClock* fake = clock.get();
    Application app("Pong test", 800, 600, std::move(clock));

    // Skip init() (would need SDL video) and prime m_lastTickNs the
    // same way init() does: one discarded tickFrameClock().
    (void)app.tickFrameClock();

    CHECK(app.tickFrameClock() == doctest::Approx(0.0));

    fake->advance(1'000'000'000ULL);
    CHECK(app.tickFrameClock() == doctest::Approx(1.0));

    fake->advance(500'000'000ULL);
    CHECK(app.tickFrameClock() == doctest::Approx(0.5));

    // Re-check zero-advance after real advances: catches a clock
    // accidentally wired to a constant.
    CHECK(app.tickFrameClock() == doctest::Approx(0.0));

    // Backward clock: must not feed a negative dt into update().
    // secondsBetween() clamps to 0.0; tickFrameClock relies on that.
    fake->setNow(0ULL);
    CHECK(app.tickFrameClock() == doctest::Approx(0.0));
}
