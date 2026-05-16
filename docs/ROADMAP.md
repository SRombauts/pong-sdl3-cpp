# Roadmap

This roadmap is intentionally incremental. The project starts with a single-file standard C++ hello world and evolves toward a complete Pong clone using SDL3.

The first milestone deliberately avoids SDL3. This keeps the repository, build system, scripts, and CI simple before introducing external dependencies.

## Testing strategy

Tests are a first-class deliverable, not a per-milestone afterthought. The same bar applies to every milestone below.

- **Every pure-logic unit ships with at least one `TEST_CASE`.** This covers math (collisions, reflection angles, dead-zones, frame-time arithmetic), state transitions (game-state machine, menu cursor, controller-assignment policy), and layout helpers (dashed-line segments, default starting positions, score-digit segments). Test code lands in the same pull request as the production code it covers.
- **TDD is the preferred workflow for new pure-logic helpers.** Write the failing `TEST_CASE` first, implement until it passes, then refactor. This applies most cleanly to small free functions (clamping, normalization, AABB intersection) and to state-transition tables (one row of the table = one `SUBCASE`).
- **SDL-touching code is excluded from unit tests.** Window/renderer creation, the SDL3 event pump, real gamepad hot-plug, and audio playback are validated by each milestone's manual smoke test ("opens a window, exits cleanly", "rally the ball without crashing", etc.). To keep this exclusion small, SDL access is concentrated inside `Application` and a handful of thin device adapters; everything else stays pure and testable.
- **Time and randomness are injected, never read directly from globals.** `Application` owns a `Clock` (default `ClockSdlTicks`, test `ClockFake`) and a seedable `RandomSource`. Gameplay code reads `dtSeconds` and random values through these abstractions so tests can drive the simulation deterministically. Both abstractions are introduced in the SDL3 milestone, before they have any consumer, to avoid retrofitting them later.
- **Production source files live in a single CMake variable** (e.g. `PONG_SRC`) that both the executable target `pong-sdl3-cpp` and the test target `pong-sdl3-cpp-tests` consume. Adding a new `Foo.cpp` only requires editing the variable in one place; both targets pick it up automatically. The variable is introduced in the SDL3 milestone, before the first non-trivial production source lands. Per-target dependencies (SDL3, `doctest`) and compile flags are still set on each target individually — keep the two `target_link_libraries`/`target_compile_options` blocks visually adjacent in `CMakeLists.txt` so a reviewer can spot drift in one glance.

Each milestone below carries its own concrete test list under "Scope". Acceptance criteria explicitly require those tests to pass locally and in CI.

## Repository, build scripts & CI

Goal: establish a usable repository baseline and make local and remote builds repeatable before introducing SDL3.

Scope:

- Create the GitHub repository.
- Add a README.
- Add a `LICENSE`.
- Add a `.gitignore`.
- Add a minimal `CMakeLists.txt`.
- Add a single `src/main.cpp` hello world.
- Add `docs/ROADMAP.md`.
- Define the initial GitHub issue backlog.
- Keep the produced executable dependency-free at runtime. The only build-time dependencies are the C++ toolchain, CMake, and the unit-test framework introduced below.
- Add a basic Unix-like build script, for example `scripts/build.sh`.
- Add a basic Windows PowerShell build script, for example `scripts/build.ps1`.
- Optionally add a thin Windows Command Prompt wrapper, for example `scripts/build.cmd`.
- Use out-of-source CMake builds, preferably under `build/`.
- Support at least a default Debug build, with `Release` available as a configuration argument.
- Acquire `doctest` through CMake `FetchContent`, pinned to a specific release tag, so the project stays self-contained and reproducible without a system-wide install or a separate package manager. The same `FetchContent` mechanism is reused for SDL3 in the next milestone.
- Add a unit-test target under `tests/`, gated behind CMake's standard `BUILD_TESTING` option (set by `include(CTest)`, defaulted ON for developers), and wire it into CTest.
- Add a single smoke test (for example asserting `1 + 1 == 2`) so the test target actually compiles and `ctest` actually runs. Real tests are added in later milestones.
- Add a GitHub Actions workflow under `.github/workflows/build.yml` that, on every push and pull request to `main`, builds the project and runs `ctest --output-on-failure`, on at least Windows latest (MSVC), Ubuntu latest (GCC or Clang), and macOS latest (Apple Clang).
- Keep scripts thin: they should call CMake, not become a custom build system.
- Add a `.editorconfig` so editors agree on indentation, line endings, and trailing newlines.
- Add a `.clang-format` describing the project's formatting style.
- Add agent skills under `.claude/skills/` (one folder per skill, each with a `SKILL.md` carrying YAML frontmatter) documenting common tasks for AI agents and human contributors: build, test, format, repository conventions.

