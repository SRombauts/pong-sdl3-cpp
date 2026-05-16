#include "PlayfieldLayout.h"
#include "Playfield.h"

#include <doctest/doctest.h>

#include <SDL3/SDL.h>

#include <cmath>
#include <vector>

namespace
{
constexpr int kWidth = Playfield::kLogicalWidth;
constexpr int kHeight = Playfield::kLogicalHeight;
constexpr float kPaddleHW = Playfield::kPaddleHalfWidth;
constexpr float kPaddleHH = Playfield::kPaddleHalfHeight;
constexpr float kBallHS = Playfield::kBallHalfSize;
constexpr float kInset = Playfield::kWallInset;
constexpr float kDashW = Playfield::kCentreDashWidth;
constexpr float kDashH = Playfield::kCentreDashHeight;
constexpr float kDashGap = Playfield::kCentreDashGap;
constexpr int kDashN = Playfield::kCentreDashSegmentCount;
} // namespace

TEST_CASE("leftPaddle: centred vertically, inset from the left wall")
{
    const SDL_FRect r = PlayfieldLayout::leftPaddle(kWidth, kHeight, kPaddleHW, kPaddleHH, kInset);

    CHECK(r.w == doctest::Approx(2.0f * kPaddleHW));
    CHECK(r.h == doctest::Approx(2.0f * kPaddleHH));

    CHECK(r.x == doctest::Approx(kInset));
    CHECK((r.y + r.h * 0.5f) == doctest::Approx(static_cast<float>(kHeight) * 0.5f));
}

TEST_CASE("rightPaddle: centred vertically, inset from the right wall")
{
    const SDL_FRect r = PlayfieldLayout::rightPaddle(kWidth, kHeight, kPaddleHW, kPaddleHH, kInset);

    CHECK(r.w == doctest::Approx(2.0f * kPaddleHW));
    CHECK(r.h == doctest::Approx(2.0f * kPaddleHH));

    CHECK((r.x + r.w) == doctest::Approx(static_cast<float>(kWidth) - kInset));
    CHECK((r.y + r.h * 0.5f) == doctest::Approx(static_cast<float>(kHeight) * 0.5f));
}

TEST_CASE("paddles: left and right are mirrored across the vertical midline")
{
    const SDL_FRect left = PlayfieldLayout::leftPaddle(kWidth, kHeight, kPaddleHW, kPaddleHH, kInset);
    const SDL_FRect right = PlayfieldLayout::rightPaddle(kWidth, kHeight, kPaddleHW, kPaddleHH, kInset);

    const float leftMidX = left.x + left.w * 0.5f;
    const float rightMidX = right.x + right.w * 0.5f;
    CHECK((leftMidX + rightMidX) == doctest::Approx(static_cast<float>(kWidth)));

    CHECK(left.y == doctest::Approx(right.y));
    CHECK(left.w == doctest::Approx(right.w));
    CHECK(left.h == doctest::Approx(right.h));
}

TEST_CASE("paddles: oversized paddle still returns a usable rect (no NaN, no asserts)")
{
    // A paddle taller than the playfield mirrors the Paddle-controls milestone's clamping concern: the layout helper
    // must not assert or produce NaN dimensions, even though the resulting rect cannot be drawn entirely on-screen.
    // The Paddle-controls milestone is the right place to clamp; this layer just stays well-behaved.
    const float oversizedHalfHeight = static_cast<float>(kHeight);
    const SDL_FRect r = PlayfieldLayout::leftPaddle(kWidth, kHeight, kPaddleHW, oversizedHalfHeight, kInset);

    CHECK(r.w == doctest::Approx(2.0f * kPaddleHW));
    CHECK(r.h == doctest::Approx(2.0f * oversizedHalfHeight));
    CHECK_FALSE(std::isnan(r.x));
    CHECK_FALSE(std::isnan(r.y));

    // Still vertically centred (the midpoint is on the playfield centre line) -- only the rect extends past the top
    // and bottom walls.
    CHECK((r.y + r.h * 0.5f) == doctest::Approx(static_cast<float>(kHeight) * 0.5f));
}

