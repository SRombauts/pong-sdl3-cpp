# Issues backlog

This file is a staging area for proposed GitHub issues. Each entry below maps to one pull request and is intended to be filed against the matching milestone on GitHub. Once an issue is filed, the corresponding entry can be removed from this file.

This file itself satisfies the _"Define the initial GitHub issue backlog"_ scope item in [`docs/ROADMAP.md`](ROADMAP.md).

Issues for later milestones will be added to this file in subsequent batches.

---

## Milestone: Repository, build scripts & CI

> All deliverables of this milestone are now implemented in the repository: core scaffolding (`README.md`, `LICENSE`, `.gitignore`, `docs/ROADMAP.md`), the minimal `CMakeLists.txt` + `src/main.cpp`, the build scripts (`scripts/build.sh`, `scripts/build.ps1`, `scripts/build.cmd`), the `doctest`/CTest unit-test scaffold (issue #1), the GitHub Actions CI workflow (issue #2), the editor formatting config `.editorconfig` + `.clang-format` (issue #3), and the per-task agent skills under `.claude/skills/` (issue #4). No backlog entries remain for this milestone.

---

## Milestone: SDL3 window and game loop

### Inject a `Clock` abstraction into `Application` and add unit tests for the frame-timing helpers

**Labels:** `app`, `tests`, `tdd`

**Depends on:** _Implement the SDL3 main loop inside an Application class_, _Extract the production source list into a shared CMake variable_

#### Description

`Application::run()` currently reads time directly from `SDL_GetTicksNS` (or equivalent) and computes `dtSeconds` inline. That makes anything time-dependent (paddle motion, ball motion, AI tracking, FPS smoothing) impossible to drive from a unit test without faking SDL itself.

Introduce a small `Clock` interface with a default SDL-backed implementation and a test-friendly fake. Move tick-delta-to-seconds conversion and the manual frame-cap arithmetic into pure free functions that take their inputs as parameters. Cover both helpers with `TEST_CASE`s, written test-first per the project's [testing strategy](ROADMAP.md#testing-strategy).

#### Tasks

- [ ] Add `src/Clock.h` defining an `IClock` interface (or `Clock` concept / function-like type — pick one and document the choice) with a single `now()` method returning a monotonic timestamp in nanoseconds.
- [ ] Add `src/SdlTicksClock.{h,cpp}` implementing `IClock` over `SDL_GetTicksNS`. Keep it tiny; it is the only place in the codebase that calls `SDL_GetTicksNS`. Append it to the `PONG_SRC` variable so both targets pick it up.
- [ ] Add `src/FakeClock.{h,cpp}` (also in `PONG_SRC` so tests can construct it without extra plumbing) returning scripted values via `advance(SDL_Uint64 ns)` / `setNow(SDL_Uint64 ns)` helpers.
- [ ] Modify `Application` to take an `IClock&` (or owning `std::unique_ptr<IClock>`) via its constructor; default-construct an `SdlTicksClock` for production callers. `Application::run()` reads time only through the injected clock.
- [ ] Add `src/FrameTiming.{h,cpp}` (or inline in `Application.cpp` if preferred — but a separate header keeps the test surface small) and append to `PONG_SRC`. Two pure free functions:
  - `double secondsBetween(std::uint64_t prevNs, std::uint64_t nowNs)` — returns the elapsed time in seconds, handling the `now == prev` case (returns `0.0`) and a defensive `now < prev` case (returns `0.0`, never negative).
  - `std::uint64_t frameCapDelayNs(std::uint64_t elapsedNs, double targetFps)` — returns the sleep duration to reach `1 / targetFps`, clamped to `0` when `elapsedNs` already exceeds the budget.
- [ ] Wire both helpers into `Application::run()` so the V-Sync-fallback manual cap path uses `frameCapDelayNs`.
- [ ] Add `tests/FrameTimingTest.cpp` with `TEST_CASE`s for both helpers, written test-first:
  - `secondsBetween`: zero delta, one-second delta, sub-millisecond delta, wrap-around / `now < prev` defensive case.
  - `frameCapDelayNs`: under-budget frame returns the remaining time, exactly-on-budget returns `0`, over-budget returns `0`, fractional FPS values (e.g. `59.94`) round correctly.
- [ ] (Optional but encouraged) Extend the `tests/ApplicationTest.cpp` introduced by the lib-refactor issue with one integration-style `TEST_CASE` that constructs an `Application` with a `FakeClock`, advances time by a scripted sequence, and verifies the resulting `dtSeconds` sequence — without ever touching SDL video.

#### Acceptance criteria

- `Application` no longer calls `SDL_GetTicksNS` directly; the only place that does is `SdlTicksClock`.
- `pong-sdl3-cpp-tests` includes `tests/FrameTimingTest.cpp` and all new `TEST_CASE`s pass locally and in CI.
- Behaviour of the shipped executable is unchanged: V-Sync still preferred; manual cap still kicks in when V-Sync is unavailable.
- Each new `TEST_CASE` was demonstrably written before the corresponding production code (TDD): the PR description shows the failing-test-first commit at least for `secondsBetween` and `frameCapDelayNs`.

#### Notes

- Keep the `IClock` interface minimal (one method). Adding `sleep()` or `nowSeconds()` is tempting but expands the test-doubles surface for no current win.
- The TDD evidence in the PR description does not need to be elaborate: linking the commit that adds the failing test, then the commit that makes it pass, is enough.

---

### Inject a seedable `RandomSource` abstraction into `Application`

**Labels:** `app`, `tests`, `infrastructure`

**Depends on:** _Implement the SDL3 main loop inside an Application class_, _Extract the production source list into a shared CMake variable_

#### Description

The **Ball and collisions** milestone serves the ball with a small random vertical angle, and the **One-player AI** milestone introduces bounded prediction error. Both require a seedable random source so tests can pin the outcome. Like the `Clock`, we introduce the abstraction now — before any consumer exists — to avoid retrofitting random calls scattered across gameplay code later.

This issue is pure infrastructure: it adds the abstraction, wires it into `Application`, and ships a smoke test that the abstraction is deterministic under a fixed seed. Real consumers (serve angle, AI noise) land in their own milestone issues.

#### Tasks

- [ ] Add `src/RandomSource.h` defining a small interface (`IRandomSource`) with the operations gameplay actually needs: `int intInRange(int lo, int hi)` (inclusive bounds) and `double doubleInRange(double lo, double hi)` (half-open `[lo, hi)`). Keep the surface intentionally small; add operations only when a real consumer needs them.
- [ ] Add `src/Mt19937RandomSource.{h,cpp}` implementing `IRandomSource` over `std::mt19937` with a constructor that takes an explicit `std::uint64_t seed`. Provide a free helper `Mt19937RandomSource makeNonDeterministicRandomSource()` that seeds from `std::random_device` for production use. Append to `PONG_SRC` so both targets pick it up.
- [ ] Modify `Application` to take an `IRandomSource&` (or owning `std::unique_ptr<IRandomSource>`) via its constructor; default-construct a non-deterministically-seeded `Mt19937RandomSource` for production callers. The reference is exposed to gameplay code via an accessor (e.g. `Application::random()`) for now; the **Ball and collisions** milestone will tighten this when it introduces the serve policy.
- [ ] Add `tests/RandomSourceTest.cpp` with `TEST_CASE`s verifying:
  - `Mt19937RandomSource` with seed `S` produces the same sequence on two independent instances (determinism under fixed seed).
  - `intInRange` respects inclusive bounds across many samples.
  - `doubleInRange` respects half-open bounds across many samples.
  - The non-deterministic helper produces _different_ sequences across two calls (statistical, not strict — acceptable to compare just the first few values).

#### Acceptance criteria

- `IRandomSource`, `Mt19937RandomSource`, and `makeNonDeterministicRandomSource()` are part of `${PONG_SRC}` and therefore available to both the executable and the test binary.
- `Application` carries an injected `IRandomSource&` available to subsequent milestones.
- The shipped executable behaves identically to before (the abstraction has no production consumer yet).
- The new `tests/RandomSourceTest.cpp` `TEST_CASE`s pass locally and in CI.

#### Notes

- Trade-off: this is mild YAGNI — we add infrastructure with no immediate caller. Accepted because the alternative is a churn-y refactor of the **Ball and collisions** PR, where it would land mixed with the actual gameplay diff.
- We deliberately do _not_ expose a generic `next()` returning a raw `uint32_t`. Gameplay code should call typed range helpers; that keeps the call sites readable and the test fakes simple.
