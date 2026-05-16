#include "Application.h"

#include "ClockSdlTicks.h"
#include "FrameTiming.h"
#include "Playfield.h"
#include "PlayfieldLayout.h"
#include "PlayfieldRenderer.h"
#include "RandomSourceMt19937.h"

#include <SDL3/SDL.h>

#include <cstdint>
#include <iostream>
#include <utility>

Application::Application(std::string title, int width, int height, std::unique_ptr<IClock> clock,
                         std::unique_ptr<IRandomSource> random)
    : m_title(std::move(title)), m_width(width), m_height(height),
      m_clock(clock ? std::move(clock) : std::make_unique<ClockSdlTicks>()),
      m_random(random ? std::move(random) : std::make_unique<RandomSourceMt19937>(makeNonDeterministicSeed())),
      m_playfield(std::make_unique<PlayfieldRenderer>(Playfield::kLogicalWidth, Playfield::kLogicalHeight,
                                                      Playfield::kCentreDashSegmentCount, Playfield::kCentreDashWidth,
                                                      Playfield::kCentreDashHeight, Playfield::kCentreDashGap))
{
}

Application::~Application()
{
    // Tear down in reverse construction order. Each guard makes the
    // destructor safe to call after a failed or partial init().
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

    // Fix the gameplay coordinate system to a stable logical resolution and let SDL scale it to the window.
    // LETTERBOX is picked over STRETCH (which would distort the aspect ratio) and OVERSCAN (which would crop the
    // playfield); the user keeps the full 4:3 playable area with black bars on the sides or top/bottom when the window
    // aspect differs. INTEGER_SCALE is deferred until a pixel-perfect rendering need actually appears. Failure here is
    // non-fatal -- the window still presents, just without the logical mapping -- but every gameplay layout assumes it
    // succeeded, so surface the error loudly.
    if (!SDL_SetRenderLogicalPresentation(m_renderer, Playfield::kLogicalWidth, Playfield::kLogicalHeight,
                                          SDL_LOGICAL_PRESENTATION_LETTERBOX))
    {
        std::cerr << "SDL_SetRenderLogicalPresentation failed (non-fatal): " << SDL_GetError() << std::endl;
    }
    else
    {
        std::cout << "SDL_SetRenderLogicalPresentation(" << Playfield::kLogicalWidth << "x" << Playfield::kLogicalHeight
                  << ", LETTERBOX) OK" << std::endl;
    }

    // V-Sync failure is non-fatal: the loop runs uncapped. A manual cap
    // can land later if a use case (headless CI, broken vsync) demands it.
    if (!SDL_SetRenderVSync(m_renderer, 1))
    {
        std::cerr << "SDL_SetRenderVSync(1) failed (non-fatal, running uncapped): " << SDL_GetError() << std::endl;
    }
    else
    {
        std::cout << "SDL_SetRenderVSync(1) OK" << std::endl;
    }

    // Seed the cache so the first tickFrameClock() returns a real frame
    // dt, not the whole pre-init duration.
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
    // Pump pending SDL events. Returns false on the first quit signal
    // (window close or Escape) without finishing the drain -- the loop
    // is exiting anyway so unread events would be discarded by SDL_Quit
    // shortly. Returns true once the queue is empty.
    //
    // Future input handling (paddle controls, menu navigation) will
    // route through here without changing the call site in run().
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
    // Gameplay state updates will land here in subsequent milestones. The
    // dt is already wired through so future code can consume it without
    // touching the main loop.
    (void)dtSeconds;
}

void Application::render()
{
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);

    // White-on-black is the only palette the static playfield needs.
    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);

    const SDL_FRect leftPaddle =
        PlayfieldLayout::leftPaddle(Playfield::kLogicalWidth, Playfield::kLogicalHeight, Playfield::kPaddleHalfWidth,
                                    Playfield::kPaddleHalfHeight, Playfield::kWallInset);
    const SDL_FRect rightPaddle =
        PlayfieldLayout::rightPaddle(Playfield::kLogicalWidth, Playfield::kLogicalHeight, Playfield::kPaddleHalfWidth,
                                     Playfield::kPaddleHalfHeight, Playfield::kWallInset);
    const SDL_FRect ball =
        PlayfieldLayout::ball(Playfield::kLogicalWidth, Playfield::kLogicalHeight, Playfield::kBallHalfSize);
    SDL_RenderFillRect(m_renderer, &leftPaddle);
    SDL_RenderFillRect(m_renderer, &rightPaddle);
    SDL_RenderFillRect(m_renderer, &ball);

    // Static-chrome draw: the dash list was computed once at construction; no per-frame layout math here.
    m_playfield->draw(m_renderer);

    // Restore the clear colour so the next frame's SDL_RenderClear() starts from black even if a future caller forgets
    // to set it explicitly.
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);

    SDL_RenderPresent(m_renderer);
}
