#pragma once

#include <cstdint>

// Pure free function for per-frame timing arithmetic, so the frame
// loop can be tested without standing up SDL.

// Elapsed time in seconds, defensively clamped to 0.0 when nowNs <=
// prevNs so a non-monotonic reading cannot inject a negative dt.
double secondsBetween(std::uint64_t prevNs, std::uint64_t nowNs);
