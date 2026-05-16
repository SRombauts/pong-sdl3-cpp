#include "PlayfieldRenderer.h"

#include "PlayfieldLayout.h"

PlayfieldRenderer::PlayfieldRenderer(int playfieldWidth,
                                     int playfieldHeight,
                                     int segmentCount,
                                     float dashWidth,
                                     float dashHeight,
                                     float gap)
    : m_centreDashes(PlayfieldLayout::centreDashSegments(playfieldWidth,
                                                         playfieldHeight,
                                                         segmentCount,
                                                         dashWidth,
                                                         dashHeight,
                                                         gap))
{
}

void PlayfieldRenderer::draw(SDL_Renderer* renderer) const
{
    for (const SDL_FRect& dash : m_centreDashes)
    {
        SDL_RenderFillRect(renderer, &dash);
    }
}
