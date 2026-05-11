#include <SDL3/SDL.h>

#include <iostream>

int main()
{
    std::cout << "Hello, Pong SDL3 C++!" << std::endl;

    const int version = SDL_GetVersion();
    std::cout << "SDL3 runtime version: " << SDL_VERSIONNUM_MAJOR(version) << "."
              << SDL_VERSIONNUM_MINOR(version) << "." << SDL_VERSIONNUM_MICRO(version) << std::endl;

    return 0;
}
