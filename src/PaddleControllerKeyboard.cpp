#include "PaddleControllerKeyboard.h"

#include "KeyboardState.h"

PaddleControllerKeyboard::PaddleControllerKeyboard(SDL_Scancode upScancode, SDL_Scancode downScancode)
    : m_upScancode(upScancode), m_downScancode(downScancode)
{
}

PaddleControllerRequest PaddleControllerKeyboard::tick(const KeyboardState& keyboard)
{
    const bool up = keyboard.isDown(m_upScancode);
    const bool down = keyboard.isDown(m_downScancode);

    PaddleControllerRequest request;
    if (up && !down)
    {
        request.axis = -1.0f; // up is toward the top wall: -Y in the top-left origin convention.
    }
    else if (down && !up)
    {
        request.axis = +1.0f;
    }
    // Both held or neither: axis stays at its 0.0f default (the dead-key/no-input contract documented on tick()).
    return request;
}
