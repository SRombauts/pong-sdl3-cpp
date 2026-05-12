#pragma once

#include "Clock.h"

#include <cstdint>
#include <memory>
#include <string>

struct SDL_Window;
struct SDL_Renderer;

// Owns the SDL3 lifetime (init, window, renderer, event loop) and
// per-frame timing. Reads time through an injected IClock so the frame
// loop can be exercised with a ClockFake in tests, without SDL video.
// main.cpp stays a thin entry point.
class Application
{
public:
    Application(std::string title, int width, int height, std::unique_ptr<IClock> clock = nullptr);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    // Initialise SDL3, create the window and the renderer. Returns true on
    // success, false on any SDL failure (in which case SDL_GetError() has
    // been logged and the destructor is still safe to run).
    bool init();

    // Drive the main loop until the user closes the window or presses
    // Escape. Returns a process exit code (0 on a clean exit). Must only
    // be called after a successful init().
    int run();

    // Elapsed seconds since the previous tick; updates the cache.
    // Public so tests can drive the frame clock with a ClockFake
    // without bringing up SDL video.
    //
    // Precondition: init() seeded the cache, or a test discarded one
    // call to prime it the same way.
    [[nodiscard]] double tickFrameClock();

private:
    bool pollEvents();
    void update(double dtSeconds);
    void render();

    std::string m_title;
    int m_width = 0;
    int m_height = 0;

    bool m_sdlInitialised = false;
    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;

    std::unique_ptr<IClock> m_clock;
    std::uint64_t m_lastTickNs = 0;
};
