# Pong SDL3 C++

A Pong clone written in modern C++ with SDL3.

This repository is intended as a small, finishable 2D game programming project.
The first target is a faithful two-player Pong-style game.
A one-player mode against a simple AI could be planned as a later extension.

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
- Ball movement and wall bounces.
- Paddle collisions with angle variation.
- Score tracking.
- Match end condition.

### Extensions

- Title screen.
- Mode selection screen.
- Pause screen.
- Game-over screen.
- One-player mode against AI.
- Minimal arcade-style sound effects.
- Debug overlay.
- Visual polish.

## Controls

Planned default controls:

| Action | Key |
| --- | --- |
| Left paddle up | W & Z |
| Left paddle down | S |
| Right paddle up | Up Arrow |
| Right paddle down | Down Arrow |
| Start / confirm | Enter |
| Pause / back | Escape |

## Build

This project uses CMake and SDL3.

```bash
cmake -S . -B build
cmake --build build
```

Run:

```bash
./build/pong-sdl3-cpp
```

On Windows, the executable path may differ depending on the generator, for example:

```powershell
.\build\Debug\pong-sdl3-cpp.exe
```

## Dependencies

- C++20 compiler.
- CMake 3.24 or newer.
- SDL3 development package discoverable by CMake.

The initial `CMakeLists.txt` expects SDL3 to expose the `SDL3::SDL3` CMake target.

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
- Ball and collisions
- Scoring and match flow
- Screens and menus
- One-player AI
- Audio and polish

A detailed roadmap is available in [`docs/ROADMAP.md`](docs/ROADMAP.md).
