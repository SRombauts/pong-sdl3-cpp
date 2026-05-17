#pragma once

#include <SDL3/SDL.h>

#include <vector>

// Layout helpers for the static playfield. Each function takes the playfield dimensions plus the tunable sizes
// (see Playfield.h for the canonical values) and returns an SDL_FRect in logical-pixel coordinates that the renderer
// can hand straight to SDL_RenderFillRect.
//
// Helpers expose the half-extent inputs separately rather than full width/height so the call site can express the
// "centered on" intent directly without scattering "/ 2.0f" arithmetic.
namespace PlayfieldLayout
{
// Ball centered on both axes of the playfield.
SDL_FRect ball(float playfieldWidth, float playfieldHeight, float ballHalfSize);

// Evenly distributed dashes for the vertical center line.
//
// Layout convention:
//   - The playfield height is split into `segmentCount` equal vertical bands and one dash is centered in each band.
//     Equivalently, dash k (0-indexed) sits at y = gap/2 + k * (dashHeight + gap), so the line is symmetric around
//     the playfield's vertical midpoint when (dashHeight + gap) divides the playfield height evenly.
//   - Every dash is horizontally centered on `playfieldWidth / 2`.
//
// Defensive behavior:
//   - segmentCount == 0 returns an empty vector (no crash, no negative dimensions).
//   - segmentCount == 1 returns a single dash centered on both axes; the gap parameter is ignored in that case because
//     "first dash starts at gap/2" no longer pins the line symmetrically when only one dash exists.
//
// The `gap` parameter is the documented vertical spacing *between* consecutive dashes; the caller picks it (typically
// from Playfield::kCenterDashGap) so the helper does not need to know the production tuning.
std::vector<SDL_FRect> centerDashSegments(float playfieldWidth,
                                          float playfieldHeight,
                                          int segmentCount,
                                          float dashWidth,
                                          float dashHeight,
                                          float gap);

} // namespace PlayfieldLayout
