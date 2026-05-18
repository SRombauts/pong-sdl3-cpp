#include "Score.h"

#include <doctest/doctest.h>

TEST_CASE("Score: default-constructed matches the documented value=0, text=\"0\" invariant")
{
    Score score;
    CHECK(score.value == 0);
    CHECK(score.text == "0");
}

TEST_CASE("setScore: single-digit assignment refreshes both fields in lockstep")
{
    Score score;
    setScore(score, 5);
    CHECK(score.value == 5);
    CHECK(score.text == "5");
}

TEST_CASE("setScore: two-digit assignment renders both digits without padding")
{
    Score score;
    setScore(score, 11);
    CHECK(score.value == 11);
    CHECK(score.text == "11");
}

TEST_CASE("setScore: reset path returns text to the default \"0\"")
{
    Score score;
    setScore(score, 7);
    setScore(score, 0);
    CHECK(score.value == 0);
    CHECK(score.text == "0");
}

TEST_CASE("setScore: assigning the current value is idempotent")
{
    Score score;
    setScore(score, 4);
    setScore(score, 4);
    CHECK(score.value == 4);
    CHECK(score.text == "4");
}

TEST_CASE("setScore: negative values render with the leading minus sign (defensive case)")
{
    // Scoring code never sets a negative value today, but std::to_string handles it the way the standard mandates.
    // Pinning the behavior here catches accidental sign drift if Score ever gets reused outside Pong's
    // 0..kScoreWinningPoints range (e.g. a future "lives remaining" or "overtime countdown" use).
    Score score;
    setScore(score, -3);
    CHECK(score.value == -3);
    CHECK(score.text == "-3");
}