Non-goals:

- Do not add SDL3 yet (deferred to the next milestone).
- Do not add an application framework yet.
- Do not add gameplay code yet.
- Do not package release artifacts yet.
- Do not add code coverage yet.
- Do not add static analysis unless it stays trivial.

Acceptance criteria:

- The repository can be cloned.
- The project configures with CMake.
- The project builds a minimal standard C++ executable.
- The executable prints a simple hello-world message.
- A developer can build locally using one command.
- The unit-test target builds and `ctest` passes locally and in CI, even before any real tests exist.
- GitHub Actions builds the project and runs the test suite on every push and pull request, on at least Windows, Linux, and macOS.
- CI status is visible on pull requests.
- The workflow remains simple enough to extend when SDL3 is introduced.
- No SDL3 dependency is required yet.

## SDL3 window and game loop

Goal: introduce SDL3 and open a minimal window.

Scope:

- Acquire SDL3 through CMake `FetchContent` (pinned to a specific release tag), reusing the same mechanism that was introduced for `doctest` in the previous milestone.
- Link against the `SDL3::SDL3` target.
- Update GitHub Actions for Linux to install SDL3's transitive system dev-headers (X11/Wayland/OpenGL/EGL/PulseAudio/ALSA), since `FetchContent` builds SDL3 from source. Windows MSVC does not need extra packages.
- Update local build scripts only if needed.
- Implement an `Application` class (`src/Application.{h,cpp}`) that owns the SDL3 lifetime; keep `main.cpp` a thin entry point that instantiates the class and forwards its exit code. Subsequent milestones extend the class instead of editing `main.cpp` directly.
- Inside the class, initialize and shut down SDL3 cleanly.
- Create a window.
- Create a renderer.
- Drive an event loop that handles `SDL_EVENT_QUIT` cleanly.
- Cap the frame rate using V-Sync (via `SDL_SetRenderVSync`) and expose a per-frame delta time value for later milestones. A manual `SDL_DelayNS`-based fallback is intentionally deferred: V-Sync covers the realistic case on every CI platform, the loop runs uncapped on V-Sync failure (logged as non-fatal), and a software cap can be added later if a real use case (headless CI, broken vsync) appears.
- Clear and present a black frame each tick.
- Move the production source list into a single CMake variable (e.g. `PONG_SRC`) declared near the top of `CMakeLists.txt`, and pass it to both `add_executable(pong-sdl3-cpp ${PONG_SRC} src/main.cpp)` and `add_executable(pong-sdl3-cpp-tests ${PONG_SRC} ...)` (the latter declared in `tests/CMakeLists.txt`). Adding a new production file then requires editing exactly one place. Per-target settings (SDL3 link, `doctest` link, warning flags) stay on the individual targets; keep the two `target_*` blocks adjacent so divergence is visible to reviewers.
- Inject a `Clock` abstraction into `Application`: a tiny interface with one virtual `now()` returning a monotonic timestamp. Provide `ClockSdlTicks` (wraps `SDL_GetTicksNS`) as the default and `ClockFake` (returns scripted values) for tests. Move tick-delta-to-seconds conversion into a pure free function that takes its inputs as parameters, so it has no SDL dependency.
- Inject a seedable `RandomSource` abstraction into `Application` for use by later milestones (serve direction in **Ball and collisions**, AI noise in **One-player AI**). Production seeds it from a non-deterministic source at startup; tests pass a fixed seed. The abstraction is introduced now even without a consumer, to avoid a churn-y retrofit later.
- Add unit tests for the pure helpers introduced by this milestone (TDD-friendly: write the `TEST_CASE` first):
  - Tick-delta to `dtSeconds` conversion (including the wrap-around / zero-delta edge cases).

