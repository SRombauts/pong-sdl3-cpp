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
// 800x600 is picked as the starting logical resolution to match the current default window size (see main.cpp);
// the two values are independent and the window can be resized freely at runtime without affecting gameplay
// coordinates.
namespace Playfield
{
constexpr int kLogicalWidth = 800;
constexpr int kLogicalHeight = 600;
} // namespace Playfield
