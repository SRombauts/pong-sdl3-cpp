#pragma once

#include "Clock.h"

#include <cstdint>

// Production IClock backed by SDL_GetTicksNS. Sole caller of
// SDL_GetTicksNS in the codebase; everything else reads time through
// IClock so it stays unit-testable.
class SdlTicksClock : public IClock
{
public:
    SdlTicksClock() = default;
    ~SdlTicksClock() override = default;

    std::uint64_t now() const override;
};
