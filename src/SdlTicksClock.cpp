#include "SdlTicksClock.h"

#include <SDL3/SDL_timer.h>

std::uint64_t SdlTicksClock::now() const
{
    return SDL_GetTicksNS();
}
