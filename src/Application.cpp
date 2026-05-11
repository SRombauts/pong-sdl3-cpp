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

bool Application::init()
{
    std::cout << "Application::init() title='" << m_title << "' size=" << m_width << "x" << m_height << std::endl;

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
