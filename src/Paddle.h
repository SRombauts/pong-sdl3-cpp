#pragma once

struct SDL_FRect;

// Player- (or AI-) controlled paddle. Stores center, half-extents, and per-paddle speed cap.
//
// Position convention: (centerX, centerY) is the geometric center of the rectangle, not its top-left corner. The AABB
// collision math added by the Ball-and-collisions milestone reads center-vs-center, so storing the center directly
// avoids scattering `+ halfHeight` / `- halfHeight` arithmetic at every call site. SDL_RenderFillRect takes a top-left
// rect, so the SDL boundary -- one conversion in Paddle.cpp -- handles the translation.
//
// `speed` is stored per Paddle rather than as a global constant so the Analog-and-gamepad and One-player-AI milestones
// can give different paddles different caps (e.g. an Easy AI moves slower than the player) without retrofitting the
// data structure.
struct Paddle
{
    float centerX = 0.0f;
    float centerY = 0.0f;
    float halfWidth = 0.0f;
    float halfHeight = 0.0f;
    float speed = 0.0f;
};

// Which wall a paddle hugs. Used by makePaddle to pick the X coordinate (the Y math is identical for both sides --
// vertically centered on the playfield) so the placement rule has a single implementation.
enum class PaddleSide
{
    Left,
    Right,
};

// Build a paddle parked against `side`'s wall, vertically centered, with its outer edge `wallInset` pixels from the
// wall. Pure and tested -- this is the placement math that used to live in PlayfieldLayout::{left,right}Paddle, now
// returning a `Paddle` directly so Application can hold the entity by value and the renderer reads `toFRect(paddle)`.
[[nodiscard]] Paddle makePaddle(PaddleSide side,
                                float playfieldWidth,
                                float playfieldHeight,
                                float halfWidth,
                                float halfHeight,
                                float wallInset,
                                float speed);

// Convert a Paddle to the top-left-anchored SDL_FRect the renderer wants. Lives in Paddle.cpp so this header stays
// SDL-free and consumers that only need the struct (e.g. the pure motion helpers, their tests) don't drag in SDL.
[[nodiscard]] SDL_FRect toFRect(const Paddle& paddle);
