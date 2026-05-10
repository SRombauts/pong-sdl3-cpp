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
- Cap the frame rate (V-Sync when available, otherwise a simple manual cap) and expose a per-frame delta time value for later milestones.
- Clear and present a black frame each tick.

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

Goal: make the two paddles controllable, with an input layer that other devices (mouse, gamepad, AI) can plug into in later milestones.

Scope:

- Introduce a `Paddle` struct with position, half-size, and speed; replace the hard-coded paddle rectangles from the previous milestone by reading from these structs.
- Introduce a small `PaddleController` abstraction (one instance per paddle) so the paddle's update step does not depend directly on a specific input device. A controller returns a per-tick request expressed both as a desired axis value in `[-1, +1]` (used by digital and stick-as-velocity inputs) and an optional target Y (used by mouse and AI). The paddle moves toward the target while remaining clamped by the paddle's speed cap, so no controller can teleport the paddle.
- Implement `KeyboardPaddleController` as the first concrete controller. Map W and Z to "up" / S to "down" for the left paddle, and Up / Down arrows for the right paddle.
- Add keyboard input state plumbing.
- Move paddles using a variable timestep driven by the per-frame delta time.
- Clamp paddles inside the playfield.

Non-goals:

- Mouse, gamepad, and single-gamepad couch co-op analog inputs (covered by the **Analog and gamepad controls** milestone).
- AI-controlled paddles (covered by the **One-player AI** milestone, which adds an `AiPaddleController` that plugs into the same abstraction).

Acceptance criteria:

- Both local players can move their paddles with the keyboard.
- Movement is framerate independent.
- The paddle update code reads input only through `PaddleController`; it does not call SDL3 keyboard / mouse / gamepad APIs directly.

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

- Add an `AiPaddleController` that implements the `PaddleController` abstraction by computing a target Y from the ball's vertical position. It plugs into a paddle the same way as the keyboard, mouse, or gamepad controllers from earlier milestones.
- Wire the AI controller to the right paddle in 1-player mode (the left paddle stays player-controlled).
- Limit AI paddle speed (must use the same speed cap as the player); since the paddle's `PaddleController`-driven update already clamps motion to the speed cap, the AI cannot teleport.
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

- Package release artifacts on tag pushes (a zip per platform with the executable and any required runtime libraries).
- Add code coverage on Linux + GCC using `gcovr` or `llvm-cov` and publish the report as a CI artifact.
- Add a basic static-analysis step in CI (`clang-tidy` on Linux, optionally MSVC `/analyze` on Windows). Treat findings as informational at first.

Acceptance criteria:

- Tag pushes produce downloadable artifacts on every CI platform that is green.
- A coverage report and a static-analysis report are produced by CI, even if not enforced.
