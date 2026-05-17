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
// avoids "/2" in call sites. The paddle dimensions (8x32 logical pixels) target the thin, short-bar look of arcade
// Pong on the 800x600 playfield -- ~1% of width, ~5% of height -- so the playable area dominates the chrome.
constexpr float kPaddleHalfWidth = 4.0f;
constexpr float kPaddleHalfHeight = 16.0f;
constexpr float kBallHalfSize = 6.0f;
constexpr float kWallInset = 24.0f;

// Dashed center line. With kCenterDashSegmentCount = 20 and pitch (kCenterDashHeight + kCenterDashGap) = 30, the 20
// bands exactly tile the 600-pixel playfield, leaving symmetric gap/2 = 6 px clearances at top and bottom.
constexpr float kCenterDashWidth = 6.0f;
constexpr float kCenterDashHeight = 18.0f;
constexpr float kCenterDashGap = 12.0f;
constexpr int kCenterDashSegmentCount = 20;

// Placeholder per-player score readouts, drawn by Application::render via TextRenderer::drawTextCentered. Each player's
// score is rendered as its own string and centered horizontally on the midpoint of its half of the playfield (left
// score on x = kLogicalWidth/4, right score on x = 3*kLogicalWidth/4); this matches arcade-Pong reference frames where
// the two scores sit well away from the dashed center line rather than hugging it. Both scores share the same
// kScoreTopY top offset and kScorePixelSize font scale: at 10.0 logical pixels per font pixel, each digit of the 4x8
// bitmap font occupies 40x80 logical pixels. kScoreGlyphSpacing applies between adjacent digits once a score reaches
// two figures (drawTextCentered re-centers automatically when the string grows). All five constants are tuning knobs,
// expected to be revisited by the scoring-visuals milestone once real values land.
constexpr float kScoreTopY = 24.0f;
constexpr float kScorePixelSize = 10.0f;
constexpr float kScoreGlyphSpacing = 4.0f;
constexpr float kScoreLeftCenterX = static_cast<float>(kLogicalWidth) * 0.25f;
constexpr float kScoreRightCenterX = static_cast<float>(kLogicalWidth) * 0.75f;

// Placeholder match dynamics, driven from Application::update so the readout cycles through every digit shape (single
// and two-digit) for visual inspection of the bitmap font. Every kScoreUpdateIntervalSeconds a random player is
// awarded one point; once either player reaches kScoreWinningPoints both scores reset to 0.
//
// kScoreWinningPoints = 11 matches the original Atari Pong arcade cabinet (1972) -- the canonical "first to 11 wins"
// arcade rule. The Atari 2600 home variants later added 15- and 21-point modes as switch options; 11 stays the most
// recognizable target and the smallest value that exercises a two-digit readout. The whole placeholder collapses into
// the Scoring-and-match-flow milestone, which replaces the random ticker with real point-awarding logic.
constexpr int kScoreWinningPoints = 11;
constexpr float kScoreUpdateIntervalSeconds = 1.0f;
} // namespace Playfield
