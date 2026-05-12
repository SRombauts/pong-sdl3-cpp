#include "ClockSdlTicks.h"

#include <SDL3/SDL_timer.h>

std::uint64_t ClockSdlTicks::now() const
{
    return SDL_GetTicksNS();
}
