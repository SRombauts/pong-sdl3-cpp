# Issues backlog

This file is a staging area for proposed GitHub issues. Each entry below maps to one pull request and is intended to be filed against the matching milestone on GitHub. Once an issue is filed, the corresponding entry can be removed from this file.

This file itself satisfies the _"Define the initial GitHub issue backlog"_ scope item in [`docs/ROADMAP.md`](ROADMAP.md).

Issues for later milestones will be added to this file in subsequent batches.

---

## Milestone: Repository, build scripts & CI

> The following deliverables of this milestone are already implemented in the repository and therefore do not need a backlog entry: core scaffolding (`README.md`, `LICENSE`, `.gitignore`, `docs/ROADMAP.md`), the minimal `CMakeLists.txt` + `src/main.cpp`, the build scripts (`scripts/build.sh`, `scripts/build.ps1`, `scripts/build.cmd`), and the `doctest`/CTest unit-test scaffold (issue #1).

### Add GitHub Actions CI workflow

**Labels:** `ci`, `infra`, `tests`

**Depends on:** _Integrate doctest via FetchContent_

#### Description

Add a CI workflow that builds the project and runs the test suite on every push and pull request to `main`, on Windows and Linux. CI must fail on a non-zero CTest exit code so breakage on either platform blocks merges.

#### Tasks

- [ ] Add `.github/workflows/build.yml` with a clear `name`, triggers `on: push` and `on: pull_request` targeting the `main` branch.
- [ ] Use a build matrix with at least `os: [windows-latest, ubuntu-latest]`.
- [ ] Steps per job: `actions/checkout@v4`, configure (`cmake -S . -B build`), build (`cmake --build build --config Debug`), and test (`ctest --test-dir build --output-on-failure -C Debug`).
- [ ] Optionally cache `build/_deps/` keyed on the doctest tag to skip re-cloning on every run.
- [ ] Make the CI badge URL available; adding the badge itself to the README can be a follow-up issue if preferred.

#### Acceptance criteria

- The workflow appears in the GitHub Actions tab.
- Both `windows-latest` and `ubuntu-latest` jobs run and turn green on a no-op PR.
- A deliberately failing test causes the corresponding job to turn red.
- The workflow is no longer than ~50 lines of YAML.

#### Notes

- macOS is intentionally **not** part of this matrix; it is captured by the post-MVP _"Quality and release"_ milestone.

---

### Add editor formatting config (`.editorconfig` + `.clang-format`)

**Labels:** `tooling`, `infra`

#### Description

Pin formatting expectations so that contributors and AI agents converge on a single style without arguing. `.editorconfig` covers indentation, line endings, charset, and final newlines for any editor that supports it; `.clang-format` is the source of truth for C++ formatting.

#### Tasks

- [ ] Add `.editorconfig` at the repository root with: `root = true`, `indent_style = space`, `indent_size = 4`, `end_of_line = lf`, `charset = utf-8`, `trim_trailing_whitespace = true`, `insert_final_newline = true`. Override `indent_size = 2` for `*.{yml,yaml,json,md}`.
- [ ] Add `.clang-format` at the repository root, based on a known preset (`BasedOnStyle: LLVM` is the simplest starting point), with `IndentWidth: 4`, `ColumnLimit: 120`, and an explicit brace style (`BreakBeforeBraces: Allman` is suggested; pick one and document it).
- [ ] Run `clang-format -i src/main.cpp tests/*.cpp` once after creating the file so the existing code matches.

#### Acceptance criteria

- `.editorconfig` is present at the repository root.
- `.clang-format` is present at the repository root.
- `clang-format --dry-run --Werror src/main.cpp tests/*.cpp` exits with code 0 (no diff).

---

### Add agent skills under `.claude/skills/`

**Labels:** `docs`, `infra`, `tooling`

**Depends on:** _Integrate doctest via FetchContent_, _Add editor formatting config_

#### Description

Add per-task agent skills under `.claude/skills/` so AI agents (and human contributors reading the same files) have one structured place describing how to build, test, format, and navigate the project. Each skill is a standalone folder with a `SKILL.md` carrying a YAML frontmatter (`name`, `description`) and a concise body.

The repository already contains:

- `.claude/skills/agent-response-style/SKILL.md` — default response behavior for AI agents.
- `.claude/skills/build/SKILL.md` — how to build and run the project.

This issue adds the missing companion skills so the basic dev loop is fully covered.

#### Tasks

- [ ] Add `.claude/skills/test/SKILL.md` describing how to run the test suite (`ctest --test-dir build --output-on-failure`), how to build and run an individual test target, and how to disable testing entirely (`-DBUILD_TESTING=OFF`).
- [ ] Add `.claude/skills/format/SKILL.md` describing how to format C/C++ files with `clang-format -i <files>` and how to verify with `clang-format --dry-run --Werror <files>`. Reference `.clang-format` as the source of truth for the style.
- [ ] Add `.claude/skills/repo-conventions/SKILL.md` describing where things live (`src/`, `tests/`, `scripts/`, `docs/`, `.github/`, `.claude/`), naming conventions, and the rule "add new domain code under `src/<area>/`, add tests under `tests/test_<area>.cpp`".
- [ ] Each `SKILL.md` must have a YAML frontmatter with at least `name` and `description`. Keep each one short (~30–50 lines).
- [ ] Add a brief mention of the `.claude/skills/` directory in `README.md`, pointing readers there for AI-agent and contributor guidance.

#### Acceptance criteria

- `.claude/skills/` contains at least: `agent-response-style/`, `build/`, `test/`, `format/`, and `repo-conventions/`, each with a `SKILL.md`.
- Every `SKILL.md` has a parseable YAML frontmatter with `name` and `description`.
- A new contributor (or AI agent) can build, test, and format the project on Windows and Linux using only the skill files plus the linked scripts/CMakeLists.
- `README.md` mentions `.claude/skills/` at least once.