Acceptance criteria:

- The app opens a window.
- The app exits cleanly.
- The app has a stable main loop.
- A per-frame delta time is computed and ready to be consumed by gameplay code.
- Local scripts and CI still build successfully and `ctest` still passes.
- The production source list exists once (as a CMake variable) and is consumed by both `pong-sdl3-cpp` and `pong-sdl3-cpp-tests`; adding a hypothetical `src/Foo.cpp` requires editing exactly one place.
- The new unit tests for the frame-timing helpers pass locally and in CI.

## Static playfield

Goal: make the program visually recognizable as Pong.

Scope:

- Pick a fixed logical resolution (for example 800x600) and integrate `SDL_SetRenderLogicalPresentation` so the game scales to any window size.
- Document the coordinate convention (origin at top-left, X to the right, Y downward, units in logical pixels).
- Pick and document a text-rendering approach for the placeholder score (hand-drawn 7-segment digits, a bitmap font, or `SDL_ttf`). The same approach is expected to be reused by the menus milestone.
- Draw black background.
- Draw center dashed line.
- Draw static paddles.
- Draw static ball.
- Draw placeholder score using the chosen text-rendering approach.
- Add unit tests for the pure layout logic of this milestone (TDD-friendly: each helper is a small function with no SDL dependency):
  - Dashed center-line segment computation: given playfield height, segment count, and gap, return the list of segment rectangles. Verify total covered length, gap respected, first/last segment positions.
  - Default starting positions for both paddles and the ball as a function of the logical playfield dimensions and entity half-sizes (paddles centred vertically and inset from the side walls; ball centred).
  - If the chosen score rendering uses 7-segment digits: the segment-on/off pattern for `0..9` and the per-segment rectangle layout for a digit drawn at a given top-left position.

Non-goals:

- Do not add gameplay logic yet.

Acceptance criteria:

- The window shows a black background, two static paddles, a static ball, a center dashed line, and a placeholder score, all positioned in a classic Pong layout.
- The window can be resized without distorting the playfield aspect ratio.
- The new unit tests for the layout helpers pass locally and in CI.

## Paddle controls

Goal: make the two paddles controllable, with an input layer that other devices (mouse, gamepad, AI) can plug into in later milestones.

Scope:

- Introduce a `Paddle` struct with position, half-size, and speed; replace the hard-coded paddle rectangles from the previous milestone by reading from these structs.
- Introduce a small `PaddleController` abstraction (one instance per paddle) so the paddle's update step does not depend directly on a specific input device. A controller returns a per-tick request expressed both as a desired axis value in `[-1, +1]` (used by digital and stick-as-velocity inputs) and an optional target Y (used by mouse and AI). The paddle moves toward the target while remaining clamped by the paddle's speed cap, so no controller can teleport the paddle.
- Implement `KeyboardPaddleController` as the first concrete controller. Map W and Z to "up" / S to "down" for the left paddle, and Up / Down arrows for the right paddle. The controller reads its key state through a small adapter (e.g. a `KeyboardState` struct populated from `SDL_GetKeyboardState`), not directly from SDL, so tests can drive it with a fake state snapshot.
- Add keyboard input state plumbing.
- Move paddles using a variable timestep driven by the per-frame delta time.
- Clamp paddles inside the playfield.
- Add unit tests for the pure logic of this milestone (TDD-friendly):
  - Paddle clamping to playfield bounds: top edge, bottom edge, no-op interior, paddle larger than the playfield as a defensive case.
  - Paddle update step: given a `PaddleController` request, the current position, the speed cap, and `dt`, return the new position. Verifies that the speed cap always wins over a far target Y (no teleport), and that motion is framerate-independent (one large `dt` and many small `dt`s of equal sum produce the same total displacement).
  - Axis-vs-target-Y precedence inside the paddle update step (which input wins when both are present, and why); the precedence is documented and tested.
  - `KeyboardPaddleController` against a fake `KeyboardState`: W/Z held → axis = -1, S held → axis = +1, both held → axis = 0, neither held → axis = 0; same matrix for the right paddle's arrow keys.

