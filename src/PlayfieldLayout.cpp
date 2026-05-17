#include "PlayfieldLayout.h"

namespace PlayfieldLayout
{

SDL_FRect ball(float playfieldWidth, float playfieldHeight, float ballHalfSize)
{
    SDL_FRect rect{};
    rect.x = playfieldWidth * 0.5f - ballHalfSize;
    rect.y = playfieldHeight * 0.5f - ballHalfSize;
    rect.w = ballHalfSize * 2.0f;
    rect.h = ballHalfSize * 2.0f;
    return rect;
}

std::vector<SDL_FRect> centerDashSegments(float playfieldWidth,
                                          float playfieldHeight,
                                          int segmentCount,
                                          float dashWidth,
                                          float dashHeight,
                                          float gap)
{
    std::vector<SDL_FRect> dashes;
    if (segmentCount <= 0)
    {
        return dashes;
    }

    const float dashX = playfieldWidth * 0.5f - dashWidth * 0.5f;

    if (segmentCount == 1)
    {
        // One dash cannot honor the "first dash starts at gap/2" symmetry rule, so center it vertically instead. This
        // matches the defensive case documented in the header.
        SDL_FRect rect{};
        rect.x = dashX;
        rect.y = playfieldHeight * 0.5f - dashHeight * 0.5f;
        rect.w = dashWidth;
        rect.h = dashHeight;
        dashes.push_back(rect);
        return dashes;
    }

    dashes.reserve(static_cast<std::size_t>(segmentCount));
    const float pitch = dashHeight + gap;
    for (int k = 0; k < segmentCount; ++k)
    {
        SDL_FRect rect{};
        rect.x = dashX;
        rect.y = gap * 0.5f + static_cast<float>(k) * pitch;
        rect.w = dashWidth;
        rect.h = dashHeight;
        dashes.push_back(rect);
    }
    return dashes;
}

} // namespace PlayfieldLayout
