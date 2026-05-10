# Issues backlog

This file is a staging area for proposed GitHub issues. Each entry below maps to one pull request and is intended to be filed against the matching milestone on GitHub. Once an issue is filed, the corresponding entry can be removed from this file.

This file itself satisfies the _"Define the initial GitHub issue backlog"_ scope item in [`docs/ROADMAP.md`](ROADMAP.md).

Issues for later milestones will be added to this file in subsequent batches.

---

## Milestone: Repository, build scripts & CI

> All deliverables of this milestone are now implemented in the repository: core scaffolding (`README.md`, `LICENSE`, `.gitignore`, `docs/ROADMAP.md`), the minimal `CMakeLists.txt` + `src/main.cpp`, the build scripts (`scripts/build.sh`, `scripts/build.ps1`, `scripts/build.cmd`), the `doctest`/CTest unit-test scaffold (issue #1), the GitHub Actions CI workflow (issue #2), the editor formatting config `.editorconfig` + `.clang-format` (issue #3), and the per-task agent skills under `.claude/skills/` (issue #4). No backlog entries remain for this milestone.

---

## Milestone: SDL3 window and game loop

### Integrate SDL3 via `FetchContent` and link the executable

**Labels:** `build`, `ci`, `sdl3`

#### Description

Acquire SDL3 through CMake `FetchContent` (pinned to a release tag), reusing the same mechanism that was introduced for `doctest` in the previous milestone. Link the `pong-sdl3-cpp` executable against `SDL3::SDL3` and update CI so the Linux job has the system dev-headers needed to build SDL3 from source. This issue does not introduce any windowing or rendering code: it only proves the dependency can be acquired and linked on every supported platform (Windows MSVC, Ubuntu GCC or Clang, and macOS Apple Clang).

#### Tasks

- [ ] In `CMakeLists.txt`, declare `FetchContent_Declare(SDL3 GIT_REPOSITORY https://github.com/libsdl-org/SDL.git GIT_TAG <latest stable release-3.x.x tag>)` and call `FetchContent_MakeAvailable(SDL3)`. Pin to a specific release tag, never `main`.
- [ ] Pick a link mode by setting `SDL_SHARED`/`SDL_STATIC` cache variables before `FetchContent_MakeAvailable`. Suggested default: `SDL_SHARED=ON` (matches SDL3's upstream default; on Windows, copy `SDL3.dll` next to the executable via a CMake `add_custom_command(... POST_BUILD ...)`). Document the chosen mode.
- [ ] Link the `pong-sdl3-cpp` target against `SDL3::SDL3`.
- [ ] In `src/main.cpp`, include `<SDL3/SDL.h>` and call `SDL_GetVersion()` once on startup, printing the version. This proves linking works without exercising any subsystem.
- [ ] In `.github/workflows/build.yml`, on the Ubuntu runner, install the SDL3 transitive dev-headers before configuring CMake: `libx11-dev libxext-dev libxrandr-dev libwayland-dev libxkbcommon-dev libegl1-mesa-dev libgl1-mesa-dev libpulse-dev libasound2-dev`. Windows MSVC needs no extra packages. macOS needs no extra packages either: the Xcode Command Line Tools provide Apple Clang and SDL3 uses the system Cocoa/Metal/CoreAudio frameworks.
- [ ] Add a CMake cache (or step) so SDL3 sources fetched into `build/_deps/` are cached in CI keyed on the SDL3 tag, to avoid recompiling SDL3 on every run.
- [ ] Verify the `Dependencies` section of `README.md` matches the dev-header list installed in CI, and update if needed.

#### Acceptance criteria

- `cmake -S . -B build && cmake --build build` succeeds on Windows MSVC, on Ubuntu (GCC or Clang), and on macOS (Apple Clang).
- The resulting `pong-sdl3-cpp` executable starts, prints the SDL3 version to stdout, and exits with code 0.
- All three CI jobs (Windows, Ubuntu, macOS) are green; build logs show SDL3 being fetched and built once, then cached.
- `ctest --output-on-failure` still passes (the smoke test is unaffected).
- The SDL3 version is pinned to a specific release tag in `CMakeLists.txt`.

#### Notes

- The exact SDL3 tag should be agreed in the PR. As of writing, the latest is in the `release-3.2.x` series.

---

### Implement the SDL3 main loop inside an `Application` class

**Labels:** `sdl3`, `app`

**Depends on:** _Integrate SDL3 via FetchContent and link the executable_

#### Description

Build the SDL3 application around an `Application` class that owns the entire SDL3 lifetime: initialization, the window, the renderer, the event loop, and per-frame timing. `main.cpp` stays a thin entry point that instantiates the class and forwards its exit code, so subsequent milestones extend the class rather than touching `main.cpp` directly.

#### Tasks

- [ ] Add `src/Application.h` and `src/Application.cpp` defining a class `Application` with: a constructor taking a window title and size, a public `run()` returning an exit code, and (private) `init()`, `pollEvents()`, `update(double dtSeconds)`, `render()`, and `shutdown()` helpers.
- [ ] In `init()`, call `SDL_Init(SDL_INIT_VIDEO)`, create an `SDL_Window` (e.g. 800×600, resizable, titled `Pong SDL3 C++`), and create an `SDL_Renderer` for it. Request V-Sync via `SDL_SetRenderVSync(renderer, 1)`; if it fails, fall back to a manual 60 FPS cap using `SDL_DelayNS`.
- [ ] In `run()`, drive the main loop: poll events; exit cleanly on `SDL_EVENT_QUIT` and on `SDL_EVENT_KEY_DOWN` with `SDLK_ESCAPE` (placeholder until the menus milestone); compute a per-frame `dtSeconds` from `SDL_GetTicksNS` (or `SDL_GetPerformanceCounter`); call `update(dtSeconds)`, then `render()`.
- [ ] In `render()`, clear to black with `SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255)` + `SDL_RenderClear`, then `SDL_RenderPresent`.
- [ ] In `shutdown()` (or the destructor), destroy the renderer and the window in the right order, then call `SDL_Quit`. The destructor must be safe to call after a failed `init()` (no double-shutdown, no use-after-free on early exit paths).
- [ ] Reduce `src/main.cpp` to a thin entry point: instantiate `Application`, call `run()`, return its exit code. On any SDL3 failure, the class prints `SDL_GetError()` and `run()` returns a non-zero exit code.
- [ ] Wire `src/Application.cpp` into the `pong-sdl3-cpp` target in the top-level `CMakeLists.txt`.
- [ ] Document the V-Sync vs manual-cap behaviour in a short code comment near the frame-timing logic.

#### Acceptance criteria

- Running the executable opens a black window titled `Pong SDL3 C++`.
- Closing the window or pressing Escape exits the program with code 0.
- The window survives at least 5 seconds without crashing on Windows, Linux, and macOS.
- A per-frame `dtSeconds` is computed and passed into `update(double dtSeconds)`, ready to be consumed by gameplay code (even though `update()` is currently empty).
- `main.cpp` does not contain any direct SDL3 call other than instantiating `Application`.
- The `Application` class has a clean ownership model: no leaked `SDL_*` resources, no double-shutdown, no use-after-free on early exit paths (verifiable by reading the destructor and `init()` failure paths).
- CI is still green on Windows, Ubuntu, and macOS; `ctest` still passes (the existing smoke test is unaffected).

#### Notes

- This issue itself adds no unit tests: the `Application` class is almost entirely SDL plumbing, which is excluded from unit tests by the project's [testing strategy](ROADMAP.md#testing-strategy). The two pure helpers it relies on (tick-delta to seconds conversion, manual frame-cap arithmetic) are extracted and unit-tested in the separate _Inject a Clock abstraction…_ issue, which depends on this one.

---

### Extract the production source list into a shared CMake variable

**Labels:** `build`, `tests`, `refactor`

**Depends on:** _Implement the SDL3 main loop inside an Application class_

#### Description

Today, `tests/CMakeLists.txt` declares its own `add_executable(pong-sdl3-cpp-tests …)` that re-lists each source file it wants to test. As soon as `Paddle`, `Ball`, `Application` etc. exist, this forces every new production file to be added to two `CMakeLists.txt` files, and any divergence silently makes the test binary build a different set of sources than the shipped binary.

The lightest fix that scales: declare the production source list once as a CMake variable (`PONG_SRC`) at the top of the root `CMakeLists.txt`, and pass it to both `add_executable` calls. Adding a new production file then requires editing exactly one place. Per-target settings (`target_link_libraries`, `target_compile_options`) stay on each target individually — keep the two blocks visually adjacent in the file so a reviewer can spot drift in one glance.

This pattern is intentionally simpler than introducing a `pong-sdl3-cpp-lib` static library: for a project whose only deliverable is the executable, a separate library target adds graph complexity without a matching benefit. The trade-off is that each new transitive dependency (e.g. SDL3 in this very milestone) has to be linked into both targets manually, rather than once on a shared library — see Notes.

This issue lands the refactor _before_ the first non-trivial production source (paddles, ball, state machine) so the test issues that follow can simply add a `Test.cpp` file and let `${PONG_SRC}` carry the production code in.

#### Tasks

- [ ] In the top-level `CMakeLists.txt`, declare `set(PONG_SRC src/Application.cpp)` (the only non-`main.cpp` source at this point) before any `add_executable` call. Use absolute paths via `${CMAKE_CURRENT_SOURCE_DIR}/src/...` so the variable can be consumed unchanged from `tests/CMakeLists.txt`, which sits one directory deeper.
- [ ] Update `add_executable(pong-sdl3-cpp ...)` to read `add_executable(pong-sdl3-cpp ${PONG_SRC} src/main.cpp)`.
- [ ] In `tests/CMakeLists.txt`, update the test target to `add_executable(pong-sdl3-cpp-tests ${PONG_SRC} main.cpp SmokeTest.cpp)`. Test files can now `#include "Application.h"` (and any future production header) without re-listing the corresponding `.cpp`.
- [ ] Add a small `target_include_directories(pong-sdl3-cpp-tests PRIVATE ${CMAKE_SOURCE_DIR}/src)` (and the equivalent on the executable if needed) so test sources can include production headers without relative paths.
- [ ] Add one trivial test (e.g. `tests/ApplicationTest.cpp` containing a `TEST_CASE` that constructs an `Application` description struct or verifies a header-only constant) just to prove `${PONG_SRC}` is correctly compiled into the test binary.
- [ ] Update the `repo-conventions` skill (`.claude/skills/repo-conventions/SKILL.md`) to document the shared-variable pattern: production sources live in `${PONG_SRC}` in the root `CMakeLists.txt`; new files go there.

#### Acceptance criteria

- `cmake -S . -B build && cmake --build build` succeeds on Windows MSVC, Ubuntu, and macOS.
- The production source list exists once (as the `PONG_SRC` CMake variable) and is consumed by both `pong-sdl3-cpp` and `pong-sdl3-cpp-tests`.
- `pong-sdl3-cpp` still runs and behaves exactly as before.
- `ctest --output-on-failure` still passes; the new link-path test is registered and green.
- Adding a hypothetical `src/Foo.cpp` requires editing exactly one place (the `PONG_SRC` declaration).

#### Notes

- Trade-off: per-target dependencies and compile flags do _not_ propagate automatically. Each new transitive dependency (SDL3, future audio backend) must be linked into both targets explicitly, and warning options are duplicated between them. Mitigation: keep the two `target_link_libraries` / `target_compile_options` blocks adjacent in the file. If they ever grow large, extract a small helper macro `pong_apply_target_settings(<target>)`.
- A `pong-sdl3-cpp-lib` static-library refactor was considered and rejected at this project size. It would have auto-propagated the per-target settings via `PUBLIC`, but at the cost of an extra target and a more complex build graph for a project whose only deliverable is the executable. Re-evaluate if the per-target settings ever drift in practice or if a future milestone needs to ship a library.

---

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
