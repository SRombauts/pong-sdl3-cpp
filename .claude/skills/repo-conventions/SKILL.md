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
- Wire new sources into the **`PONG_SRC` variable** in the top-level `CMakeLists.txt`. Use the existing `${CMAKE_CURRENT_SOURCE_DIR}/src/<TypeName>.cpp` form so the variable can be consumed from `tests/CMakeLists.txt` unchanged. Both `pong-sdl3-cpp` and `pong-sdl3-cpp-tests` read this single list, so adding a file means editing exactly one place. Entry-point sources (`src/main.cpp`, `tests/main.cpp`) stay off the list because each binary supplies its own.
- Per-target settings (`target_link_libraries`, `target_compile_options`, Windows DLL-copy steps) are intentionally duplicated between the two targets. Keep the two blocks visually parallel so a reviewer can spot drift in one glance; promote to a helper macro only if they grow.
- Add the matching test file as `tests/<TypeName>Test.cpp`. Append it to `tests/CMakeLists.txt` so it is compiled into `pong-sdl3-cpp-tests`; `doctest_discover_tests()` will register every `TEST_CASE` automatically. Test sources can `#include "<TypeName>.h"` directly because both targets carry `src/` on their include path.

## Naming

- Source files use **CamelCase** matching the class, struct, or dominant namespace they contain. Example: a class `Application` lives in `Application.h` (header) and `Application.cpp` (implementation). Files that contain no class or struct (entry points, free-function modules) keep their conventional lowercase name; the most common case is `main.cpp`.
- Header extension is `.h`; implementation extension is `.cpp`.
- Test files mirror the unit they test, in CamelCase, with a `Test` suffix: tests for `Application` go into `tests/ApplicationTest.cpp`. The doctest entry point stays at `tests/main.cpp`.
- CMake target names use kebab-case (`pong-sdl3-cpp`, `pong-sdl3-cpp-tests`).

## Code comments

- Describe **architecture, design goals, and *why***, not *how*. The code shows the "how"; comments earn their keep by explaining the choice, the invariant, or the rejected alternative.
- **Cut anything the code, the file name, or the signature already says.** A tight intent comment beats a paragraph re-narrating the code below. "Concise" applies to *content*, not line length: still wrap at the same **120-column** limit as code (see the `format` skill) and run `clang-format -i` on touched files so reflow stays consistent.
- **Drop anything that ages badly or restates universal hygiene.** In particular: forward-looking promises about future milestones (`"will be revisited by the Paddle-controls milestone"`), generic C++ admonitions that apply everywhere (`"do not hard-code these outside this header"`), parentheticals that re-state the surrounding context (`"Static-playfield tuning constants"` inside `Playfield.h`), and `(see foo.cpp)` cross-references the reader can grep for in two seconds. Keep the comment if it explains *this* choice or invariant; cut it if it just states a general best practice.
- Skip obvious comments. `// Returns the elapsed time in seconds` above `double secondsBetween(...)` is noise — the signature already says it.
- Public-API contract notes (preconditions, defensive behaviour, ownership) are fine in headers when they document something the signature itself cannot.

## Build & test commands

Use the wrappers, never hand-rolled `cmake`/`ctest` invocations, unless the wrappers are unavailable:

- Build: `scripts\build.ps1` (Windows) or `./scripts/build.sh` (Linux/macOS) — see the `build` skill.
- Test: `scripts\test.ps1` or `./scripts/test.sh` — see the `test` skill.
- Format: `clang-format -i …` / `clang-format --dry-run --Werror …` — see the `format` skill.

## Documentation cross-references

- Roadmap and milestone planning: [`docs/ROADMAP.md`](../../../docs/ROADMAP.md).
- Backlog of proposed issues not yet filed on GitHub: [`docs/ISSUES.md`](../../../docs/ISSUES.md).
- Project overview, build, test, formatting: top-level [`README.md`](../../../README.md).
