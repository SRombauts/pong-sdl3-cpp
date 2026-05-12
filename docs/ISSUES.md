# Issues backlog

This file is a staging area for proposed GitHub issues. Each entry below maps to one pull request and is intended to be filed against the matching milestone on GitHub. Once an issue is filed, the corresponding entry can be removed from this file.

This file itself satisfies the _"Define the initial GitHub issue backlog"_ scope item in [`docs/ROADMAP.md`](ROADMAP.md).

Issues for later milestones will be added to this file in subsequent batches.

---

## Milestone: Repository, build scripts & CI

> All deliverables of this milestone are now implemented in the repository: core scaffolding (`README.md`, `LICENSE`, `.gitignore`, `docs/ROADMAP.md`), the minimal `CMakeLists.txt` + `src/main.cpp`, the build scripts (`scripts/build.sh`, `scripts/build.ps1`, `scripts/build.cmd`), the `doctest`/CTest unit-test scaffold (issue #1), the GitHub Actions CI workflow (issue #2), the editor formatting config `.editorconfig` + `.clang-format` (issue #3), and the per-task agent skills under `.claude/skills/` (issue #4). No backlog entries remain for this milestone.

---

## Milestone: SDL3 window and game loop

### Inject a seedable `RandomSource` abstraction into `Application`

**Labels:** `app`, `tests`, `infrastructure`

**Depends on:** _Implement the SDL3 main loop inside an Application class_, _Extract the production source list into a shared CMake variable_

#### Description

The **Ball and collisions** milestone serves the ball with a small random vertical angle, and the **One-player AI** milestone introduces bounded prediction error. Both require a seedable random source so tests can pin the outcome. Like the `Clock`, we introduce the abstraction now — before any consumer exists — to avoid retrofitting random calls scattered across gameplay code later.

This issue is pure infrastructure: it adds the abstraction, wires it into `Application`, and ships a smoke test that the abstraction is deterministic under a fixed seed. Real consumers (serve angle, AI noise) land in their own milestone issues.

#### Tasks

- [ ] Add `src/RandomSource.h` defining a small interface (`IRandomSource`) with the operations gameplay actually needs: `int intInRange(int lo, int hi)` (inclusive bounds) and `double doubleInRange(double lo, double hi)` (half-open `[lo, hi)`). Keep the surface intentionally small; add operations only when a real consumer needs them.
- [ ] Add `src/RandomSourceMt19937.{h,cpp}` implementing `IRandomSource` over `std::mt19937` with a constructor that takes an explicit `std::uint64_t seed`. Provide a free helper `RandomSourceMt19937 makeNonDeterministicRandomSource()` that seeds from `std::random_device` for production use. Append to `PONG_SRC` so both targets pick it up.
- [ ] Modify `Application` to take an `IRandomSource&` (or owning `std::unique_ptr<IRandomSource>`) via its constructor; default-construct a non-deterministically-seeded `RandomSourceMt19937` for production callers. The reference is exposed to gameplay code via an accessor (e.g. `Application::random()`) for now; the **Ball and collisions** milestone will tighten this when it introduces the serve policy.
- [ ] Add `tests/RandomSourceTest.cpp` with `TEST_CASE`s verifying:
  - `RandomSourceMt19937` with seed `S` produces the same sequence on two independent instances (determinism under fixed seed).
  - `intInRange` respects inclusive bounds across many samples.
  - `doubleInRange` respects half-open bounds across many samples.
  - The non-deterministic helper produces _different_ sequences across two calls (statistical, not strict — acceptable to compare just the first few values).

#### Acceptance criteria

- `IRandomSource`, `RandomSourceMt19937`, and `makeNonDeterministicRandomSource()` are part of `${PONG_SRC}` and therefore available to both the executable and the test binary.
- `Application` carries an injected `IRandomSource&` available to subsequent milestones.
- The shipped executable behaves identically to before (the abstraction has no production consumer yet).
- The new `tests/RandomSourceTest.cpp` `TEST_CASE`s pass locally and in CI.

#### Notes

- Trade-off: this is mild YAGNI — we add infrastructure with no immediate caller. Accepted because the alternative is a churn-y refactor of the **Ball and collisions** PR, where it would land mixed with the actual gameplay diff.
- We deliberately do _not_ expose a generic `next()` returning a raw `uint32_t`. Gameplay code should call typed range helpers; that keeps the call sites readable and the test fakes simple.
