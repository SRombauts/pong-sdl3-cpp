#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <cstdint>
#include <string_view>
#include <vector>

// Text rendering for the playfield score and the upcoming menu strings (PONG, PAUSE, GAME OVER, 1 PLAYER, 2 PLAYER,
// RESTART, QUIT).
//
// Chosen approach: a small hand-drawn 5x7 bitmap font. Each glyph is a constexpr table of seven uint8_t rows, with
// bit (kGlyphPixelCols - 1 - col) representing the pixel at column `col` (bit 4 = leftmost column, bit 0 = rightmost).
// `textGlyphRects` returns one SDL_FRect per on-pixel scaled by `pixelSize`, which the thin SDL boundary draws with
// SDL_RenderFillRect.
//
// Why this option, rejecting the two alternatives evaluated in docs/ISSUES.md:
//   - Hand-drawn 7-segment digits would have shipped a digits-only renderer that the Screens-and-menus milestone would
//     have had to throw away (menus need full words). The constant we save by going simpler now is paid back twice
//     when menus land.
//   - SDL3_ttf with a bundled TTF font would add a third-party dependency, CI dev-headers on Linux, an asset-path
//     resolution problem on three OSes, and a font-license note -- all for a fixed, small set of arcade strings.
//
// The glyph table covers `0..9` plus the menu alphabet (A, E, G, I, L, M, N, O, P, Q, R, S, T, U, V, Y) and `space`.
// It is intentionally easy to extend: add another entry to the kGlyphs table in TextRenderer.cpp; no signature changes.
namespace TextRenderer
{

// 5 columns x 7 rows per glyph. Both halves of the project (layout math and the SDL renderer) read these so that
// changing the grid dimension is a single-point edit.
constexpr int kGlyphPixelCols = 5;
constexpr int kGlyphPixelRows = 7;

// Bitmap for the given character. Each returned uint8_t encodes one row: bit (kGlyphPixelCols - 1 - col) is the pixel
// at column `col` (so bit 4 is the leftmost column, bit 0 the rightmost, and the upper three bits are always zero).
// Unsupported characters return an all-zero pattern; they still consume one glyph slot in textGlyphRects so the
// caller-visible layout stays consistent (useful for `space` and as a defensive fallback).
[[nodiscard]] std::array<std::uint8_t, kGlyphPixelRows> glyphPattern(char c);

// One SDL_FRect per on-pixel of `text` rendered at (`originX`, `originY`) with each font pixel scaled to a
// `pixelSize` x `pixelSize` square and `glyphSpacing` logical pixels between adjacent glyphs.
//
// Layout convention:
//   - Glyph i sits at originX + i * (kGlyphPixelCols * pixelSize + glyphSpacing), originY. Total text width is
//     therefore N * (kGlyphPixelCols * pixelSize + glyphSpacing) - glyphSpacing (no trailing spacing).
//   - Inside a glyph, row 0 is the top row and column 0 the leftmost; rects grow rightward and downward to match
//     SDL's screen-space convention (see Playfield.h).
//   - A `space` (or any unsupported character) contributes zero rects but still advances the layout by one glyph.
//
// Defensive behavior: an empty `text` or a non-positive `pixelSize` returns an empty vector instead of asserting,
// matching the centerDashSegments contract in PlayfieldLayout.
[[nodiscard]] std::vector<SDL_FRect> textGlyphRects(std::string_view text,
                                                    float originX,
                                                    float originY,
                                                    float pixelSize,
                                                    float glyphSpacing);

} // namespace TextRenderer
