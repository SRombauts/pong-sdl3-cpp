#include "TextRenderer.h"

#include <doctest/doctest.h>

#include <SDL3/SDL.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace
{
using Pattern = std::array<std::uint8_t, TextRenderer::kGlyphPixelRows>;

constexpr Pattern kZero = {0b1111, 0b1001, 0b1001, 0b1001, 0b1001, 0b1001, 0b1001, 0b1111};
constexpr Pattern kNine = {0b1111, 0b1001, 0b1001, 0b1111, 0b0001, 0b0001, 0b0001, 0b0001};
constexpr Pattern kP = {0b1111, 0b1001, 0b1110, 0b1000, 0b1000, 0b1000, 0b1000, 0b1000};
constexpr Pattern kA = {0b0110, 0b1001, 0b1001, 0b1111, 0b1001, 0b1001, 0b1001, 0b1001};
constexpr Pattern kBlank = {0, 0, 0, 0, 0, 0, 0, 0};

// Count the on-pixels in a 5-column bitmap row -- a small popcount restricted to the 5 used bits.
int onPixelsInRow(std::uint8_t row)
{
    int count = 0;
    for (int col = 0; col < TextRenderer::kGlyphPixelCols; ++col)
    {
        if ((row & (1u << static_cast<unsigned int>(col))) != 0u)
        {
            ++count;
        }
    }
    return count;
}

int onPixelsIn(const Pattern& pattern)
{
    int total = 0;
    for (const std::uint8_t row : pattern)
    {
        total += onPixelsInRow(row);
    }
    return total;
}
} // namespace

TEST_CASE("glyphPattern: representative digits and letters match the documented bitmap")
{
    SUBCASE("0 -- plain rectangular outline, no diagonal corner bar (matches Atari Pong digit ROM)")
    {
        CHECK(TextRenderer::glyphPattern('0') == kZero);
    }
    SUBCASE("9 -- top bar, top loop, middle bar, right descending vertical (no bottom horizontal cap)")
    {
        // The Atari Pong digit ROM omits the bottom horizontal on '9'; the bottom-right vertical just ends at row 7
        // with no cap. Mirror image of the '6' shape (which omits the top horizontal).
        CHECK(TextRenderer::glyphPattern('9') == kNine);
    }
    SUBCASE("P -- shortened top loop (waist at row 2) above a long descending stem")
    {
        CHECK(TextRenderer::glyphPattern('P') == kP);
    }
    SUBCASE("A -- rounded apex, crossbar at row 3 with four leg rows below")
    {
        CHECK(TextRenderer::glyphPattern('A') == kA);
    }
    SUBCASE("space -- all rows zero (blank but layout-advancing)")
    {
        CHECK(TextRenderer::glyphPattern(' ') == kBlank);
    }
    SUBCASE("unsupported character -- defensive blank pattern (no crash)")
    {
        CHECK(TextRenderer::glyphPattern('@') == kBlank);
        CHECK(TextRenderer::glyphPattern('\0') == kBlank);
    }
}

TEST_CASE("glyphPattern: all 4-bit rows use only the documented column bits (upper 4 bits always zero)")
{
    for (char c : std::string_view{"0123456789AEGILMNOPQRSTUVY "})
    {
        const Pattern p = TextRenderer::glyphPattern(c);
        for (const std::uint8_t row : p)
        {
            CHECK((row & 0b11110000u) == 0u);
        }
    }
}

TEST_CASE("textGlyphRects: single-glyph emission yields one rect per on-pixel at the documented coordinates")
{
    constexpr float kOriginX = 100.0f;
    constexpr float kOriginY = 50.0f;
    constexpr float kPixelSize = 2.0f;

    const std::vector<SDL_FRect> rects =
        TextRenderer::textGlyphRects("P", kOriginX, kOriginY, kPixelSize, /*glyphSpacing=*/0.0f);

    REQUIRE(static_cast<int>(rects.size()) == onPixelsIn(kP));

    for (const SDL_FRect& r : rects)
    {
        CHECK(r.w == doctest::Approx(kPixelSize));
        CHECK(r.h == doctest::Approx(kPixelSize));
        // Each rect must align to the (originX, originY) + (col, row) * pixelSize grid.
        const float colIndex = (r.x - kOriginX) / kPixelSize;
        const float rowIndex = (r.y - kOriginY) / kPixelSize;
        CHECK(colIndex == doctest::Approx(std::floor(colIndex)));
        CHECK(rowIndex == doctest::Approx(std::floor(rowIndex)));
        CHECK(colIndex >= 0.0f);
        CHECK(colIndex < static_cast<float>(TextRenderer::kGlyphPixelCols));
        CHECK(rowIndex >= 0.0f);
        CHECK(rowIndex < static_cast<float>(TextRenderer::kGlyphPixelRows));
    }
}

