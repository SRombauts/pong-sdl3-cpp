#include "PaddleControllerKeyboard.h"

#include "KeyboardState.h"

#include <doctest/doctest.h>

#include <SDL3/SDL_scancode.h>

namespace
{
// The two production binding sets, kept here so every case below exercises the exact scancodes Application wires up.
constexpr SDL_Scancode kLeftUp = SDL_SCANCODE_W;
constexpr SDL_Scancode kLeftDown = SDL_SCANCODE_S;
constexpr SDL_Scancode kRightUp = SDL_SCANCODE_UP;
constexpr SDL_Scancode kRightDown = SDL_SCANCODE_DOWN;

// Every assertion checks targetY is empty too: the keyboard is digital and must never emit an absolute target.
void expectAxis(const PaddleControllerRequest& request, float expectedAxis)
{
    CHECK(request.axis == doctest::Approx(expectedAxis));
    CHECK_FALSE(request.targetY.has_value());
}
} // namespace

TEST_CASE("PaddleControllerKeyboard: left-paddle bindings (W / S)")
{
    PaddleControllerKeyboard controller(kLeftUp, kLeftDown);

    SUBCASE("up scancode held yields axis -1")
    {
        expectAxis(controller.tick(KeyboardState::withKeysDown({kLeftUp})), -1.0f);
    }
    SUBCASE("down scancode held yields axis +1")
    {
        expectAxis(controller.tick(KeyboardState::withKeysDown({kLeftDown})), +1.0f);
    }
    SUBCASE("up and down held simultaneously yields axis 0 (dead key)")
    {
        expectAxis(controller.tick(KeyboardState::withKeysDown({kLeftUp, kLeftDown})), 0.0f);
    }
    SUBCASE("no relevant key held, only unrelated keys, yields axis 0")
    {
        const KeyboardState keyboard =
            KeyboardState::withKeysDown({SDL_SCANCODE_SPACE, SDL_SCANCODE_RETURN, SDL_SCANCODE_LSHIFT});
        expectAxis(controller.tick(keyboard), 0.0f);
    }
    SUBCASE("empty keyboard yields axis 0")
    {
        expectAxis(controller.tick(KeyboardState{}), 0.0f);
    }
}

TEST_CASE("PaddleControllerKeyboard: right-paddle bindings (Up / Down)")
{
    PaddleControllerKeyboard controller(kRightUp, kRightDown);

    SUBCASE("up scancode held yields axis -1")
    {
        expectAxis(controller.tick(KeyboardState::withKeysDown({kRightUp})), -1.0f);
    }
    SUBCASE("down scancode held yields axis +1")
    {
        expectAxis(controller.tick(KeyboardState::withKeysDown({kRightDown})), +1.0f);
    }
    SUBCASE("up and down held simultaneously yields axis 0 (dead key)")
    {
        expectAxis(controller.tick(KeyboardState::withKeysDown({kRightUp, kRightDown})), 0.0f);
    }
    SUBCASE("no relevant key held, only unrelated keys, yields axis 0")
    {
        // Include the left paddle's keys among the noise: the right controller must ignore scancodes it was not bound
        // to.
        const KeyboardState keyboard = KeyboardState::withKeysDown({kLeftUp, kLeftDown, SDL_SCANCODE_SPACE});
        expectAxis(controller.tick(keyboard), 0.0f);
    }
    SUBCASE("empty keyboard yields axis 0")
    {
        expectAxis(controller.tick(KeyboardState{}), 0.0f);
    }
}
