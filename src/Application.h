#pragma once

#include "Clock.h"
#include "RandomSource.h"

#include <cstdint>
#include <memory>
#include <string>

struct SDL_Window;
struct SDL_Renderer;
class PlayfieldRenderer;

// Owns the SDL3 lifetime (init, window, renderer, event loop) and per-frame timing. Reads time through an injected
// IClock and random numbers through an injected IRandomSource, so the frame loop can run with a ClockFake and scripted
// fakes in tests without SDL video. main.cpp stays a thin entry point.
class Application
{
public:
    Application(std::string title,
                int width,
                int height,
                std::unique_ptr<IClock> clock = nullptr,
                std::unique_ptr<IRandomSource> random = nullptr);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    // Initialise SDL3, create the window and the renderer. Returns true on success, false on any SDL failure (in which
    // case SDL_GetError() has been logged and the destructor is still safe to run).
    bool init();

    // Drive the main loop until the user closes the window or presses Escape. Returns a process exit code (0 on a
    // clean exit). Must only be called after a successful init().
    int run();

    // Elapsed seconds since the previous tick; updates the cache. Public so tests can drive the frame clock with a
    // ClockFake without bringing up SDL video.
    //
    // Precondition: init() seeded the cache, or a test discarded one call to prime it the same way.
    [[nodiscard]] double tickFrameClock();

    // Exposed for gameplay code (serve angles, AI noise); not intended for tests, which inject their own source.
    [[nodiscard]] IRandomSource& random() noexcept
    {
        return *m_random;
    }

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
    std::unique_ptr<IRandomSource> m_random;
    // Owns the static-playfield rectangles (center-line dashes). Held by unique_ptr to keep this header SDL-free; the
    // member is constructed eagerly in Application's constructor since the layout math has no SDL dependency.
    std::unique_ptr<PlayfieldRenderer> m_playfield;
    std::uint64_t m_lastTickNs = 0;

    // Placeholder match state: per-player score plus the accumulator that decides when the next random tick fires.
    // Wholly thrown away by the Scoring-and-match-flow milestone; see Playfield.h for the cadence and cap.
    int m_leftScore = 0;
    int m_rightScore = 0;
    double m_scoreTickSeconds = 0.0;
};
