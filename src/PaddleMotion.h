#pragma once

#include "Paddle.h"

// Pure paddle-motion helpers, split by controller family.
//
// The split is deliberate, because the two controller families have genuinely different physics:
//
//   - Absolute-position controllers (mouse, AI absolute-intercept) name a target Y in playfield coordinates and the
//     paddle goes there. This mirrors how the original Atari Pong (1972) arcade hardware and the Atari 2600 paddle
//     controllers worked: the player's potentiometer fed a timing circuit that set the paddle's vertical position
//     directly, once per frame, with no software- or hardware-imposed "max paddle speed" -- the only rate limit was
//     the player's wrist. The mouse and AI controllers in this codebase follow the same model. See README.md's
//     "Historical references" section for sources.
//
//   - Velocity-style controllers (keyboard, gamepad stick treated as a joystick) emit an axis intent in [-1, +1] and
//     have no notion of "how fast" beyond that. They need a per-tick speed cap to feel playable -- a sustained key
//     press otherwise has no information about how far to move per frame.
//
// Mapping controllers to helpers:
//
//   - Absolute-position (mouse, AI absolute-intercept) -> PaddleMotion::clampCenterY(targetY, …): snap to the target,
//     enforce the playfield bounds, no rate limit. Scalar inputs because the absolute-position controllers compute
//     their target Y outside the paddle's frame of reference and pass it in alongside the paddle's half-height.
//   - Velocity-style (keyboard, stick-as-velocity)     -> PaddleMotion::stepCenterY(paddle, axis, …, dt): per-tick
//     step capped at `paddle.speed * dt`, then clamped to the playfield via PaddleMotion::clampCenterY. Takes the
//     Paddle by const reference because the three required inputs (`centerY`, `halfHeight`, `speed`) all come from
//     the same paddle; passing them as separate scalars at every call site would let mismatched values slip in.
//
// A single unified API was considered (one struct with both an axis and an optional target) and rejected: mixing
// the two families behind one signature forced an artificial speed cap on the absolute-position path that does not
// match the historical reference, and introduced a precedence rule for a "both set" combination no real controller
// produces.

namespace PaddleMotion
{

// Clamp a paddle's center-Y so the whole rectangle stays inside the playfield's vertical bounds.
//
// Also serves as the absolute-position controller's per-tick step: a mouse or AI controller computes its target Y
// and calls this directly to snap the paddle to it, with the playfield bounds enforced.
//
//   - Top edge   : if centerY < halfHeight,                       returns halfHeight.
//   - Bottom edge: if centerY > playfieldHeight - halfHeight,     returns playfieldHeight - halfHeight.
//   - Interior   : centerY unchanged.
//
// Defensive case: when 2 * halfHeight > playfieldHeight the two clamp bounds cross (`halfHeight` > `playfieldHeight -
// halfHeight`), so the function returns the playfield's vertical midpoint instead. The paddle then visibly extends
// past both walls, but the helper never returns NaN, asserts, or picks an arbitrary edge.
[[nodiscard]] float clampCenterY(float centerY, float halfHeight, float playfieldHeight);

// Velocity-style per-tick step. Returns the paddle's new center-Y after advancing it by one tick under an axis intent
// in [-1, +1], where +1 means "down" per the +Y-down convention documented in Playfield.h. Reads three fields from
// the paddle: `centerY` (the starting position), `halfHeight` (for the trailing clamp), and `speed` (the per-tick
// displacement cap).
//
// Algorithm:
//   1. Compute the requested displacement `axis * paddle.speed * dtSeconds`, capped in magnitude at
//      `paddle.speed * dtSeconds` so a buggy controller returning `axis = 2.0f` cannot move the paddle faster than
//      its own speed cap.
//   2. Apply the capped displacement to `paddle.centerY`, then clamp via clampCenterY.
//
// Properties guaranteed by this shape (covered by unit tests):
//   - Framerate independence: one big `dt` step matches N small `dt` steps of equal sum.
//   - Speed cap: an arbitrarily out-of-range axis still moves the paddle at most `paddle.speed * dtSeconds` per call.
//
// Defensive case: a negative `paddle.speed` or `dtSeconds` collapses the cap to 0, so the call is a no-op (after the
// trailing clamp). The production caller never passes negatives -- FrameTiming::secondsBetween clamps non-monotonic
// deltas to 0.0 and `paddle.speed` is seeded from a tuning constant -- but the helper is unit-tested and reusable,
// so it never lets a `std::clamp(x, lo, hi)` with `lo > hi` slip through. `paddle.halfHeight < 0` is handled by
// clampCenterY's own boundary defense.
[[nodiscard]] float stepCenterY(const Paddle& paddle, float axis, float playfieldHeight, double dtSeconds);

} // namespace PaddleMotion