TEST_CASE("ball: centred on both axes within sub-pixel tolerance")
{
    const SDL_FRect r = PlayfieldLayout::ball(kWidth, kHeight, kBallHS);

    CHECK(r.w == doctest::Approx(2.0f * kBallHS));
    CHECK(r.h == doctest::Approx(2.0f * kBallHS));
    CHECK((r.x + r.w * 0.5f) == doctest::Approx(static_cast<float>(kWidth) * 0.5f).epsilon(1e-4));
    CHECK((r.y + r.h * 0.5f) == doctest::Approx(static_cast<float>(kHeight) * 0.5f).epsilon(1e-4));
}

TEST_CASE("centreDashSegments: production tuning yields the expected count and total covered length")
{
    const std::vector<SDL_FRect> dashes =
        PlayfieldLayout::centreDashSegments(kWidth, kHeight, kDashN, kDashW, kDashH, kDashGap);

    REQUIRE(static_cast<int>(dashes.size()) == kDashN);

    float totalCovered = 0.0f;
    for (const SDL_FRect& d : dashes)
    {
        totalCovered += d.h;
    }
    CHECK(totalCovered == doctest::Approx(static_cast<float>(kDashN) * kDashH));
}

TEST_CASE("centreDashSegments: line is horizontally centred and gaps follow the documented convention")
{
    const std::vector<SDL_FRect> dashes =
        PlayfieldLayout::centreDashSegments(kWidth, kHeight, kDashN, kDashW, kDashH, kDashGap);
    REQUIRE(dashes.size() >= 2);

    const float playfieldMidX = static_cast<float>(kWidth) * 0.5f;
    for (const SDL_FRect& d : dashes)
    {
        CHECK(d.w == doctest::Approx(kDashW));
        CHECK(d.h == doctest::Approx(kDashH));
        CHECK((d.x + d.w * 0.5f) == doctest::Approx(playfieldMidX));
    }

    // Documented convention: dash k starts at y = gap/2 + k * (dashHeight + gap). Equivalently, the spacing between
    // dash k's bottom and dash k+1's top is exactly `gap`.
    CHECK(dashes.front().y == doctest::Approx(kDashGap * 0.5f));
    for (std::size_t i = 1; i < dashes.size(); ++i)
    {
        const float bottomOfPrev = dashes[i - 1].y + dashes[i - 1].h;
        const float topOfNext = dashes[i].y;
        CHECK((topOfNext - bottomOfPrev) == doctest::Approx(kDashGap));
    }
}

TEST_CASE("centreDashSegments: first and last segment positions stay inside the playfield")
{
    const std::vector<SDL_FRect> dashes =
        PlayfieldLayout::centreDashSegments(kWidth, kHeight, kDashN, kDashW, kDashH, kDashGap);
    REQUIRE(!dashes.empty());

    CHECK(dashes.front().y >= 0.0f);
    CHECK((dashes.back().y + dashes.back().h) <= static_cast<float>(kHeight));
}

TEST_CASE("centreDashSegments: defensive segmentCount == 1 returns a single centred dash")
{
    const std::vector<SDL_FRect> dashes =
        PlayfieldLayout::centreDashSegments(kWidth, kHeight, 1, kDashW, kDashH, kDashGap);

    REQUIRE(dashes.size() == 1);
    const SDL_FRect& d = dashes.front();
    CHECK(d.w == doctest::Approx(kDashW));
    CHECK(d.h == doctest::Approx(kDashH));
    CHECK((d.x + d.w * 0.5f) == doctest::Approx(static_cast<float>(kWidth) * 0.5f));
    CHECK((d.y + d.h * 0.5f) == doctest::Approx(static_cast<float>(kHeight) * 0.5f));
}

TEST_CASE("centreDashSegments: defensive segmentCount == 0 returns an empty range with no negative dimensions")
{
    const std::vector<SDL_FRect> dashes =
        PlayfieldLayout::centreDashSegments(kWidth, kHeight, 0, kDashW, kDashH, kDashGap);
    CHECK(dashes.empty());

    // Negative segmentCount is treated the same way (defensive against int-typed callers), no crash.
    const std::vector<SDL_FRect> alsoEmpty =
        PlayfieldLayout::centreDashSegments(kWidth, kHeight, -3, kDashW, kDashH, kDashGap);
    CHECK(alsoEmpty.empty());
}
