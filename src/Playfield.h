#pragma once

// Logical playfield resolution and coordinate convention.
//
// All gameplay code is written in *logical* pixels, decoupled from the actual window size. The renderer is configured
// with SDL_SetRenderLogicalPresentation in Application::init(), which scales the logical playfield to whatever the
// user's window currently is and preserves the aspect ratio with letterbox bars.
//
// Coordinate convention:
//   - Origin at the top-left of the playfield.
//   - +X points to the right, +Y points downward (matches SDL's screen-space convention, so SDL_FRect values can be
//     filled in directly from gameplay coordinates without flipping).
//   - All gameplay positions and sizes are expressed in logical pixels; the renderer is responsible for mapping them to
//     physical pixels.
//
// 800x600 is picked as the starting logical resolution to match the current default window size; the values are
// independent and the window can be resized freely at runtime without affecting gameplay coordinates.
namespace Playfield
{
constexpr int kLogicalWidth = 800;
constexpr int kLogicalHeight = 600;

// Sizes are half-extents because the layout helpers and collision math operate on centers; storing the half-extent
// avoids "/2" in call sites. Values give classic arcade-Pong proportions on the 800x600 playfield.
constexpr float kPaddleHalfWidth = 6.0f;
constexpr float kPaddleHalfHeight = 40.0f;
constexpr float kBallHalfSize = 6.0f;
constexpr float kWallInset = 24.0f;

// Dashed center line. With kCenterDashSegmentCount = 20 and pitch (kCenterDashHeight + kCenterDashGap) = 30, the 20
// bands exactly tile the 600-pixel playfield, leaving symmetric gap/2 = 6 px clearances at top and bottom.
constexpr float kCenterDashWidth = 6.0f;
constexpr float kCenterDashHeight = 18.0f;
constexpr float kCenterDashGap = 12.0f;
constexpr int kCenterDashSegmentCount = 20;

// Placeholder score readout, drawn by Application::render via TextRenderer. The score string ("0 0" today, real values
// once the Scoring-and-match-flow milestone wires them in) is centered horizontally on the playfield's vertical midline
// and pinned `kScoreTopY` logical pixels below the top edge. kScorePixelSize scales each font pixel of the 5x7 bitmap
// glyph: at 4.0 logical pixels per font pixel, each digit occupies 20x28 logical pixels and the literal space character
// in "0 0" yields a 20-pixel gap with kScoreGlyphSpacing of breathing room on either side. These three constants are
// tuning knobs (font size, gap, top offset) and are expected to be revisited by the scoring-visuals milestone.
constexpr float kScoreTopY = 24.0f;
constexpr float kScorePixelSize = 4.0f;
constexpr float kScoreGlyphSpacing = 4.0f;
} // namespace Playfield
