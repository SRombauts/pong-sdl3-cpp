#include "Application.h"

#include "ClockSdlTicks.h"
#include "FrameTiming.h"
#include "Playfield.h"
#include "PlayfieldLayout.h"
#include "PlayfieldRenderer.h"
#include "RandomSourceMt19937.h"
#include "Score.h"
#include "TextRenderer.h"

#include <SDL3/SDL.h>

#include <cstdint>
#include <iostream>
#include <string>
#include <utility>

Application::Application(std::string title,
                         int width,
                         int height,
                         std::unique_ptr<IClock> clock,
                         std::unique_ptr<IRandomSource> random)
    : m_title(std::move(title)), m_width(width), m_height(height),
      m_clock(clock ? std::move(clock) : std::make_unique<ClockSdlTicks>()),
      m_random(random ? std::move(random) : std::make_unique<RandomSourceMt19937>(makeNonDeterministicSeed())),
      m_playfield(std::make_unique<PlayfieldRenderer>(Playfield::kLogicalWidth,
                                                      Playfield::kLogicalHeight,
                                                      Playfield::kCenterDashSegmentCount,
                                                      Playfield::kCenterDashWidth,
                                                      Playfield::kCenterDashHeight,
                                                      Playfield::kCenterDashGap))
{
}

Application::~Application()
{
    // Tear down in reverse construction order; each guard keeps the destructor safe after a failed or partial init().
    if (m_renderer != nullptr)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    if (m_window != nullptr)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    if (m_sdlInitialised)
    {
        SDL_Quit();
        m_sdlInitialised = false;
    }
}

static void logSDLVersion()
{
    const int version = SDL_GetVersion();
    const int major = SDL_VERSIONNUM_MAJOR(version);
    const int minor = SDL_VERSIONNUM_MINOR(version);
    const int micro = SDL_VERSIONNUM_MICRO(version);
    std::cout << "SDL3 runtime version: " << major << "." << minor << "." << micro << std::endl;
}

bool Application::init()
{
    std::cout << "Application::init() title='" << m_title << "' size=" << m_width << "x" << m_height << std::endl;

    logSDLVersion();

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }
    m_sdlInitialised = true;
    std::cout << "SDL_Init(SDL_INIT_VIDEO) OK" << std::endl;

    m_window = SDL_CreateWindow(m_title.c_str(), m_width, m_height, SDL_WINDOW_RESIZABLE);
    if (m_window == nullptr)
    {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        return false;
    }
    std::cout << "SDL_CreateWindow OK" << std::endl;

    // SDL3 renderer creation: nullptr name = first available driver.
    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (m_renderer == nullptr)
    {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        return false;
    }
    std::cout << "SDL_CreateRenderer OK (driver='" << SDL_GetRendererName(m_renderer) << "')" << std::endl;

    // Pin gameplay coordinates to a stable logical resolution; SDL scales them to the window. LETTERBOX over STRETCH
    // (distorts aspect ratio) and OVERSCAN (crops the playfield): the 4:3 playable area stays intact with black bars
    // where the window aspect differs. INTEGER_SCALE waits for a pixel-perfect need. Failure is non-fatal -- the window
    // still presents without the logical mapping -- but every layout assumes success, so log loudly.
    // SDL takes the logical resolution as int; the Playfield constants are float so every gameplay site reads them
    // straight into float math without a per-call cast. The single float→int cast lives here, at the SDL boundary,
    // matching the "SDL stays concentrated at thin adapters" rule. The constants are integer-valued (800.0f, 600.0f)
    // so the conversion is bit-exact.
    if (!SDL_SetRenderLogicalPresentation(m_renderer,
                                          static_cast<int>(Playfield::kLogicalWidth),
                                          static_cast<int>(Playfield::kLogicalHeight),
                                          SDL_LOGICAL_PRESENTATION_LETTERBOX))
    {
        std::cerr << "SDL_SetRenderLogicalPresentation failed (non-fatal): " << SDL_GetError() << std::endl;
    }
    else
    {
        std::cout << "SDL_SetRenderLogicalPresentation(" << Playfield::kLogicalWidth << "x" << Playfield::kLogicalHeight
                  << ", LETTERBOX) OK" << std::endl;
    }

    // V-Sync failure is non-fatal: the loop runs uncapped. A manual cap can land later if a use case (headless CI,
    // broken vsync) demands it.
    if (!SDL_SetRenderVSync(m_renderer, 1))
    {
        std::cerr << "SDL_SetRenderVSync(1) failed (non-fatal, running uncapped): " << SDL_GetError() << std::endl;
    }
    else
    {
        std::cout << "SDL_SetRenderVSync(1) OK" << std::endl;
    }

    // Seed the cache so the first tickFrameClock() returns a real frame dt, not the whole pre-init duration.
    m_lastTickNs = m_clock->now();

    return true;
}

