#pragma once

#include <SDL3/SDL_scancode.h>

#include <array>
#include <initializer_list>

// Per-frame snapshot of the physical keyboard. Decoupled from SDL's internal buffer so paddle controllers can be
// unit-tested against scripted fixtures without bringing up SDL video.
//
// Storage choice: std::array<bool, SDL_SCANCODE_COUNT> copied from SDL's buffer rather than a const bool* pointer
// into it. SDL_SCANCODE_COUNT is 512 (one entry per scancode, including the gaps), so a snapshot is ~512 bytes; copying
// that once per frame is a sub-microsecond memcpy on every supported target.
// Owning the bytes -- rather than a const bool* view -- lets the snapshot be returned by value from the factories below
// and stay frozen for the frame, independent of SDL's mutable buffer.
//
// Why scancodes, not keycodes? Scancodes are physical key positions, so the W/Z dual-up binding the future
// KeyboardPaddleController uses (AZERTY swaps the W and Z physical keys) lands on the same physical key on both
// layouts. Keycodes would route the binding through the OS layout map and break that property.
//
// Type shape: struct with a public array because the type is a value snapshot with no invariants to protect; for now
// an accessor method (isDown) and two static factories (withKeysDown, allKeysDown) cover production reads and test
// construction without hiding the storage from the snapshot adapter.
struct KeyboardState
{
    // Indexed query. Returns false for any scancode outside [0, SDL_SCANCODE_COUNT) so a caller passing a freshly-cast
    // integer cannot trip an out-of-bounds array access. The defensive branch is one compare-and-jump and never fires
    // under SDL-supplied scancodes.
    [[nodiscard]] bool isDown(SDL_Scancode scancode) const;

    // Build a test fixture with the listed scancodes flagged down, all others up. Ergonomic shortcut for the
    // "I want this specific set of keys held" pattern; production code reaches the same array via
    // snapshotKeyboardState() below.
    [[nodiscard]] static KeyboardState withKeysDown(std::initializer_list<SDL_Scancode> keys);

    // Build a test fixture with every entry flagged down: the pathological "all keys held simultaneously" case used to
    // pin PaddleControllerNull's no-input contract under maximum noise. Separate factory rather than extending
    // withKeysDown because enumerating 512 scancode literals at every call site is impractical.
    [[nodiscard]] static KeyboardState allKeysDown();

    std::array<bool, SDL_SCANCODE_COUNT> keys{};
};

// Production adapter: copy SDL's per-frame keyboard buffer into a fresh KeyboardState. The sole consumer of
// SDL_GetKeyboardState in the codebase; every other reader (Application, controllers, tests) goes through the value
// type above so the SDL surface stays one line wide.
[[nodiscard]] KeyboardState snapshotKeyboardState();
