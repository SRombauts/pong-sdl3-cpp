#include "PaddleControllerNull.h"

#include "KeyboardState.h"

#include <doctest/doctest.h>

#include <SDL3/SDL_scancode.h>

namespace
{
// Helper used by every case below to assert the no-input contract in one place: axis is exactly zero, no absolute
// target. Centralises the contract so a future change to PaddleControllerRequest's shape only touches one spot.
void expectNoInput(const PaddleControllerRequest& request)
{
    CHECK(request.axis == doctest::Approx(0.0f));
    CHECK_FALSE(request.targetY.has_value());
}
} // namespace

TEST_CASE("PaddleControllerNull::tick: empty keyboard yields no input")
{
    PaddleControllerNull controller;
    const KeyboardState keyboard{};
    expectNoInput(controller.tick(keyboard));
}

TEST_CASE("PaddleControllerNull::tick: every relevant key held yields no input")
{
    // The keys the next PR's KeyboardPaddleController will look at -- W, Z, S for the left paddle and Up, Down for
    // the right -- all held at once. The null controller must still report no input: it ignores its keyboard
    // argument by design, and this case proves it.
    PaddleControllerNull controller;
    const KeyboardState keyboard = KeyboardState::withKeysDown(
        {SDL_SCANCODE_W, SDL_SCANCODE_Z, SDL_SCANCODE_S, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN});
    expectNoInput(controller.tick(keyboard));
}

TEST_CASE("PaddleControllerNull::tick: pathological all-keys-down keyboard still yields no input")
{
    // The maximum-noise fixture -- every in-range scancode flagged down -- pins the no-input contract under any
    // SDL state the production code could ever observe.
    PaddleControllerNull controller;
    const KeyboardState keyboard = KeyboardState::allKeysDown();
    expectNoInput(controller.tick(keyboard));
}

TEST_CASE("PaddleControllerNull::tick: repeated calls keep returning the same no-input request")
{
    // The interface is stateful in principle (a future MousePaddleController may smooth across frames). The null
    // controller must remain stateless so swapping it in temporarily (e.g. for a paused menu) never carries hidden
    // state into a later swap-back.
    PaddleControllerNull controller;
    const KeyboardState keyboardA{};
    const KeyboardState keyboardB = KeyboardState::withKeysDown({SDL_SCANCODE_W});
    const KeyboardState keyboardC = KeyboardState::allKeysDown();

    expectNoInput(controller.tick(keyboardA));
    expectNoInput(controller.tick(keyboardB));
    expectNoInput(controller.tick(keyboardC));
    expectNoInput(controller.tick(keyboardA));
}
