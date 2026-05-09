# Roadmap

This roadmap is intentionally incremental. The project starts with a single-file standard C++ hello world and evolves toward a complete Pong clone using SDL3.

The first milestone deliberately avoids SDL3. This keeps the repository, build system, scripts, and CI simple before introducing external dependencies.

## Repository, build scripts & CI

Goal: establish a usable repository baseline and make local and remote builds repeatable before introducing SDL3.

Scope:

- Create the GitHub repository.
- Add a README.
- Add a `.gitignore`.
- Add a minimal `CMakeLists.txt`.
- Add a single `src/main.cpp` hello world.
- Add `docs/ROADMAP.md`.
- Define the initial GitHub issue backlog.
- Keep the initial project dependency-free except for the C++ toolchain and CMake.
- Add a basic Unix-like build script, for example `scripts/build.sh`.
- Add a basic Windows PowerShell build script, for example `scripts/build.ps1`.
- Optionally add a thin Windows Command Prompt wrapper, for example `scripts/build.cmd`.
- Use out-of-source CMake builds, preferably under `build/`.
- Support at least a default Debug build.
- Optionally support a `Release` configuration argument.
- Add a GitHub Actions workflow under `.github/workflows/build.yml`.
- Build on pull requests and pushes to `main`.
- Start CI with the dependency-free hello-world executable.
- Keep scripts thin: they should call CMake, not become a custom build system.

Suggested script behavior:

- Configure with `cmake -S . -B build`.
- Build with `cmake --build build`.
- Return a non-zero exit code on failure.
- Avoid silently deleting user files.

Suggested initial CI matrix:

- Windows latest, MSVC.
- Ubuntu latest, GCC or Clang.
- macOS latest is optional at this stage.

Non-goals:

- Do not add SDL3 yet.
- Do not add an application framework yet.
- Do not add multiple source files yet unless they are documentation or repository infrastructure.
- Do not add gameplay code yet.
- Do not install SDL3 in CI yet.
- Do not package release artifacts yet.
- Do not add code coverage yet.
- Do not add static analysis unless it stays trivial.

Acceptance criteria:

- The repository can be cloned.
- The project configures with CMake.
- The project builds a minimal standard C++ executable.
- The executable prints a simple hello-world message.
- A developer can build locally using one command.
- GitHub Actions builds the project successfully on at least Windows and Linux.
- CI status is visible on pull requests.
- The workflow remains simple enough to extend when SDL3 is introduced.
- No SDL3 dependency is required yet.

## SDL3 window and game loop

Goal: introduce SDL3 and open a minimal window.

Scope:

- Install and discover SDL3 through CMake.
- Update local build scripts if SDL3 needs explicit discovery paths.
- Update GitHub Actions to install or provide SDL3.
- Add SDL3 initialization and shutdown.
- Create a window.
- Create a renderer.
- Add an event loop.
- Clear and present a black frame.

Acceptance criteria:

- The app opens a window.
- The app exits cleanly.
- The app has a stable main loop.
- Local scripts and CI still build successfully.

## Static playfield

Goal: make the program visually recognizable as Pong.

Scope:

- Add fixed logical rendering.
- Draw black background.
- Draw center dashed line.
- Draw static paddles.
- Draw static ball.
- Draw placeholder score.

Acceptance criteria:

- A screenshot visually resembles Pong.
- No gameplay logic is required yet.

## Paddle controls

Goal: make the two paddles controllable.

Scope:

- Add keyboard input state.
- Map W&Z/S to the left paddle.
- Map Up/Down arrows to the right paddle.
- Move paddles using delta time.
- Clamp paddles inside the playfield.

Acceptance criteria:

- Both local players can move their paddles.
- Movement is framerate independent.

## Ball and collisions

Goal: make the core game loop playable.

Scope:

- Add ball velocity.
- Move the ball using delta time.
- Bounce on top and bottom walls.
- Detect paddle collisions.
- Reflect the ball based on hit position.
- Optionally increase ball speed after paddle hits.

Acceptance criteria:

- Players can rally the ball.
- The ball behaves predictably and does not stick to paddles or walls.

## Scoring and match flow

Goal: complete the two-player MVP.

Scope:

- Detect when the ball leaves the playfield.
- Award points.
- Reset the round after each point.
- Detect the winning score.
- Reset the match.

Acceptance criteria:

- A full two-player Pong match can be played from start to finish.

## Screens and menus

Goal: turn the prototype into a structured game.

Scope:

- Add title screen.
- Add mode selection screen.
- Add pause state.
- Add game-over screen.
- Add restart and quit flows.

Acceptance criteria:

- The game has clear navigation states.
- Gameplay code is not mixed with menu logic.

## One-player AI

Goal: add a modern one-player extension.

Scope:

- Add simple AI paddle tracking.
- Limit AI speed.
- Add reaction delay or prediction error.
- Add difficulty tuning.

Acceptance criteria:

- One-player mode is playable.
- The AI is beatable and not perfectly reactive.

## Audio and polish

Goal: make the project presentable.

Scope:

- Add minimal sound effects.
- Add debug overlay.
- Add screenshots or GIFs to the README.
- Add small visual polish if useful.

Acceptance criteria:

- The repository demonstrates a finished mini project.