Non-goals:

- Mouse, gamepad, and single-gamepad couch co-op analog inputs (covered by the **Analog and gamepad controls** milestone).
- AI-controlled paddles (covered by the **One-player AI** milestone, which adds an `AiPaddleController` that plugs into the same abstraction).

Acceptance criteria:

- Both local players can move their paddles with the keyboard.
- Movement is framerate independent.
- The paddle update code reads input only through `PaddleController`; it does not call SDL3 keyboard / mouse / gamepad APIs directly.
- The new unit tests for paddle clamping, the update step, axis-vs-target precedence, and `KeyboardPaddleController` pass locally and in CI.

## Analog and gamepad controls

Goal: bring back the precise analog feel of the original Pong potentiometer paddles, and add modern gamepad support, including the single-gamepad couch co-op mode where one gamepad's two analog sticks drive both players.

Scope:

- Reuse the `PaddleController` abstraction introduced in the previous milestone; this milestone only adds new concrete implementations and the device-selection plumbing.
- Add a `MousePaddleController` that maps the mouse's window-Y position to a target Y in playfield coordinates (using SDL3's renderer logical-presentation conversion). Hide the system cursor while this controller is active.
- Initialize the SDL3 gamepad subsystem (`SDL_INIT_GAMEPAD`). Open and close gamepads in response to `SDL_EVENT_GAMEPAD_ADDED` / `SDL_EVENT_GAMEPAD_REMOVED` so hot-plugging works without a restart.
- Read analog stick values either from `SDL_EVENT_GAMEPAD_AXIS_MOTION` events or from `SDL_GetGamepadAxis` each tick, normalize to `[-1, +1]`, and apply a small dead-zone (typically around 10 % of the axis range) so resting sticks do not drift.
- Implement two gamepad-driven controllers:
  - `GamepadStickPaddleController`: one stick (left stick by default) of one gamepad drives one paddle. Used when each player has their own gamepad.
  - `GamepadSharedPaddleController`: one gamepad drives both paddles, left stick → P1, right stick → P2 (the explicit single-gamepad couch co-op mode).
- Add a controller-assignment policy that picks default controllers based on the connected devices: zero gamepads → keyboard for both players (as in the previous milestone), one gamepad → shared-controller mode, two or more gamepads → first two gamepads with one stick each (P1 → first gamepad, P2 → second gamepad). Re-run the policy on hot-plug.
- Add a placeholder runtime toggle (e.g. `M`) that hands the left paddle to `MousePaddleController`, and toggles back to whatever controller the assignment policy chose. The menus milestone will replace this with a proper picker.
- Add unit tests for the pure logic of this milestone:
  - Stick dead-zone normalization (raw axis input → normalized `[-1, +1]` output, including the dead-zone cutoff).
  - Mouse window-Y to logical playfield-Y mapping (independent of any SDL3 call; pass the window and logical sizes as parameters).
  - Controller-assignment policy as a function of the number of connected gamepads.

Non-goals:

- Custom per-key / per-button rebinding UI (deferred to **Screens and menus** or post-MVP).
- Force feedback / rumble (post-MVP nice-to-have).
- Per-axis sensitivity exposed through a settings menu (hardcoded constants for now).
- Touch / accelerometer / extended-input support.

Acceptance criteria:

