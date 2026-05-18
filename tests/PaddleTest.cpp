#include "Paddle.h"
#include "Playfield.h"

#include <doctest/doctest.h>

#include <SDL3/SDL.h>

namespace
{
constexpr float kWidth = Playfield::kLogicalWidth;
constexpr float kHeight = Playfield::kLogicalHeight;
constexpr float kPaddleHW = Playfield::kPaddleHalfWidth;
constexpr float kPaddleHH = Playfield::kPaddleHalfHeight;
constexpr float kInset = Playfield::kWallInset;
constexpr float kSpeed = Playfield::kPaddleSpeed;
} // namespace

// ---------------------------------------------------------------------------
// makePaddle
// ---------------------------------------------------------------------------

TEST_CASE("makePaddle(Left): outer edge sits on wallInset, vertically centered")
{
    const Paddle p = makePaddle(PaddleSide::Left, kWidth, kHeight, kPaddleHW, kPaddleHH, kInset, kSpeed);

    CHECK(p.halfWidth == doctest::Approx(kPaddleHW));
    CHECK(p.halfHeight == doctest::Approx(kPaddleHH));
    CHECK(p.speed == doctest::Approx(kSpeed));

    // Left edge of the rect = centerX - halfWidth must equal `wallInset`.
    CHECK((p.centerX - p.halfWidth) == doctest::Approx(kInset));
    CHECK(p.centerY == doctest::Approx(kHeight * 0.5f));
}

TEST_CASE("makePaddle(Right): outer edge sits on playfieldWidth - wallInset, vertically centered")
{
    const Paddle p = makePaddle(PaddleSide::Right, kWidth, kHeight, kPaddleHW, kPaddleHH, kInset, kSpeed);

    CHECK(p.halfWidth == doctest::Approx(kPaddleHW));
    CHECK(p.halfHeight == doctest::Approx(kPaddleHH));
    CHECK(p.speed == doctest::Approx(kSpeed));

    // Right edge of the rect = centerX + halfWidth must equal `playfieldWidth - wallInset`.
    CHECK((p.centerX + p.halfWidth) == doctest::Approx(kWidth - kInset));
    CHECK(p.centerY == doctest::Approx(kHeight * 0.5f));
}

TEST_CASE("makePaddle: Left and Right are mirrored across the vertical midline")
{
    const Paddle left = makePaddle(PaddleSide::Left, kWidth, kHeight, kPaddleHW, kPaddleHH, kInset, kSpeed);
    const Paddle right = makePaddle(PaddleSide::Right, kWidth, kHeight, kPaddleHW, kPaddleHH, kInset, kSpeed);

    CHECK((left.centerX + right.centerX) == doctest::Approx(kWidth));
    CHECK(left.centerY == doctest::Approx(right.centerY));
    CHECK(left.halfWidth == doctest::Approx(right.halfWidth));
    CHECK(left.halfHeight == doctest::Approx(right.halfHeight));
    CHECK(left.speed == doctest::Approx(right.speed));
}

// ---------------------------------------------------------------------------
// toFRect
// ---------------------------------------------------------------------------

TEST_CASE("toFRect: translates center + half-extents to the top-left rect SDL_RenderFillRect expects")
{
    const Paddle p{100.0f, 200.0f, 4.0f, 16.0f, 0.0f};
    const SDL_FRect r = toFRect(p);

    CHECK(r.x == doctest::Approx(96.0f));  // centerX - halfWidth
    CHECK(r.y == doctest::Approx(184.0f)); // centerY - halfHeight
    CHECK(r.w == doctest::Approx(8.0f));   // halfWidth  * 2
    CHECK(r.h == doctest::Approx(32.0f));  // halfHeight * 2

    // Round-trip: the rect's geometric center must equal the paddle's stored center.
    CHECK((r.x + r.w * 0.5f) == doctest::Approx(p.centerX));
    CHECK((r.y + r.h * 0.5f) == doctest::Approx(p.centerY));
}

TEST_CASE("toFRect: a default-constructed paddle yields a zero-extent rect at the origin (no NaN)")
{
    // Pins the default-member-initializer contract: every field defaults to 0.0f, so toFRect is well-defined even
    // before the Application constructor seeds the real placement.
    const Paddle p{};
    const SDL_FRect r = toFRect(p);
    CHECK(r.x == doctest::Approx(0.0f));
    CHECK(r.y == doctest::Approx(0.0f));
    CHECK(r.w == doctest::Approx(0.0f));
    CHECK(r.h == doctest::Approx(0.0f));
}
