# Issues backlog

This file is a staging area for proposed GitHub issues. Each entry below maps to one pull request and is intended to be filed against the matching milestone on GitHub. Once an issue is filed, the corresponding entry can be removed from this file.

This file itself satisfies the _"Define the initial GitHub issue backlog"_ scope item in [`docs/ROADMAP.md`](ROADMAP.md).

Issues for later milestones will be added to this file in subsequent batches.

---

## Milestone: Repository, build scripts & CI

> The following deliverables of this milestone are already implemented in the repository and therefore do not need a backlog entry: core scaffolding (`README.md`, `LICENSE`, `.gitignore`, `docs/ROADMAP.md`), the minimal `CMakeLists.txt` + `src/main.cpp`, the build scripts (`scripts/build.sh`, `scripts/build.ps1`, `scripts/build.cmd`), the `doctest`/CTest unit-test scaffold (issue #1), and the GitHub Actions CI workflow (issue #2).

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