- The keyboard path from the previous milestone still works exactly as before (no regression).
- Plugging in a single gamepad makes both analog sticks of that gamepad drive the two paddles, without restarting the application.
- Plugging in a second gamepad reassigns each player to one full gamepad, without restarting.
- Mouse mode visibly improves precision over keyboard for the left paddle.
- Hot-plugging gamepads (connect or disconnect at runtime) is handled cleanly: no crash, no stuck binding, the assignment policy re-runs.
- The new unit tests pass locally and in CI.

Notes:

- Use the SDL3 gamepad API (`SDL_Gamepad`, `SDL_GetGamepadAxis`, `SDL_EVENT_GAMEPAD_*`) rather than the lower-level joystick API; the gamepad layer abstracts over device-specific quirks.
- CI runners typically have no physical gamepad attached. Tests for this milestone must therefore not require a real gamepad; they cover only the device-independent logic. End-to-end gamepad behaviour is a manual smoke test.

## Ball and collisions

Goal: make the core game loop playable.

Scope:

- Introduce a `Ball` struct with position, half-size, and velocity; replace the hard-coded ball rectangle from the static playfield milestone by reading from this struct.
- Move the ball using delta time.
- Bounce on top and bottom walls.
- Detect paddle collisions using AABB intersection.
- Reflect the ball based on the vertical offset of the hit on the paddle, so the bounce angle varies within a bounded range (for example up to ±60° from horizontal).
- On hit, push the ball outside the paddle before applying the new velocity, to prevent it from sticking.
- Define and document a serve policy when the ball is reset (for example: serve toward the player who just lost the point, with a small random vertical angle). The random vertical angle is drawn from the `RandomSource` introduced in the SDL3 milestone, so the serve policy is deterministic under a fixed seed in tests.
- Increase ball speed on each paddle hit, with a maximum speed cap, so rallies become harder over time.
- Add unit tests covering the pure logic of this milestone (TDD-friendly: each piece is a small free function with no SDL dependency):
  - Top/bottom wall bounce: given a ball position, half-size, vertical velocity, and the playfield height, return the corrected position and the (possibly flipped) vertical velocity. Cover above-the-top, below-the-bottom, exactly-on-edge, and interior cases.
  - AABB collision detection between the ball and a paddle (4 floats per box, bool out), including the touching-edge case.
  - Reflection-angle computation: given the normalized hit offset on the paddle (`-1` at the top edge, `+1` at the bottom) and the incoming velocity, return the outgoing velocity. Verifies the bounce angle stays inside the documented ±60° envelope.
  - Anti-stick correction: after a paddle collision, the ball is repositioned outside the paddle along the collision normal before the new velocity is applied (no overlap remains on the next frame).
  - Speed-up curve: ball speed after `N` paddle hits matches the documented progression and saturates at the configured maximum-speed cap.
  - Serve-direction policy: under a fixed `RandomSource` seed, the serve direction goes toward the side that just lost the point and the random vertical angle stays inside the documented bounds.

Acceptance criteria:

- Players can rally the ball.
- The ball behaves predictably and does not stick to paddles or walls.
- The new unit tests pass locally and in CI.

## Scoring and match flow

Goal: complete the two-player MVP.

Scope:

- Detect when the ball leaves the playfield.
- Award points.
- Reset the round after each point.
- Replace the placeholder score from the previous milestone with the real, updating score.
- Detect the winning score (target: first to 11 points, exposed as a tunable constant).
- Reset the match.
- Add unit tests for the pure logic of this milestone (TDD-friendly):
  - Out-of-bounds detection on the left and right edges of the playfield (cover exactly-on-edge as well as past-the-edge).
  - Score increment for the correct side as a function of the side the ball left from.
  - Win detection at the configured target score, parameterized over several target values (e.g. first-to-3, first-to-11, first-to-21) so the same logic is exercised at multiple scales.
  - Round-reset state: after a point, paddles return to their starting Y and the ball is re-served according to the serve policy.
  - Match state machine: `(score_left, score_right, ball_out_event) → next_state` (continue rally, increment + reset, declare winner). One `SUBCASE` per row of the table.

