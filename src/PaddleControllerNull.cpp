#include "PaddleControllerNull.h"

PaddleControllerRequest PaddleControllerNull::tick(const KeyboardState& /*keyboard*/)
{
    // Default-constructed PaddleControllerRequest: axis = 0, targetY = std::nullopt. The paddle update step sees a
    // zero velocity and no absolute target, so the paddle stays put regardless of which keys SDL reports as down.
    return {};
}
