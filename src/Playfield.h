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
// 800x600 is picked as the starting logical resolution to match the current default window size;
// the values are independent and the window can be resized freely at runtime without affecting gameplay coordinates.
namespace Playfield
{
constexpr int kLogicalWidth = 800;
constexpr int kLogicalHeight = 600;

// Static tuning constants. Sizes are expressed as half-extents because the layout helpers and the collision math both
// operate on centres rather than top-left corners; storing the half-extent avoids scattering "/2" through call sites.
// Values are chosen to give classic arcade-Pong proportions on the 800x600 playfield.
constexpr float kPaddleHalfWidth = 6.0f;
constexpr float kPaddleHalfHeight = 40.0f;
constexpr float kBallHalfSize = 6.0f;
constexpr float kWallInset = 24.0f;

// Dashed centre line. With kCentreDashSegmentCount = 20 and (kCentreDashHeight + kCentreDashGap) = 30, the 20 evenly
// distributed bands of height 30 exactly tile the 600-pixel playfield, leaving symmetric gap/2 = 6 px clearances at
// the top and bottom edges.
constexpr float kCentreDashWidth = 6.0f;
constexpr float kCentreDashHeight = 18.0f;
constexpr float kCentreDashGap = 12.0f;
constexpr int kCentreDashSegmentCount = 20;
} // namespace Playfield
