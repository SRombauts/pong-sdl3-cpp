#include "PlayfieldRenderer.h"

#include "PlayfieldLayout.h"

PlayfieldRenderer::PlayfieldRenderer(int playfieldWidth,
                                     int playfieldHeight,
                                     int segmentCount,
                                     float dashWidth,
                                     float dashHeight,
                                     float gap)
    : m_centerDashes(PlayfieldLayout::centerDashSegments(playfieldWidth,
                                                         playfieldHeight,
                                                         segmentCount,
                                                         dashWidth,
                                                         dashHeight,
                                                         gap))
{
}

void PlayfieldRenderer::draw(SDL_Renderer* renderer) const
{
    for (const SDL_FRect& dash : m_centerDashes)
    {
        SDL_RenderFillRect(renderer, &dash);
    }
}