Acceptance criteria:

- A full two-player Pong match can be played from start to finish.
- The new unit tests pass locally and in CI.

## Screens and menus

Goal: turn the prototype into a structured game.

Scope:

- Introduce an explicit game-state machine (for example Title, ModeSelect, Playing, Paused, GameOver) that owns transitions between screens.
- Move the existing gameplay code behind the Playing state so menu logic and gameplay logic stay separated.
- Add menu navigation bindings: Up/Down to change selection, Enter to confirm, Escape to go back or pause.
- Add title screen.
- Add mode selection screen (1P vs 2P). May be stubbed until the one-player AI milestone lands.
- Add pause state, toggled with Escape during gameplay.
- Add game-over screen showing the winner.
- Add restart and quit flows:
  - From the pause overlay: resume returns to gameplay; restart returns to the title screen; quit exits the application.
  - From the game-over screen: restart returns to the title screen; quit exits the application.
- Add unit tests for the pure logic of this milestone (TDD-friendly: state-machine work is the cleanest TDD shape in the project):
  - State-machine transition table: `(currentState, inputEvent) → nextState`. One `SUBCASE` per documented transition (Title → ModeSelect on Enter, ModeSelect → Playing on Enter, Playing ↔ Paused on Escape, Playing → GameOver on win condition, GameOver → Title on Enter, Paused → Title on restart, etc.). Unhandled inputs leave the state unchanged.
  - Menu cursor navigation: `(currentIndex, itemCount, direction, wrapEnabled) → newIndex`, including wrap-around at both ends and the no-wrap variant.

Acceptance criteria:

- The game has clear navigation states driven by the state machine.
- Gameplay code is not mixed with menu logic.
- From a fresh launch, the user can reach Title → ModeSelect → Playing → Paused → Playing → GameOver → Title using only keyboard input.
- The new unit tests for the state-machine transitions and menu cursor pass locally and in CI.

## One-player AI

Goal: add a 1-player extension against an AI opponent.

Scope:

- Add an `AiPaddleController` that implements the `PaddleController` abstraction by computing a target Y from the ball's vertical position. It plugs into a paddle the same way as the keyboard, mouse, or gamepad controllers from earlier milestones.
- Wire the AI controller to the right paddle in 1-player mode (the left paddle stays player-controlled).
- Limit AI paddle speed (must use the same speed cap as the player); since the paddle's `PaddleController`-driven update already clamps motion to the speed cap, the AI cannot teleport.
- Add at least one of: a reaction delay before the AI starts tracking, or a bounded prediction error on the targeted Y position. Both are driven by the `Clock` and `RandomSource` injected into `Application` in the SDL3 milestone, so tests can advance time and seed randomness deterministically.
- Expose difficulty levels (for example Easy / Normal / Hard) wired to the mode selection screen, tuning speed and/or error.
- Add unit tests for the AI controller (TDD-friendly):
  - Target-Y selection: with reaction delay = 0 and prediction error = 0, the AI's target Y matches the ball's Y.
  - Speed-cap enforcement: even with the target arbitrarily far, the AI paddle moves no faster than the player paddle's speed cap (validated via the same paddle update step tests use elsewhere).
  - Bounded prediction error: with a fixed `RandomSource` seed, the per-frame error stays inside the documented `[-maxError, +maxError]` envelope across many samples.
  - Reaction-delay model: with a delay of `D` seconds, the AI's target Y at time `t` matches the ball's Y at time `t - D` (cover `t < D` start-of-rally case as well).
  - Difficulty-preset mapping: `Difficulty → (speedFactor, maxError, reactionDelay)` returns the documented values for each level.

Acceptance criteria:

- One-player mode is playable.
- The AI is beatable and not perfectly reactive.
- The new unit tests pass locally and in CI.

## Audio and polish

Goal: make the project presentable.

Scope:

