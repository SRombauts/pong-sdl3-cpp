#pragma once

#include <string>

struct SDL_Window;
struct SDL_Renderer;

// Owns the SDL3 lifetime: SDL_Init, the window, the renderer, and (later)
// the event loop and per-frame timing. `main.cpp` is meant to stay a thin
// entry point that just instantiates this class.
class Application
{
public:
    Application(std::string title, int width, int height);
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
};
