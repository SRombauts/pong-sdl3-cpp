---
name: repo-conventions
description: Repository layout and naming conventions. Use to find or place files.
---

# Repository Conventions

## Directory layout

| Path | Contents |
| --- | --- |
| `src/` | C++ application source. New types live in `src/<TypeName>.{h,cpp}` directly under `src/`, or under `src/<area>/` once an area grows several related files. |
| `tests/` | doctest unit tests. One file per area, CamelCase: `tests/<TypeName>Test.cpp`. The doctest entry point is `tests/main.cpp`. |
| `scripts/` | Thin build/test wrappers (`build.{ps1,sh,cmd}`, `test.{ps1,sh,cmd}`). |
| `docs/` | Long-form documentation: `ROADMAP.md`, `ISSUES.md`. |
| `.github/` | CI workflows (`.github/workflows/build.yml`) and other GitHub config. |
| `.claude/` | Agent skills under `.claude/skills/<skill-name>/SKILL.md`. |
| `CMakeLists.txt` | Top-level build definition. The test target is wired up from `tests/CMakeLists.txt`. |
| `.editorconfig`, `.clang-format` | Style sources of truth (see the `format` skill). |

## Where to put new code

- New domain code (e.g. `Paddle`, `Ball`, `Score`): create `src/<TypeName>.h` and `src/<TypeName>.cpp` directly under `src/`. If an area grows enough to need a folder (several related files), promote it to `src/<area>/` with the same CamelCase rule for individual files.
- Wire new sources into the existing `pong-sdl3-cpp` target in the top-level `CMakeLists.txt` (or, when an area gets its own folder, `add_subdirectory(src/<area>)` with its own `CMakeLists.txt`).
- Add the matching test file as `tests/<TypeName>Test.cpp`. Append it to `tests/CMakeLists.txt` so it is compiled into `pong-sdl3-cpp-tests`; `doctest_discover_tests()` will register every `TEST_CASE` automatically.

## Naming

- Source files use **CamelCase** matching the class, struct, or dominant namespace they contain. Example: a class `Application` lives in `Application.h` (header) and `Application.cpp` (implementation). Files that contain no class or struct (entry points, free-function modules) keep their conventional lowercase name; the most common case is `main.cpp`.
- Header extension is `.h`; implementation extension is `.cpp`.
- Test files mirror the unit they test, in CamelCase, with a `Test` suffix: tests for `Application` go into `tests/ApplicationTest.cpp`. The doctest entry point stays at `tests/main.cpp`.
- CMake target names use kebab-case (`pong-sdl3-cpp`, `pong-sdl3-cpp-tests`).

## Build & test commands

Use the wrappers, never hand-rolled `cmake`/`ctest` invocations, unless the wrappers are unavailable:

- Build: `scripts\build.ps1` (Windows) or `./scripts/build.sh` (Linux/macOS) — see the `build` skill.
- Test: `scripts\test.ps1` or `./scripts/test.sh` — see the `test` skill.
- Format: `clang-format -i …` / `clang-format --dry-run --Werror …` — see the `format` skill.

## Documentation cross-references

- Roadmap and milestone planning: [`docs/ROADMAP.md`](../../../docs/ROADMAP.md).
- Backlog of proposed issues not yet filed on GitHub: [`docs/ISSUES.md`](../../../docs/ISSUES.md).
- Project overview, build, test, formatting: top-level [`README.md`](../../../README.md).