- Pick an audio backend (SDL3 native audio or `SDL3_mixer`) and integrate it through CMake (via `FetchContent`, like SDL3) and CI.
- Add minimal arcade-style sound effects: wall bounce, paddle hit, score, and match end.
- Add a debug overlay (toggleable) showing at least FPS and ball/paddle state. The overlay text is built by a pure formatting function so it is unit-testable.
- Add screenshots or a short GIF to the `README.md`.
- Add small visual polish if useful (for example a brief flash when a point is scored).
- Add unit tests for the pure logic added by this milestone:
  - Debug-overlay text formatting: given an FPS value, ball state, and paddle states, return the expected display string.
  - FPS smoothing: a moving average (or equivalent) over the last `N` frame durations returns the documented value for representative input sequences (constant frame time, sudden spike, warm-up before `N` samples are available).

Acceptance criteria:

- Sound effects play on wall bounce, paddle hit, score, and match end.
- The debug overlay can be toggled at runtime.
- The `README.md` contains at least one screenshot or GIF of the running game.
- The full menu navigation chain from the previous milestone still works end-to-end (regression check).
- The new unit tests for overlay formatting and FPS smoothing pass locally and in CI.

## Quality and release (post-MVP)

Goal: harden the project once the playable MVP is finished. Optional, can be tackled in any order.

Scope:

- Package release artifacts on tag pushes (a zip per platform with the executable and any required runtime libraries).
- Add line coverage on Linux + GCC using `gcovr`, uploaded to a hosted service (Codecov) and surfaced as a badge in `README.md`:
  - Gate instrumentation behind a CMake option (off by default), e.g. `PONG_ENABLE_COVERAGE`, that appends GCC's `--coverage` (alias for `-fprofile-arcs -ftest-coverage`) to the compile and link flags of both `pong-sdl3-cpp` and `pong-sdl3-cpp-tests`. Match the existing per-target style: duplicate the flag block alongside the current `target_compile_options(... -Wall -Wextra ...)` in `CMakeLists.txt` and `tests/CMakeLists.txt`; promote to an `add_coverage_flags(<target>)` helper only if it grows.
  - Add a `coverage` job to `.github/workflows/build.yml`, `runs-on: ubuntu-latest`, kept separate from the cross-platform `build` matrix so the matrix stays clean. The job installs the same SDL3 transitive dev-headers, configures with `-DCMAKE_BUILD_TYPE=Debug -DPONG_ENABLE_COVERAGE=ON`, builds, runs `ctest --output-on-failure`, then runs `gcovr --lcov coverage.lcov --filter src/ --exclude build/_deps --exclude tests` so the report measures project sources only. The `--exclude build/_deps` filter is mandatory: without it the SDL3 source tree pulled in via `FetchContent` dominates the percentage and makes the number meaningless.
  - Upload `coverage.lcov` with `codecov/codecov-action@v4` (no token required for public GitHub repos on GitHub-hosted runners) and add a Codecov badge to `README.md` alongside the existing Windows / Ubuntu / macOS CI badges.
  - Add a thin `scripts/coverage.sh` wrapper (Linux-only, matching the existing `scripts/build.sh` style) so contributors can reproduce the CI run locally with one command.
  - Start in report-only mode: no `--fail-under-line` threshold. Revisit once a few weeks of data are available.
  - Local Windows coverage is **out of scope** for this milestone. The MSVC build path is small (mostly conditional `#ifdef _WIN32` blocks around DLL handling) and contributors rely on CI for the coverage number. If MSVC-only branches grow non-trivial, revisit with LLVM source-based coverage (`-fprofile-instr-generate -fcoverage-mapping` via `clang-cl`) so a single pipeline runs on every OS.
- Add a basic static-analysis step in CI (`clang-tidy` on Linux, optionally MSVC `/analyze` on Windows). Treat findings as informational at first.

Acceptance criteria:

- Tag pushes produce downloadable artifacts on every CI platform that is green.
- The coverage CI job runs on every push and pull request to `main`, uploads an LCOV report to Codecov, and a Codecov line-coverage badge is visible in `README.md`.
- A static-analysis report is produced by CI, even if not enforced.
