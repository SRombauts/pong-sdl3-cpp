#include "PaddleMotion.h"

#include <algorithm>

float clampPaddleCenterY(float centerY, float halfHeight, float playfieldHeight)
{
    const float minCenter = halfHeight;
    const float maxCenter = playfieldHeight - halfHeight;

    // Oversized paddle: the two bounds cross. Centering on the playfield's midpoint is the only choice that does not
    // arbitrarily prefer one wall over the other; the paddle still extends past both walls visibly.
    if (minCenter > maxCenter)
    {
        return playfieldHeight * 0.5f;
    }

    return std::clamp(centerY, minCenter, maxCenter);
}

float stepPaddleCenterY(float axis,
                        float currentCenterY,
                        float halfHeight,
                        float speed,
                        float playfieldHeight,
                        double dtSeconds)
{
    // Defend at the boundary: negative `speed` or negative `dtSeconds` would make `maxStep` negative, and
    // `std::clamp(x, -maxStep, maxStep)` below becomes `std::clamp(x, lo, hi)` with `lo > hi` -- undefined behavior per
    // [alg.clamp]. The header documents both as preconditions, but the helper is unit-tested and reusable, so it
    // collapses degenerate inputs to a zero step instead of asserting (same shape as FrameTiming::secondsBetween,
    // which clamps a non-monotonic delta to 0.0).
    const float maxStep = std::max(0.0f, speed * static_cast<float>(dtSeconds));

    // Single clamp covers both directions and also catches out-of-range axis values (e.g. a buggy controller returning
    // axis = 2.0f cannot move the paddle faster than the speed cap).
    const float cappedDelta = std::clamp(axis * maxStep, -maxStep, maxStep);
    return clampPaddleCenterY(currentCenterY + cappedDelta, halfHeight, playfieldHeight);
}
