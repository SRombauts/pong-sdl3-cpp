#pragma once

#include "PaddleController.h"

// The default IPaddleController whenever Application is constructed without an explicit controller. Returns a no-op
// PaddleControllerRequest (axis = 0, no absolute target) on every tick.
//
// Lives under src/, not tests/, because production code uses it too: as the Application default constructor's
// fallback when the user does not wire a real controller, and as the controller of choice for any state where input
// must be ignored without tearing down the frame loop (paused menus, "watch the AI demo" mode, debug freeze).
class PaddleControllerNull final : public IPaddleController
{
public:
    PaddleControllerNull() = default;
    ~PaddleControllerNull() override = default;

    [[nodiscard]] PaddleControllerRequest tick(const KeyboardState& keyboard) override;
};