int Application::run()
{
    std::cout << "Application::run() entering main loop" << std::endl;

    while (pollEvents())
    {
        const double dtSeconds = tickFrameClock();
        update(dtSeconds);
        render();
    }

    std::cout << "Application::run() exiting main loop" << std::endl;
    return 0;
}

double Application::tickFrameClock()
{
    const std::uint64_t now = m_clock->now();
    const double dtSeconds = secondsBetween(m_lastTickNs, now);
    m_lastTickNs = now;
    return dtSeconds;
}

bool Application::pollEvents()
{
    // Pump SDL events. Returns false on the first quit signal (window close or Escape) without draining the queue;
    // unread events would be discarded by SDL_Quit shortly. Returns true once the queue is empty.
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            std::cout << "SDL_EVENT_QUIT received" << std::endl;
            return false;
        }
        else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
        {
            std::cout << "SDL_EVENT_KEY_DOWN received" << std::endl;
            return false;
        }
    }
    return true;
}

void Application::update(double dtSeconds)
{
    placeholderScoreDriver(dtSeconds);
}

void Application::placeholderScoreDriver(double dtSeconds)
{
    // Every kScoreUpdateIntervalSeconds, award one point to a uniformly random player. When either side reaches
    // kScoreWinningPoints (arcade-Pong cap) both scores reset to 0 so the readout cycles through every digit shape
    // for visual inspection. Replaced wholesale by the Scoring-and-match-flow milestone.
    //
    // Loop (not just one increment per call) so a very long dt frame -- e.g. the application paused under a debugger
    // -- catches up cleanly instead of starving the score updates and stalling visually.
    m_scoreTickSeconds += dtSeconds;
    while (m_scoreTickSeconds >= Playfield::kScoreUpdateIntervalSeconds)
    {
        m_scoreTickSeconds -= Playfield::kScoreUpdateIntervalSeconds;
        const int scoringSide = m_random->intInRange(0, 1);
        Score& scoreToBump = (scoringSide == 0) ? m_leftScore : m_rightScore;
        setScore(scoreToBump, scoreToBump.value + 1);

        if (m_leftScore.value > Playfield::kScoreWinningPoints || m_rightScore.value > Playfield::kScoreWinningPoints)
        {
            setScore(m_leftScore, 0);
            setScore(m_rightScore, 0);
        }
    }
}

void Application::render()
{
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    // White-on-black is the only palette the static playfield needs.
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);

    const SDL_FRect leftPaddle = PlayfieldLayout::leftPaddle(Playfield::kLogicalWidth,
                                                             Playfield::kLogicalHeight,
                                                             Playfield::kPaddleHalfWidth,
                                                             Playfield::kPaddleHalfHeight,
                                                             Playfield::kWallInset);
    const SDL_FRect rightPaddle = PlayfieldLayout::rightPaddle(Playfield::kLogicalWidth,
                                                               Playfield::kLogicalHeight,
                                                               Playfield::kPaddleHalfWidth,
                                                               Playfield::kPaddleHalfHeight,
                                                               Playfield::kWallInset);
    const SDL_FRect ball =
        PlayfieldLayout::ball(Playfield::kLogicalWidth, Playfield::kLogicalHeight, Playfield::kBallHalfSize);
    SDL_RenderFillRect(m_renderer, &leftPaddle);
    SDL_RenderFillRect(m_renderer, &rightPaddle);
    SDL_RenderFillRect(m_renderer, &ball);

    // Static-chrome draw: the dash list was computed once at construction; no per-frame layout math here.
    m_playfield->draw(m_renderer);

    // Per-player scores: each Score bundles the numeric value with its cached decimal text, refreshed on the score
    // change event by setScore(). render() just reads .text every frame; no formatting happens here.
    TextRenderer::drawTextCentered(m_renderer,
                                   m_leftScore.text,
                                   Playfield::kScoreLeftCenterX,
                                   Playfield::kScoreTopY,
                                   Playfield::kScorePixelSize,
                                   Playfield::kScoreGlyphSpacing);
    TextRenderer::drawTextCentered(m_renderer,
                                   m_rightScore.text,
                                   Playfield::kScoreRightCenterX,
                                   Playfield::kScoreTopY,
                                   Playfield::kScorePixelSize,
                                   Playfield::kScoreGlyphSpacing);

    // Restore the clear color so the next SDL_RenderClear() starts from black even if a future caller forgets it.
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);

    SDL_RenderPresent(m_renderer);
}
