#include "KeyboardState.h"

#include <SDL3/SDL_keyboard.h>

#include <algorithm>
#include <cstddef>

bool KeyboardState::isDown(SDL_Scancode scancode) const
{
    const int idx = static_cast<int>(scancode);
    if (idx < 0 || idx >= static_cast<int>(SDL_SCANCODE_COUNT))
    {
        return false;
    }
    return keys[static_cast<std::size_t>(idx)];
}

KeyboardState KeyboardState::withKeysDown(std::initializer_list<SDL_Scancode> downKeys)
{
    KeyboardState state{};
    for (SDL_Scancode key : downKeys)
    {
        const int idx = static_cast<int>(key);
        if (idx < 0 || idx >= static_cast<int>(SDL_SCANCODE_COUNT))
        {
            continue;
        }
        state.keys[static_cast<std::size_t>(idx)] = true;
    }
    return state;
}

KeyboardState KeyboardState::allKeysDown()
{
    KeyboardState state{};
    state.keys.fill(true);
    return state;
}

KeyboardState snapshotKeyboardState()
{
    // SDL3 returns const bool* (one entry per scancode, indexed by SDL_Scancode) and writes the array length into
    // `numkeys`. The pointer is owned by SDL and remains valid for the application lifetime, so the copy is safe to
    // perform unsynchronised: we never dereference it after this function returns.
    int sdlKeyCount = 0;
    const bool* sdlKeys = SDL_GetKeyboardState(&sdlKeyCount);

    KeyboardState snapshot{};
    if (sdlKeys == nullptr || sdlKeyCount <= 0)
    {
        return snapshot;
    }

    // Defend against an SDL build whose scancode count differs from ours -- copy at most what our array holds, and at
    // most what SDL produced. The runtime min keeps the copy bounded in both directions; SDL_SCANCODE_COUNT and
    // sdlKeyCount agree in practice (both 512 in release-3.4.8) but the helper stays robust to a future header bump.
    const std::size_t copyCount =
        std::min(static_cast<std::size_t>(sdlKeyCount), static_cast<std::size_t>(SDL_SCANCODE_COUNT));
    std::copy_n(sdlKeys, copyCount, snapshot.keys.begin());
    return snapshot;
}
