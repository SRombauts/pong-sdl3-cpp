#include "Paddle.h"

#include <SDL3/SDL.h>

Paddle makePaddle(PaddleSide side,
                  float playfieldWidth,
                  float playfieldHeight,
                  float halfWidth,
                  float halfHeight,
                  float wallInset,
                  float speed)
{
    // Left paddle's center-X = wallInset + halfWidth, so its outer (left) edge sits exactly on `wallInset`. The right
    // paddle mirrors this across the vertical midline: center-X = playfieldWidth - wallInset - halfWidth. Both share
    // the same Y placement (centered on the playfield).
    const float centerX = (side == PaddleSide::Left) ? wallInset + halfWidth : playfieldWidth - wallInset - halfWidth;
    const float centerY = playfieldHeight * 0.5f;
    return Paddle{centerX, centerY, halfWidth, halfHeight, speed};
}

SDL_FRect toFRect(const Paddle& paddle)
{
    SDL_FRect rect{};
    rect.x = paddle.centerX - paddle.halfWidth;
    rect.y = paddle.centerY - paddle.halfHeight;
    rect.w = paddle.halfWidth * 2.0f;
    rect.h = paddle.halfHeight * 2.0f;
    return rect;
}
