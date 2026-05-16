#pragma once

#include <SDL3/SDL.h>

#include <vector>

// Owns the rectangles for the static parts of the playfield (today: the dashed vertical centre line) and writes them
// to the SDL renderer once per frame. The layout math runs at construction time, not in the hot path of
// Application::render(), so the per-frame heap allocation the layout helpers' return-by-vector contract would
// otherwise introduce only happens once.
//
// Scope choice: paddles and ball are deliberately *not* owned here even though they happen to be drawn from static
// rectangles in this milestone. The Paddle-controls and Ball-and-collisions milestones replace those draws with
// entity-driven rendering, so caching them here would create churn for no payoff. PlayfieldRenderer is the home for
// chrome that stays static across the project's lifetime (centre line today; potentially border frames, score
// backings, etc. once the menus milestone lands).
class PlayfieldRenderer
{
public:
    // Pre-compute every cached rect from the playfield dimensions and the centre-line tuning. Parameters mirror
    // PlayfieldLayout::centreDashSegments by design so the class stays testable in isolation, without baking the
    // production Playfield:: constants into the type.
    //
    // No SDL renderer is required at construction; only draw() touches SDL. Tests can therefore inspect centreDashes()
    // without bringing up SDL video.
    PlayfieldRenderer(int playfieldWidth,
                      int playfieldHeight,
                      int segmentCount,
                      float dashWidth,
                      float dashHeight,
                      float gap);

    // Issue one SDL_RenderFillRect per cached dash. Does not touch the renderer's draw-colour state, so the caller can
    // set white once and draw paddles, ball, and dashes under the same setup.
    void draw(SDL_Renderer* renderer) const;

    // Read-only view of the cached dash list. Exposed so tests can verify the cache content (and the no-recompute
    // contract) without going through SDL.
    [[nodiscard]] const std::vector<SDL_FRect>& centreDashes() const noexcept
    {
        return m_centreDashes;
    }

private:
    std::vector<SDL_FRect> m_centreDashes;
};
