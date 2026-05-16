#include "PlayfieldRenderer.h"
#include "Playfield.h"
#include "PlayfieldLayout.h"

#include <doctest/doctest.h>

#include <SDL3/SDL.h>

#include <cstddef>
#include <vector>

namespace
{
constexpr int kWidth = Playfield::kLogicalWidth;
constexpr int kHeight = Playfield::kLogicalHeight;
constexpr float kDashW = Playfield::kCentreDashWidth;
constexpr float kDashH = Playfield::kCentreDashHeight;
constexpr float kDashGap = Playfield::kCentreDashGap;
constexpr int kDashN = Playfield::kCentreDashSegmentCount;
} // namespace

// PlayfieldRenderer delegates the actual layout to PlayfieldLayout::centreDashSegments at construction time. Verifying
// the cache content matches the helper output is what lets the helper's existing tests stay authoritative on the
// layout math; this case only guards the wiring.
TEST_CASE("PlayfieldRenderer caches the same dashes as PlayfieldLayout::centreDashSegments")
{
    const PlayfieldRenderer renderer(kWidth, kHeight, kDashN, kDashW, kDashH, kDashGap);
    const std::vector<SDL_FRect> expected =
        PlayfieldLayout::centreDashSegments(kWidth, kHeight, kDashN, kDashW, kDashH, kDashGap);

    const std::vector<SDL_FRect>& cached = renderer.centreDashes();

    REQUIRE(cached.size() == expected.size());
    for (std::size_t i = 0; i < cached.size(); ++i)
    {
        CHECK(cached[i].x == doctest::Approx(expected[i].x));
        CHECK(cached[i].y == doctest::Approx(expected[i].y));
        CHECK(cached[i].w == doctest::Approx(expected[i].w));
        CHECK(cached[i].h == doctest::Approx(expected[i].h));
    }
}

// The whole point of the class is that the layout math runs once. Comparing the underlying buffer address across two
// reads catches a refactor that would accidentally recompute (or re-allocate) the vector on every centreDashes() call.
TEST_CASE("PlayfieldRenderer::centreDashes returns a stable buffer across reads")
{
    const PlayfieldRenderer renderer(kWidth, kHeight, kDashN, kDashW, kDashH, kDashGap);

    const SDL_FRect* first = renderer.centreDashes().data();
    const SDL_FRect* second = renderer.centreDashes().data();
    CHECK(first == second);
}

// Defensive parity with PlayfieldLayout::centreDashSegments: a degenerate segment count produces an empty cache
// instead of asserting or allocating a sentinel.
TEST_CASE("PlayfieldRenderer: defensive segmentCount == 0 produces an empty cache")
{
    const PlayfieldRenderer renderer(kWidth, kHeight, 0, kDashW, kDashH, kDashGap);
    CHECK(renderer.centreDashes().empty());
}
