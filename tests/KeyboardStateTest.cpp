#include "KeyboardState.h"

#include <doctest/doctest.h>

#include <SDL3/SDL_scancode.h>

// ---------------------------------------------------------------------------
// isDown -- indexed query
// ---------------------------------------------------------------------------

TEST_CASE("KeyboardState: default-constructed state reports every scancode as up")
{
    const KeyboardState state{};

    CHECK_FALSE(state.isDown(SDL_SCANCODE_W));
    CHECK_FALSE(state.isDown(SDL_SCANCODE_S));
    CHECK_FALSE(state.isDown(SDL_SCANCODE_UP));
    CHECK_FALSE(state.isDown(SDL_SCANCODE_DOWN));
    CHECK_FALSE(state.isDown(SDL_SCANCODE_ESCAPE));
    CHECK_FALSE(state.isDown(SDL_SCANCODE_RETURN));
    CHECK_FALSE(state.isDown(SDL_SCANCODE_SPACE));
}

TEST_CASE("KeyboardState::isDown: out-of-range scancode returns false rather than indexing past the array")
{
    // SDL_Scancode is a C-style enum and the language admits values outside the documented range. The defensive branch
    // in isDown means a freshly-cast garbage scancode cannot trip an out-of-bounds read.
    const KeyboardState state = KeyboardState::allKeysDown();
    CHECK_FALSE(state.isDown(static_cast<SDL_Scancode>(-1)));
    CHECK_FALSE(state.isDown(static_cast<SDL_Scancode>(SDL_SCANCODE_COUNT)));
    CHECK_FALSE(state.isDown(static_cast<SDL_Scancode>(99'999)));
}

// ---------------------------------------------------------------------------
// withKeysDown -- builder for "a few keys held" fixtures
// ---------------------------------------------------------------------------

TEST_CASE("KeyboardState::withKeysDown: only the listed scancodes report down")
{
    const KeyboardState state = KeyboardState::withKeysDown({SDL_SCANCODE_W, SDL_SCANCODE_S});

    CHECK(state.isDown(SDL_SCANCODE_W));
    CHECK(state.isDown(SDL_SCANCODE_S));

    // Every other scancode the keyboard controller will look at must still report up.
    CHECK_FALSE(state.isDown(SDL_SCANCODE_Z));
    CHECK_FALSE(state.isDown(SDL_SCANCODE_UP));
    CHECK_FALSE(state.isDown(SDL_SCANCODE_DOWN));
    CHECK_FALSE(state.isDown(SDL_SCANCODE_A));
}

TEST_CASE("KeyboardState::withKeysDown: empty list leaves every scancode up")
{
    // Pinned because withKeysDown is the natural fixture for the "no relevant key held" branch in the next PR's
    // KeyboardPaddleController tests.
    const KeyboardState state = KeyboardState::withKeysDown({});
    CHECK_FALSE(state.isDown(SDL_SCANCODE_W));
    CHECK_FALSE(state.isDown(SDL_SCANCODE_S));
    CHECK_FALSE(state.isDown(SDL_SCANCODE_UP));
    CHECK_FALSE(state.isDown(SDL_SCANCODE_DOWN));
}

TEST_CASE("KeyboardState::withKeysDown: out-of-range scancodes in the list are silently skipped")
{
    // Same defensive contract as isDown: a caller passing a garbage scancode (e.g. from a future SDL version with
    // extra scancodes) must not corrupt the array. The valid entries in the list still register.
    const KeyboardState state = KeyboardState::withKeysDown(
        {static_cast<SDL_Scancode>(-1), SDL_SCANCODE_W, static_cast<SDL_Scancode>(SDL_SCANCODE_COUNT), SDL_SCANCODE_S});

    CHECK(state.isDown(SDL_SCANCODE_W));
    CHECK(state.isDown(SDL_SCANCODE_S));
}

// ---------------------------------------------------------------------------
// allKeysDown -- pathological "every key held" fixture
// ---------------------------------------------------------------------------

TEST_CASE("KeyboardState::allKeysDown: every in-range scancode reports down")
{
    const KeyboardState state = KeyboardState::allKeysDown();

    // Spot-check the scancodes both paddle controllers will read.
    CHECK(state.isDown(SDL_SCANCODE_W));
    CHECK(state.isDown(SDL_SCANCODE_Z));
    CHECK(state.isDown(SDL_SCANCODE_S));
    CHECK(state.isDown(SDL_SCANCODE_UP));
    CHECK(state.isDown(SDL_SCANCODE_DOWN));

    // Walk the array bound to catch any gap a regression might leave behind.
    for (int idx = 0; idx < static_cast<int>(SDL_SCANCODE_COUNT); ++idx)
    {
        CHECK(state.isDown(static_cast<SDL_Scancode>(idx)));
    }
}
