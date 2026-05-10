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

- This milestone introduces no testable pure logic, so no unit tests are added by this issue. Tests resume from the `Ball and collisions` milestone onward.
