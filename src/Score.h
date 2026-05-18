#pragma once

#include <string>

// A player score paired with its cached decimal text form, kept in sync by setScore().
//
// Why bundle the int with its string: scoring code mutates the numeric value and the rendered text in lockstep, and
// the only way to keep that pairing safe is to route every write through one helper. Mutating the fields directly
// would let the cached text drift behind the numeric value -- a silent rendering bug. The smell list in the
// game-code-architecture skill flags any direct write to `value` or `text` on review.
//
// Why cache the text at all: render() draws the score every frame; recomputing std::to_string(value) per frame would
// be a recurring per-frame allocation (see the frame-loop rule in the game-code-architecture skill). The default text
// "0" sits well within std::string's small-string optimisation on every mainstream stdlib, so default construction is
// heap-free; setScore allocates pessimistically but SSO covers any int up to ~15 chars at the Pong scale, so the
// readout never actually touches the heap in practice.
struct Score
{
    int value = 0;
    std::string text{"0"};
};

// Invariant-preserving setter: writes both fields in one step so `text` always equals std::to_string(`value`). Tests
// pin this contract in tests/ScoreTest.cpp.
void setScore(Score& score, int newValue);
