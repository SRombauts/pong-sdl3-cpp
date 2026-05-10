# Pong SDL3 C++

[![Windows CI](https://img.shields.io/github/check-runs/SRombauts/pong-sdl3-cpp/main?nameFilter=windows-latest&label=Windows)](https://github.com/SRombauts/pong-sdl3-cpp/actions/workflows/build.yml)
[![Ubuntu CI](https://img.shields.io/github/check-runs/SRombauts/pong-sdl3-cpp/main?nameFilter=ubuntu-latest&label=Ubuntu)](https://github.com/SRombauts/pong-sdl3-cpp/actions/workflows/build.yml)
[![macOS CI](https://img.shields.io/github/check-runs/SRombauts/pong-sdl3-cpp/main?nameFilter=macos-latest&label=macOS)](https://github.com/SRombauts/pong-sdl3-cpp/actions/workflows/build.yml)

A Pong clone written in modern C++ with SDL3.

This repository is intended as a small, finishable 2D game programming project.
The first target is a faithful two-player Pong-style game.
A one-player mode against a simple AI is planned as a later extension.

Original Atari PONG (1972) arcade machine gameplay video (en) showing output from a real Atari/Syzygy board:

[![Original Atari PONG (1972) arcade machine gameplay video (en)](https://img.youtube.com/vi/fiShX2pTz9A/hqdefault.jpg)](https://www.youtube.com/watch?v=fiShX2pTz9A)

## Project goals

- Build a complete mini game from scratch.
- Use modern C++ and SDL3 directly, without a game engine.
- Keep the first playable version deliberately small.
- Separate core gameplay, rendering, input, and application lifecycle code.
- Produce a repository that can be extended with menus, AI, sound, and polish.

## Historical scope

The original Atari Pong arcade game was a local two-player game. This project starts with that simpler two-player scope.

## Planned features

### MVP

- SDL3 window and renderer.
- Fixed logical game resolution.
- Two paddles controlled locally.
- Multiple input methods: keyboard, mouse for analog precision, and gamepad analog sticks (including a single-gamepad couch co-op mode where one stick drives each paddle).
- Ball movement and wall bounces.
- Paddle collisions with angle variation.
- Score tracking.
- Match end condition.

### Extensions

- Title screen.
- Mode selection screen.
- Pause screen.
- Game-over screen.
- One-player mode against an AI opponent.
- Minimal arcade-style sound effects.
- Debug overlay.
- Visual polish.

### Post-MVP infrastructure

- Release artifact packaging on tag pushes.
- Code coverage reports.
- Basic static analysis.

## Controls

The game can be played with the keyboard, the mouse, or a gamepad. Connecting or disconnecting a gamepad at runtime reassigns players without a restart.

### Keyboard

| Action | Key |
| --- | --- |
| Left paddle up | W & Z |
| Left paddle down | S |
| Right paddle up | Up Arrow |
| Right paddle down | Down Arrow |
| Start / confirm | Enter |
| Pause / back | Escape |

### Mouse

The left paddle can optionally be driven by the mouse for analog-precise control, mirroring the feel of the original Atari Pong potentiometer paddles. The system cursor is hidden while mouse control is active.

### Gamepad

Multiple gamepads or a single shared gamepad both work:

- **Two gamepads**: each player uses one gamepad's left stick.
- **One shared gamepad**: left stick drives P1, right stick drives P2 (single-gamepad couch co-op, in the spirit of the original Pong).

A small dead-zone is applied to resting sticks.

## Build

This project uses CMake. SDL3 and the unit-test framework (`doctest`) are acquired through CMake `FetchContent` from pinned upstream tags, so no separate package manager or system-wide install is required for them. SDL3 is introduced in the second milestone; until then the runtime executable itself is dependency-free, although `doctest` is already fetched as a build-time dependency for the test target.

The repository ships with thin build scripts that wrap CMake. They produce an out-of-source build under `build/` and default to a Debug configuration.

### Windows (PowerShell)

```powershell
scripts\build.ps1
```

By default this uses the latest installed Visual Studio as the CMake generator (detected through `vswhere.exe`) and targets `x64`. Useful overrides:

```powershell
scripts\build.ps1 -Config Release
scripts\build.ps1 -Generator Ninja -Config Release
scripts\build.ps1 -Clean
scripts\build.ps1 -Help    # see Get-Help scripts\build.ps1 for full options
```

A `scripts\build.cmd` wrapper is also provided for use from `cmd.exe`:

```cmd
scripts\build.cmd
```

The resulting executable is under `build\Debug\pong-sdl3-cpp.exe` (or `build\Release\` for a Release build) when using a Visual Studio generator.

### Linux and macOS

```bash
./scripts/build.sh
./scripts/build.sh --config Release
./scripts/build.sh --generator Ninja --config Release
./scripts/build.sh --clean
./scripts/build.sh --help
```

The resulting executable is `build/pong-sdl3-cpp`.

### Plain CMake

The scripts are optional; you can always invoke CMake directly:

```bash
cmake -S . -B build
cmake --build build
```

### Tests

The unit-test target `pong-sdl3-cpp-tests` is built by default alongside the main executable. It uses [doctest](https://github.com/doctest/doctest), fetched at configure time via CMake `FetchContent` and pinned to a specific upstream tag.

After building, run all tests via the thin test scripts (which wrap CTest):

```powershell
scripts\test.ps1
```

```bash
./scripts/test.sh
```

Both scripts default to `Debug` configuration. Useful overrides:

```powershell
scripts\test.ps1 -Config Release
scripts\test.ps1 -ExtraArgs '-R','smoke','-j','4'
```

```bash
./scripts/test.sh --config Release
./scripts/test.sh -- -R smoke -j 4
```

A `scripts\test.cmd` wrapper is also provided for use from `cmd.exe`.

The scripts deliberately do **not** trigger a build; they fail fast with a clear message if `build/` does not exist. Build first via `scripts/build.{ps1,sh}`. The raw CTest invocation also works as a fallback:

```bash
ctest --test-dir build --output-on-failure
```

Each `TEST_CASE` is registered as an individual CTest entry through `doctest_discover_tests()`. The test executable can also be invoked directly (`build/pong-sdl3-cpp-tests` on Linux/macOS, `build\tests\Debug\pong-sdl3-cpp-tests.exe` on Windows with a Visual Studio generator) and accepts all standard `doctest` command-line filters.

Testing can be disabled at configure time with `-DBUILD_TESTING=OFF` (CMake's standard option, set by `include(CTest)`); doing so also skips the `FetchContent` download of `doctest`.

### Formatting

C++ formatting is pinned by [`.clang-format`](.clang-format) at the repository root (Allman braces, 4-space indent, 120-column limit, based on `LLVM`). Editor-level settings (indent style, line endings, charset, final newline) are pinned by [`.editorconfig`](.editorconfig) and are picked up automatically by editors that support EditorConfig.

Apply the format in place to all C++ sources:

```bash
clang-format -i src/main.cpp tests/*.cpp
```

Verify without rewriting (used by CI):

```bash
clang-format --dry-run --Werror src/main.cpp tests/*.cpp
```

The CI workflow runs the verification step on every push and pull request and will fail the build on any unformatted change.

## Dependencies

- C++20 compiler.
- CMake 3.24 or newer.
- Network access at first configure time, so CMake can fetch SDL3 and `doctest` via `FetchContent`.
- On Linux, SDL3's transitive system dev-headers (X11/Wayland/OpenGL/EGL/PulseAudio/ALSA) so SDL3 can build from source. On Windows with MSVC, no extra packages are required.

SDL3 is consumed through the standard `SDL3::SDL3` CMake target.

## Suggested GitHub topics

The GitHub connector used to initialize this repository does not currently expose an action for editing repository topics. Suggested topics:

```text
cpp
sdl3
pong
game-development
retro-gaming
arcade
cmake
```

## Roadmap

The repository issues are organized around the following planned milestones:

- Repository, build scripts & CI
- SDL3 window and game loop
- Static playfield
- Paddle controls
- Analog and gamepad controls
- Ball and collisions
- Scoring and match flow
- Screens and menus
- One-player AI
- Audio and polish
- Quality and release (post-MVP)

A detailed roadmap is available in [`docs/ROADMAP.md`](docs/ROADMAP.md).

## Repository conventions and agent skills

Per-task guidance for both human contributors and AI coding agents lives under [`.claude/skills/`](.claude/skills/). Each skill is a small `SKILL.md` with a YAML frontmatter and a focused body covering one topic:

- [`build/`](.claude/skills/build/SKILL.md) — how to build and run the project.
- [`test/`](.claude/skills/test/SKILL.md) — how to run, filter, or disable the test suite.
- [`format/`](.claude/skills/format/SKILL.md) — how to format and verify C/C++ sources with `clang-format`.
- [`repo-conventions/`](.claude/skills/repo-conventions/SKILL.md) — directory layout, naming conventions, and where new code/tests go.
- [`agent-response-style/`](.claude/skills/agent-response-style/SKILL.md) — default tone and reasoning style for AI coding agents.
- [`development-workflow/`](.claude/skills/development-workflow/SKILL.md) — branch / commit / PR workflow for working on a numbered task.

Read these before opening a pull request; they are the source of truth for the dev loop.

## Reference media

Pong gameplay screenshot, from Wikimedia Commons (public domain):

[![Pong gameplay screenshot](https://upload.wikimedia.org/wikipedia/commons/2/26/Pong.svg)](https://commons.wikimedia.org/wiki/File:Pong.svg)
