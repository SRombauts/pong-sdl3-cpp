#pragma once

#include "PaddleController.h"

#include <SDL3/SDL_scancode.h>

// The keyboard-driven IPaddleController: maps an "up" scancode and a "down" scancode to a velocity-style
// PaddleControllerRequest. One instance per human-controlled paddle, owned by Application.
//
// Bindings are scancodes (physical key positions), not keycodes, so the same physical keys drive the paddle on every
// keyboard layout without a layout-detection step -- e.g. SDL_SCANCODE_W is the key above S whether its cap reads W
// (QWERTY) or Z (AZERTY). KeyboardState.h carries the full rationale; this is why one up scancode suffices.
class PaddleControllerKeyboard final : public IPaddleController
{
public:
    // upScancode maps to "move up"; downScancode maps to "move down".
    PaddleControllerKeyboard(SDL_Scancode upScancode, SDL_Scancode downScancode);
    ~PaddleControllerKeyboard() override = default;

    // Decode the snapshot into a velocity-style request:
    //   axis = -1.0f  when up is down and down is not,
    //   axis = +1.0f  when down is down and up is not,
    //   axis =  0.0f  when both are held (deliberate dead-key: neither bias wins) and when nothing is held.
    // targetY is always std::nullopt: a keyboard is a digital device with no absolute target.
    [[nodiscard]] PaddleControllerRequest tick(const KeyboardState& keyboard) override;

private:
    const SDL_Scancode m_upScancode;
    const SDL_Scancode m_downScancode;
};
