#include "TextRenderer.h"

#include <cstddef>

namespace TextRenderer
{

namespace
{

// One glyph = one ASCII character paired with its 7-row bitmap. Stored in an unordered table looked up linearly by
// glyphPattern(); the table is small (a few dozen entries) and the lookups happen at most once per drawn character,
// so the linear scan is the simplest correct option. A sorted table + std::lower_bound is an obvious future
// optimization; the data structure here was picked for legibility, not throughput.
//
// Bit convention (must match the documentation in TextRenderer.h): bit (kGlyphPixelCols - 1 - col) is column `col`,
// i.e. bit 4 is the leftmost column. Each binary literal therefore reads visually left-to-right like the rendered
// glyph -- `0b11110` is `####.` (cols 0..3 on, col 4 off).
struct Glyph
{
    char character;
    std::array<std::uint8_t, kGlyphPixelRows> rows;
};

// clang-format off
constexpr std::array<Glyph, 27> kGlyphs = {{
    {'0', {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110}},
    {'1', {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}},
    {'2', {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111}},
    {'3', {0b01110, 0b10001, 0b00001, 0b00110, 0b00001, 0b10001, 0b01110}},
    {'4', {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010}},
    {'5', {0b11111, 0b10000, 0b11110, 0b00001, 0b00001, 0b10001, 0b01110}},
    {'6', {0b00110, 0b01000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110}},
    {'7', {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000}},
    {'8', {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110}},
    {'9', {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00010, 0b01100}},
    {'A', {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001}},
    {'E', {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111}},
    {'G', {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01110}},
    {'I', {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}},
    {'L', {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111}},
    {'M', {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001}},
    {'N', {0b10001, 0b11001, 0b11001, 0b10101, 0b10011, 0b10011, 0b10001}},
    {'O', {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},
    {'P', {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000}},
    {'Q', {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101}},
    {'R', {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001}},
    {'S', {0b01110, 0b10001, 0b10000, 0b01110, 0b00001, 0b10001, 0b01110}},
    {'T', {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100}},
    {'U', {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110}},
    {'V', {0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b01010, 0b00100}},
    {'Y', {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100}},
    {' ', {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}},
}};
// clang-format on

constexpr std::array<std::uint8_t, kGlyphPixelRows> kBlankGlyph = {0, 0, 0, 0, 0, 0, 0};

} // namespace

std::array<std::uint8_t, kGlyphPixelRows> glyphPattern(char c)
{
    for (const Glyph& g : kGlyphs)
    {
        if (g.character == c)
        {
            return g.rows;
        }
    }
    return kBlankGlyph;
}

std::vector<SDL_FRect> textGlyphRects(std::string_view text,
                                      float originX,
                                      float originY,
                                      float pixelSize,
                                      float glyphSpacing)
{
    std::vector<SDL_FRect> rects;
    if (text.empty() || pixelSize <= 0.0f)
    {
        return rects;
    }

    const float glyphWidth = static_cast<float>(kGlyphPixelCols) * pixelSize;
    const float glyphPitch = glyphWidth + glyphSpacing;

    // Upper-bound the rect count so the typical case avoids reallocations. Worst case is every pixel of every glyph
    // turned on (35 per glyph); the reserve cost is one allocation per call regardless of glyph density.
    rects.reserve(text.size() * static_cast<std::size_t>(kGlyphPixelCols * kGlyphPixelRows));

    for (std::size_t i = 0; i < text.size(); ++i)
    {
        const std::array<std::uint8_t, kGlyphPixelRows> pattern = glyphPattern(text[i]);
        const float glyphLeft = originX + static_cast<float>(i) * glyphPitch;
        for (int row = 0; row < kGlyphPixelRows; ++row)
        {
            const std::uint8_t rowBits = pattern[row];
            if (rowBits == 0)
            {
                continue;
            }
            for (int col = 0; col < kGlyphPixelCols; ++col)
            {
                const unsigned int mask = 1u << static_cast<unsigned int>(kGlyphPixelCols - 1 - col);
                if ((rowBits & mask) != 0u)
                {
                    SDL_FRect rect{};
                    rect.x = glyphLeft + static_cast<float>(col) * pixelSize;
                    rect.y = originY + static_cast<float>(row) * pixelSize;
                    rect.w = pixelSize;
                    rect.h = pixelSize;
                    rects.push_back(rect);
                }
            }
        }
    }
    return rects;
}

} // namespace TextRenderer
