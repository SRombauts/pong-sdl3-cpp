#pragma once

#include <optional>

struct KeyboardState;

// Per-tick request from a paddle controller back to the paddle update step. Carries either a velocity-style axis
// intent or an absolute-position target -- the two PaddleMotion families are PaddleMotion::stepCenterY(paddle, axis,
// ...) and PaddleMotion::clampCenterY(targetY, ...). The keyboard and gamepad-stick controllers set axis only; the
// mouse and AI absolute-intercept controllers set targetY only. No controller sets both today; future code that
// mixes them must document the precedence rule alongside the dispatch site.
//
// Plain-data struct rather than a class hierarchy of "request" types because the two fields are cheap to carry and
// switching on `targetY.has_value()` at the dispatch site reads more clearly than a visitor walk. Both fields default
// to the "no input" state so a PaddleControllerNull and a half-initialised request behave identically.
struct PaddleControllerRequest
{
    float axis = 0.0f;
    std::optional<float> targetY;
};

// Polymorphic seam between input devices and paddle motion. One instance per paddle, owned by Application via
// std::unique_ptr<IPaddleController>.
//
// Why tick() takes a KeyboardState even though future mouse, gamepad, and AI controllers won't read it:
//   - keeps one stable signature across every controller, so the Application dispatch loop stays a single
//     "controller->tick(keyboard)" line per paddle rather than branching on the controller's runtime type.
//   - mirrors how SDL itself exposes a per-frame keyboard snapshot, so the keyboard controller's read path is
//     "decode-from-snapshot" rather than "poll the device at tick time".
//   - lets the Analog-and-gamepad milestone extend the parameter list to (KeyboardState, MouseState, GamepadState)
//     without breaking the keyboard controller or its tests.
//
// Alternatives considered:
//   - tick() with no parameter, each controller polling the device it cares about itself: forces every controller to
//     know about SDL_GetKeyboardState/SDL_GetMouseState, which spreads the SDL surface area into the controller
//     layer and breaks the "single owner per SDL API" rule.
//   - tick(const FrameInputState&) bundling every input device in one struct: bigger upfront type, churns every
//     time a new device shows up, and only earns its keep once we have three or more devices. Revisit if the
//     extension pattern above produces more than two parameter additions.
class IPaddleController
{
public:
    IPaddleController() = default;
    virtual ~IPaddleController() = default;

    IPaddleController(const IPaddleController&) = delete;
    IPaddleController& operator=(const IPaddleController&) = delete;
    IPaddleController(IPaddleController&&) = delete;
    IPaddleController& operator=(IPaddleController&&) = delete;

    // Read the per-frame input snapshot and produce one PaddleControllerRequest. Pure with respect to SDL: the
    // production keyboard implementation reads the passed-in KeyboardState rather than calling SDL_GetKeyboardState
    // itself, so a scripted KeyboardState fixture covers every test case without bringing up SDL video.
    [[nodiscard]] virtual PaddleControllerRequest tick(const KeyboardState& keyboard) = 0;
};
