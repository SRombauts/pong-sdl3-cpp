---
name: repo-conventions
description: Repository layout and naming conventions. Use to find or place files.
---

# Repository Conventions

## Directory layout

| Path | Contents |
| --- | --- |
| `src/` | C++ application source. New domain code goes under `src/<area>/`. |
| `tests/` | doctest unit tests. One file per area: `tests/test_<area>.cpp`. |
| `scripts/` | Thin build/test wrappers (`build.{ps1,sh,cmd}`, `test.{ps1,sh,cmd}`). |
| `docs/` | Long-form documentation: `ROADMAP.md`, `ISSUES.md`. |
| `.github/` | CI workflows (`.github/workflows/build.yml`) and other GitHub config. |
| `.claude/` | Agent skills under `.claude/skills/<skill-name>/SKILL.md`. |
| `CMakeLists.txt` | Top-level build definition. The test target is wired up from `tests/CMakeLists.txt`. |
| `.editorconfig`, `.clang-format` | Style sources of truth (see the `format` skill). |

## Where to put new code

- New domain code (e.g. `paddle`, `ball`, `score`): create `src/<area>/` with the `.cpp`/`.hpp` files for that area.
- Wire new sources into the existing `pong-sdl3-cpp` target in the top-level `CMakeLists.txt` (or, if the area grows, an `add_subdirectory(src/<area>)` with its own `CMakeLists.txt`).
- Add the matching test file as `tests/test_<area>.cpp`. Append it to `tests/CMakeLists.txt` so it is compiled into `pong-sdl3-cpp-tests`; `doctest_discover_tests()` will register every `TEST_CASE` automatically.

## Naming

- Source and test files: lowercase, snake_case, `.cpp` / `.hpp` extensions.
- Test files always carry the `test_` prefix and mirror the area name: `tests/test_<area>.cpp`.
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