TEST_CASE("textGlyphRects: total layout width matches the documented N*(glyphWidth + spacing) - spacing formula")
{
    constexpr float kPixelSize = 3.0f;
    constexpr float kSpacing = 4.0f;
    const std::string_view text = "PAUSE";

    const std::vector<SDL_FRect> rects = TextRenderer::textGlyphRects(text, 0.0f, 0.0f, kPixelSize, kSpacing);
    REQUIRE_FALSE(rects.empty());

    float minX = rects.front().x;
    float maxRight = rects.front().x + rects.front().w;
    for (const SDL_FRect& r : rects)
    {
        if (r.x < minX)
        {
            minX = r.x;
        }
        if (r.x + r.w > maxRight)
        {
            maxRight = r.x + r.w;
        }
    }

    const float glyphWidth = static_cast<float>(TextRenderer::kGlyphPixelCols) * kPixelSize;
    const float expectedWidth = static_cast<float>(text.size()) * (glyphWidth + kSpacing) - kSpacing;
    // The bounding box of the *on*-pixels can be narrower than the full layout width when the last glyph's rightmost
    // column is empty, so we compare to the layout's last-glyph right edge instead of expectedWidth directly.
    const float lastGlyphLeft = static_cast<float>(text.size() - 1) * (glyphWidth + kSpacing);
    const float lastGlyphRight = lastGlyphLeft + glyphWidth;

    CHECK(minX >= 0.0f);
    CHECK(maxRight <= lastGlyphRight + 0.001f);
    CHECK(lastGlyphRight == doctest::Approx(expectedWidth));
}

TEST_CASE("textGlyphRects: shifting the origin shifts every output rect by the same delta")
{
    constexpr float kPixelSize = 2.0f;
    constexpr float kSpacing = 1.0f;
    constexpr float kDx = 17.5f;
    constexpr float kDy = -3.25f;
    const std::string_view text = "GAME OVER";

    const std::vector<SDL_FRect> baseline = TextRenderer::textGlyphRects(text, 0.0f, 0.0f, kPixelSize, kSpacing);
    const std::vector<SDL_FRect> shifted = TextRenderer::textGlyphRects(text, kDx, kDy, kPixelSize, kSpacing);

    REQUIRE(baseline.size() == shifted.size());
    for (std::size_t i = 0; i < baseline.size(); ++i)
    {
        CHECK(shifted[i].x == doctest::Approx(baseline[i].x + kDx));
        CHECK(shifted[i].y == doctest::Approx(baseline[i].y + kDy));
        CHECK(shifted[i].w == doctest::Approx(baseline[i].w));
        CHECK(shifted[i].h == doctest::Approx(baseline[i].h));
    }
}

TEST_CASE("textGlyphRects: space character contributes zero rects but still advances the layout")
{
    constexpr float kPixelSize = 2.0f;
    constexpr float kSpacing = 1.0f;
    const float glyphPitch = static_cast<float>(TextRenderer::kGlyphPixelCols) * kPixelSize + kSpacing;

    const std::vector<SDL_FRect> letters = TextRenderer::textGlyphRects("PP", 0.0f, 0.0f, kPixelSize, kSpacing);
    const std::vector<SDL_FRect> withSpace = TextRenderer::textGlyphRects("P P", 0.0f, 0.0f, kPixelSize, kSpacing);

    REQUIRE_FALSE(letters.empty());
    // Same total on-pixels (the space has none), but the second 'P' is shifted right by one extra glyph pitch.
    REQUIRE(letters.size() == withSpace.size());

    const std::size_t half = letters.size() / 2;
    for (std::size_t i = 0; i < half; ++i)
    {
        CHECK(withSpace[i].x == doctest::Approx(letters[i].x));
    }
    for (std::size_t i = half; i < letters.size(); ++i)
    {
        CHECK(withSpace[i].x == doctest::Approx(letters[i].x + glyphPitch));
    }
}

TEST_CASE("textGlyphRects: defensive empty input and non-positive pixelSize return an empty vector")
{
    CHECK(TextRenderer::textGlyphRects("", 0.0f, 0.0f, 1.0f, 0.0f).empty());
    CHECK(TextRenderer::textGlyphRects("0 0", 0.0f, 0.0f, 0.0f, 0.0f).empty());
    CHECK(TextRenderer::textGlyphRects("0 0", 0.0f, 0.0f, -1.0f, 0.0f).empty());
}

TEST_CASE("textWidth: matches the N*(glyphWidth + spacing) - spacing layout formula")
{
    constexpr float kPixelSize = 3.0f;
    constexpr float kSpacing = 4.0f;
    const float glyphWidth = static_cast<float>(TextRenderer::kGlyphPixelCols) * kPixelSize;

    CHECK(TextRenderer::textWidth("", kPixelSize, kSpacing) == doctest::Approx(0.0f));
    CHECK(TextRenderer::textWidth("0", kPixelSize, kSpacing) == doctest::Approx(glyphWidth));
    // Two-digit scores like "11" need exactly one inter-glyph spacing slot, never two trailing ones.
    CHECK(TextRenderer::textWidth("11", kPixelSize, kSpacing) == doctest::Approx(2.0f * glyphWidth + kSpacing));
    CHECK(TextRenderer::textWidth("PAUSE", kPixelSize, kSpacing) ==
          doctest::Approx(5.0f * (glyphWidth + kSpacing) - kSpacing));
}

TEST_CASE("textWidth: defensive empty input or non-positive pixelSize returns zero")
{
    CHECK(TextRenderer::textWidth("", 12.0f, 4.0f) == doctest::Approx(0.0f));
    CHECK(TextRenderer::textWidth("11", 0.0f, 4.0f) == doctest::Approx(0.0f));
    CHECK(TextRenderer::textWidth("11", -1.0f, 4.0f) == doctest::Approx(0.0f));
}
