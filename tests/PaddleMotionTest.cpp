#include "PaddleMotion.h"

#include <doctest/doctest.h>

#include <cmath>

namespace
{
constexpr float kPlayfieldHeight = 600.0f;
constexpr float kHalfHeight = 16.0f;
constexpr float kSpeed = 300.0f;
} // namespace

// ---------------------------------------------------------------------------
// clampPaddleCenterY -- also the absolute-position controller's per-tick step
// ---------------------------------------------------------------------------

TEST_CASE("clampPaddleCenterY: top edge snaps to halfHeight")
{
    CHECK(clampPaddleCenterY(0.0f, kHalfHeight, kPlayfieldHeight) == doctest::Approx(kHalfHeight));
    CHECK(clampPaddleCenterY(-100.0f, kHalfHeight, kPlayfieldHeight) == doctest::Approx(kHalfHeight));
    CHECK(clampPaddleCenterY(kHalfHeight - 0.5f, kHalfHeight, kPlayfieldHeight) == doctest::Approx(kHalfHeight));
}

TEST_CASE("clampPaddleCenterY: bottom edge snaps to playfieldHeight - halfHeight")
{
    const float bottomBound = kPlayfieldHeight - kHalfHeight;
    CHECK(clampPaddleCenterY(kPlayfieldHeight, kHalfHeight, kPlayfieldHeight) == doctest::Approx(bottomBound));
    CHECK(clampPaddleCenterY(10'000.0f, kHalfHeight, kPlayfieldHeight) == doctest::Approx(bottomBound));
    CHECK(clampPaddleCenterY(bottomBound + 0.5f, kHalfHeight, kPlayfieldHeight) == doctest::Approx(bottomBound));
}

TEST_CASE("clampPaddleCenterY: interior values pass through unchanged")
{
    CHECK(clampPaddleCenterY(300.0f, kHalfHeight, kPlayfieldHeight) == doctest::Approx(300.0f));
    CHECK(clampPaddleCenterY(kHalfHeight, kHalfHeight, kPlayfieldHeight) == doctest::Approx(kHalfHeight));
    const float bottomBound = kPlayfieldHeight - kHalfHeight;
    CHECK(clampPaddleCenterY(bottomBound, kHalfHeight, kPlayfieldHeight) == doctest::Approx(bottomBound));
}

TEST_CASE("clampPaddleCenterY: oversized paddle returns the playfield midpoint")
{
    // halfHeight * 2 > playfieldHeight, so the legal-position interval is empty. The helper must return the
    // playfield's vertical midpoint rather than an arbitrary edge, and must never produce NaN.
    const float oversized = kPlayfieldHeight;
    const float midpoint = kPlayfieldHeight * 0.5f;

    CHECK(clampPaddleCenterY(0.0f, oversized, kPlayfieldHeight) == doctest::Approx(midpoint));
    CHECK(clampPaddleCenterY(midpoint, oversized, kPlayfieldHeight) == doctest::Approx(midpoint));
    CHECK(clampPaddleCenterY(10'000.0f, oversized, kPlayfieldHeight) == doctest::Approx(midpoint));
    CHECK_FALSE(std::isnan(clampPaddleCenterY(0.0f, oversized, kPlayfieldHeight)));
}

TEST_CASE("clampPaddleCenterY: mouse-style absolute target -- a far target snaps onto the wall, not partway")
{
    // The historical reference (Atari arcade Pong 1972, Atari 2600 paddle controllers) is an absolute-position
    // model with no rate limit: the paddle goes wherever the knob points each frame. clampPaddleCenterY is the
    // per-tick step for that family of controllers. This test pins the "no rate limit" property: a mouse Y that
    // sits 100 px outside the playfield must place the paddle exactly on the matching wall in a single call, not
    // somewhere partway -- the cap belongs to stepPaddleCenterY, never to this helper.
    const float bottomBound = kPlayfieldHeight - kHalfHeight;
    CHECK(clampPaddleCenterY(kPlayfieldHeight + 100.0f, kHalfHeight, kPlayfieldHeight) == doctest::Approx(bottomBound));
    CHECK(clampPaddleCenterY(-100.0f, kHalfHeight, kPlayfieldHeight) == doctest::Approx(kHalfHeight));
}

// ---------------------------------------------------------------------------
// stepPaddleCenterY -- velocity-style controller (keyboard, stick-as-velocity)
// ---------------------------------------------------------------------------

TEST_CASE("stepPaddleCenterY: axis = +1 moves down at speed * dt")
{
    const float start = 300.0f;
    const double dt = 1.0 / 60.0;
    const float next = stepPaddleCenterY(1.0f, start, kHalfHeight, kSpeed, kPlayfieldHeight, dt);
    CHECK((next - start) == doctest::Approx(kSpeed * static_cast<float>(dt)));
}

TEST_CASE("stepPaddleCenterY: axis = -1 moves up at speed * dt")
{
    const float start = 300.0f;
    const double dt = 1.0 / 60.0;
    const float next = stepPaddleCenterY(-1.0f, start, kHalfHeight, kSpeed, kPlayfieldHeight, dt);
    CHECK((start - next) == doctest::Approx(kSpeed * static_cast<float>(dt)));
}

TEST_CASE("stepPaddleCenterY: axis = 0 leaves the paddle stationary")
{
    const float start = 300.0f;
    const float next = stepPaddleCenterY(0.0f, start, kHalfHeight, kSpeed, kPlayfieldHeight, 1.0 / 60.0);
    CHECK(next == doctest::Approx(start));
}

TEST_CASE("stepPaddleCenterY: a single huge-dt step cannot escape the playfield bounds")
{
    // A pathological dt (e.g. the app was paused under a debugger) must still clamp the paddle inside the playfield;
    // the renderer should never draw a paddle past the wall.
    const float bottomBound = kPlayfieldHeight - kHalfHeight;
    const float next = stepPaddleCenterY(1.0f, 300.0f, kHalfHeight, kSpeed, kPlayfieldHeight, 60.0);
    CHECK(next == doctest::Approx(bottomBound));

    const float prev = stepPaddleCenterY(-1.0f, 300.0f, kHalfHeight, kSpeed, kPlayfieldHeight, 60.0);
    CHECK(prev == doctest::Approx(kHalfHeight));
}

TEST_CASE("stepPaddleCenterY: an out-of-range axis is still capped by speed * dt")
{
    // Defensive: a buggy controller returning axis = 2.0f must not move the paddle faster than the speed cap.
    const float start = 300.0f;
    const double dt = 1.0 / 60.0;
    const float next = stepPaddleCenterY(2.0f, start, kHalfHeight, kSpeed, kPlayfieldHeight, dt);
    CHECK((next - start) == doctest::Approx(kSpeed * static_cast<float>(dt)));
}

TEST_CASE("stepPaddleCenterY: negative speed or dt collapses to a no-op (no UB from std::clamp lo > hi)")
{
    // Both inputs are documented preconditions, but the helper must collapse them to a zero step rather than feed
    // `std::clamp(x, lo, hi)` with `lo > hi` (UB per [alg.clamp]). The paddle's position must not change in either
    // case, regardless of the axis intent.
    const float start = 300.0f;
    const double dt = 1.0 / 60.0;

    SUBCASE("negative dt: axis request is dropped")
    {
        const float next = stepPaddleCenterY(1.0f, start, kHalfHeight, kSpeed, kPlayfieldHeight, -dt);
        CHECK(next == doctest::Approx(start));
    }

    SUBCASE("negative speed: axis request is dropped")
    {
        const float next = stepPaddleCenterY(1.0f, start, kHalfHeight, -kSpeed, kPlayfieldHeight, dt);
        CHECK(next == doctest::Approx(start));
    }
}

TEST_CASE("stepPaddleCenterY: one big dt step equals N small dt steps of equal sum (framerate independence)")
{
    const float start = 100.0f;
    const double totalSeconds = 0.1;
    const int steps = 10;
    const double smallDt = totalSeconds / steps;

    const float bigStep = stepPaddleCenterY(1.0f, start, kHalfHeight, kSpeed, kPlayfieldHeight, totalSeconds);

    float manyStepsCenterY = start;
    for (int i = 0; i < steps; ++i)
    {
        manyStepsCenterY = stepPaddleCenterY(1.0f, manyStepsCenterY, kHalfHeight, kSpeed, kPlayfieldHeight, smallDt);
    }

    // Float accumulation across 10 steps introduces sub-pixel drift; doctest::Approx's default tolerance is too tight
    // for the comparison, so widen it slightly.
    CHECK(bigStep == doctest::Approx(manyStepsCenterY).epsilon(1e-4));
}
