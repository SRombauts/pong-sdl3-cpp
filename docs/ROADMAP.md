# Roadmap

This roadmap is intentionally incremental. The project starts with a single-file standard C++ hello world and evolves toward a complete Pong clone using SDL3.

The first milestone deliberately avoids SDL3. This keeps the repository, build system, scripts, and CI simple before introducing external dependencies.

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
- Add a GitHub Actions workflow under `.github/workflows/build.yml` that, on every push and pull request to `main`, builds the project and runs `ctest --output-on-failure`, on at least Windows latest (MSVC) and Ubuntu latest (GCC or Clang).
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
- GitHub Actions builds the project and runs the test suite on every push and pull request, on at least Windows and Linux.
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
- Add SDL3 initialization and shutdown directly in `main.cpp`.
- Create a window.
- Create a renderer.
- Add an event loop that handles `SDL_EVENT_QUIT` cleanly.
- Cap the frame rate (V-Sync when available, otherwise a simple manual cap) and expose a per-frame delta time value for later milestones.
- Clear and present a black frame.
- Once the bare SDL3 loop works, refactor the inline logic into a small `Application`/`Game` class that owns SDL3 initialization and shutdown, the window, the renderer, the event loop, and the per-frame delta time. Subsequent milestones extend that class instead of editing `main.cpp` directly.

Acceptance criteria:

- The app opens a window.
- The app exits cleanly.
- The app has a stable main loop.
- A per-frame delta time is computed and ready to be consumed by gameplay code.
- Local scripts and CI still build successfully and `ctest` still passes.

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

Non-goals:

- Do not add gameplay logic yet.

Acceptance criteria:

- The window shows a black background, two static paddles, a static ball, a center dashed line, and a placeholder score, all positioned in a classic Pong layout.
- The window can be resized without distorting the playfield aspect ratio.

## Paddle controls

Goal: make the two paddles controllable.

Scope:

- Introduce a `Paddle` struct with position, half-size, and speed; replace the hard-coded paddle rectangles from the previous milestone by reading from these structs.
- Add keyboard input state.
- Map W&Z/S to the left paddle.
- Map Up/Down arrows to the right paddle.
- Move paddles using a variable timestep driven by the per-frame delta time.
- Clamp paddles inside the playfield.

Acceptance criteria:

- Both local players can move their paddles.
- Movement is framerate independent.

## Ball and collisions

Goal: make the core game loop playable.

Scope:

- Introduce a `Ball` struct with position, half-size, and velocity; replace the hard-coded ball rectangle from the static playfield milestone by reading from this struct.
- Move the ball using delta time.
- Bounce on top and bottom walls.
- Detect paddle collisions using AABB intersection.
- Reflect the ball based on the vertical offset of the hit on the paddle, so the bounce angle varies within a bounded range (for example up to ±60° from horizontal).
- On hit, push the ball outside the paddle before applying the new velocity, to prevent it from sticking.
- Define and document a serve policy when the ball is reset (for example: serve toward the player who just lost the point, with a small random vertical angle).
- Increase ball speed on each paddle hit, with a maximum speed cap, so rallies become harder over time.
- Add unit tests covering the pure logic of this milestone using the test target introduced earlier: top/bottom wall bounce, AABB collision detection, reflection-angle computation, anti-stick correction, and serve-direction policy.

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
- Add unit tests for the pure logic of this milestone: out-of-bounds detection on the left and right edges, score increment for the correct side, win detection at the configured target score, and round-reset state.

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

Acceptance criteria:

- The game has clear navigation states driven by the state machine.
- Gameplay code is not mixed with menu logic.
- From a fresh launch, the user can reach Title → ModeSelect → Playing → Paused → Playing → GameOver → Title using only keyboard input.

## One-player AI

Goal: add a 1-player extension against an AI opponent.

Scope:

- Have the AI control the right paddle (the left paddle stays player-controlled).
- Add simple AI paddle tracking of the ball's vertical position.
- Limit AI paddle speed (must use the same speed cap as the player).
- Add at least one of: a reaction delay before the AI starts tracking, or a bounded prediction error on the targeted Y position.
- Expose difficulty levels (for example Easy / Normal / Hard) wired to the mode selection screen, tuning speed and/or error.
- Add unit tests for the AI controller: target-Y selection from the ball position, speed-cap enforcement, and bounded prediction error.

Acceptance criteria:

- One-player mode is playable.
- The AI is beatable and not perfectly reactive.
- The new unit tests pass locally and in CI.

## Audio and polish

Goal: make the project presentable.

Scope:

- Pick an audio backend (SDL3 native audio or `SDL3_mixer`) and integrate it through CMake (via `FetchContent`, like SDL3) and CI.
- Add minimal arcade-style sound effects: wall bounce, paddle hit, score, and match end.
- Add a debug overlay (toggleable) showing at least FPS and ball/paddle state.
- Add screenshots or a short GIF to the `README.md`.
- Add small visual polish if useful (for example a brief flash when a point is scored).

Acceptance criteria:

- Sound effects play on wall bounce, paddle hit, score, and match end.
- The debug overlay can be toggled at runtime.
- The `README.md` contains at least one screenshot or GIF of the running game.
- The full menu navigation chain from the previous milestone still works end-to-end (regression check).

## Quality and release (post-MVP)

Goal: harden the project once the playable MVP is finished. Optional, can be tackled in any order.

Scope:

- Add macOS (latest) to the CI matrix.
- Package release artifacts on tag pushes (a zip per platform with the executable and any required runtime libraries).
- Add code coverage on Linux + GCC using `gcovr` or `llvm-cov` and publish the report as a CI artifact.
- Add a basic static-analysis step in CI (`clang-tidy` on Linux, optionally MSVC `/analyze` on Windows). Treat findings as informational at first.

Acceptance criteria:

- macOS CI is green on every push and pull request.
- Tag pushes produce downloadable artifacts on every CI platform that is green.
- macOS artifacts are produced once macOS CI is green.
- A coverage report and a static-analysis report are produced by CI, even if not enforced.
