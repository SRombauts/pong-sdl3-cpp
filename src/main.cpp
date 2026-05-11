#include "Application.h"

#include <SDL3/SDL.h>

#include <iostream>

int main()
{
    std::cout << "Hello, Pong SDL3 C++!" << std::endl;

    const int version = SDL_GetVersion();
    const int major = SDL_VERSIONNUM_MAJOR(version);
    const int minor = SDL_VERSIONNUM_MINOR(version);
    const int micro = SDL_VERSIONNUM_MICRO(version);
    std::cout << "SDL3 runtime version: " << major << "." << minor << "." << micro << std::endl;

    Application app("Pong SDL3 C++", 800, 600);
    if (!app.init())
    {
        return 1;
    }

    return app.run();
}
