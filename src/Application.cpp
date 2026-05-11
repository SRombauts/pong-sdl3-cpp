#include "Application.h"

#include <SDL3/SDL.h>

#include <iostream>
#include <utility>

Application::Application(std::string title, int width, int height)
    : m_title(std::move(title)), m_width(width), m_height(height)
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

    // Try V-Sync. Failure is non-fatal: we deliberately do not add a
    // manual SDL_DelayNS-based 60 FPS cap as the issue suggests, because
    // burning CPU on a tight loop without V-Sync is uglier than running
    // uncapped on a machine that opted out of vsync. Revisit if a future
    // need (e.g. headless CI, deterministic test runs) requires a bounded
    // loop without V-Sync.
    if (!SDL_SetRenderVSync(m_renderer, 1))
    {
        std::cerr << "SDL_SetRenderVSync(1) failed (non-fatal, running uncapped): " << SDL_GetError() << std::endl;
    }
    else
    {
        std::cout << "SDL_SetRenderVSync(1) OK" << std::endl;
    }

    return true;
}

int Application::run()
{
    std::cout << "Application::run() entering main loop" << std::endl;

    // Frame pacing: when V-Sync was enabled in init(), SDL_RenderPresent
    // blocks until vblank and the loop is naturally capped at the display
    // refresh rate. When V-Sync is unavailable we currently run uncapped
    // (the manual SDL_DelayNS fallback the issue suggests is deliberately
    // skipped -- see the comment in init() for the rationale).
    //
    // Per-frame timing: SDL_GetTicksNS is a monotonic nanosecond counter,
    // safe across the typical game-session length (overflow at ~584 years).
    // We seed it just before the loop so the very first dt is the tiny gap
    // between this call and the next one, not the entire startup duration.
    // The conversion ns -> seconds is currently inline; it will move into a
    // tested helper alongside the Clock abstraction in a follow-up issue.
    Uint64 lastTickNs = SDL_GetTicksNS();

    while (pollEvents())
    {
        const Uint64 nowNs = SDL_GetTicksNS();
        const Uint64 deltaNs = nowNs - lastTickNs;
        lastTickNs = nowNs;
        const double dtSeconds = static_cast<double>(deltaNs) / 1.0e9;

        update(dtSeconds);
        render();
    }

    std::cout << "Application::run() exiting main loop" << std::endl;
    return 0;
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
            return false;
        }
        else if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)
        {
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
    SDL_RenderPresent(m_renderer);
}
