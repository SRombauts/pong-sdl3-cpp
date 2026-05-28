# Issues backlog

This file is a staging area for highly detailed issues designed from the roadmap. Each entry below maps to one pull request and is intended to be filed against the matching milestone on GitHub. Once an issue has been fully impelmented, the corresponding entry can be removed from this file.

Issues for later milestones will be added to this file in subsequent batches.

---

## Milestone: Repository, build scripts & CI

> All deliverables of this milestone are now implemented in the repository: core scaffolding (`README.md`, `LICENSE`, `.gitignore`, `docs/ROADMAP.md`), the minimal `CMakeLists.txt` + `src/main.cpp`, the build scripts (`scripts/build.sh`, `scripts/build.ps1`, `scripts/build.cmd`), the `doctest`/CTest unit-test scaffold (issue #1), the GitHub Actions CI workflow (issue #2), the editor formatting config `.editorconfig` + `.clang-format` (issue #3), and the per-task agent skills under `.claude/skills/` (issue #4). No backlog entries remain for this milestone.

---

## Milestone: SDL3 window and game loop

> All deliverables of this milestone are now implemented in the repository. The manual `SDL_DelayNS`-based frame-cap fallback originally listed alongside V-Sync was deliberately deferred (see the rationale recorded in `docs/ROADMAP.md`); no backlog entries remain for this milestone.

---

## Milestone: Static playfield

> All deliverables of this milestone are now implemented in the repository: the fixed logical resolution with letterboxed scaling, the static playfield elements (paddles, ball, dashed center line) driven by layout helpers, and the placeholder `0 0` score rendered with a small hand-drawn 5x7 bitmap font that the Screens-and-menus milestone will reuse (the chosen approach is documented in `src/TextRenderer.h`). No backlog entries remain for this milestone.

---

## Milestone: Paddle controls

> The following deliverables turn the static paddles from the previous milestone into player-controlled entities, behind an input abstraction that future milestones (mouse, gamepad, AI) can plug into without touching the paddle update step. Each entry below is intended to map to one pull request and to leave the game in a working, compilable state.

### Wire `KeyboardPaddleController` and make both paddles playable

#### Description

The first user-visible PR of the milestone: implement `KeyboardPaddleController`, replace both `PaddleControllerNull` instances with one keyboard controller per paddle, and verify that motion is framerate-independent end-to-end. After this PR, two local players can rally without a ball — the **Ball and collisions** milestone supplies the ball motion next.

#### Tasks

- Add `src/KeyboardPaddleController.{h,cpp}` implementing `IPaddleController`. The constructor takes the three scancodes that drive this paddle: an "up" scancode, an "alternate up" scancode (used for the AZERTY-friendly second binding on the left paddle), and a "down" scancode. `tick` returns:
  - `axis = -1.0f` when either up scancode is pressed and down is not.
  - `axis = +1.0f` when down is pressed and neither up scancode is.
  - `axis = 0.0f` when both directions are held simultaneously (deliberate dead-key behavior — neither bias wins) and when no relevant key is held.
  - `targetY = std::nullopt` (the keyboard is a digital device; no absolute target).
- Document the W/Z dual-up binding in the constructor comment: on QWERTY, W is the natural "up" key (the WASD pattern) and Z is unused for movement; on AZERTY (the French layout where W and Z swap), W ends up where Z sits on QWERTY and is awkward, so accepting both gives the same physical-key feel on both layouts without a layout-detection step. Scancodes (not keycodes) are required for this trick to work — that is the load-bearing reason `KeyboardState` snapshots scancodes.
- In `Application`'s constructor, default the two controllers to:
  - **Left paddle**: `KeyboardPaddleController(SDL_SCANCODE_W, SDL_SCANCODE_Z, SDL_SCANCODE_S)`.
  - **Right paddle**: `KeyboardPaddleController(SDL_SCANCODE_UP, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN)` — passing `SDL_SCANCODE_UP` twice keeps the constructor uniform without introducing a "single-up" overload that the next milestone's gamepad controller would never use. Alternative considered: a `std::optional<SDL_Scancode>` for the second up key; rejected as overkill for one branch.
- Wire `src/KeyboardPaddleController.{h,cpp}` into `PONG_SRC` / `PONG_INC` and the test target sources.
- Add `tests/KeyboardPaddleControllerTest.cpp` covering, for both the left-paddle and the right-paddle binding sets:
  - Up scancode held → `axis == -1.0f`, `targetY` empty.
  - Alternate-up scancode held (left paddle's Z) → `axis == -1.0f`.
  - Down scancode held → `axis == +1.0f`.
  - Both up and down held → `axis == 0.0f` (documented dead-key behavior).
  - Neither relevant key held, plus a sprinkling of unrelated keys held → `axis == 0.0f`.
- Document the manual smoke test in the PR description: "build, run, press W/Z/S to move the left paddle, Up/Down to move the right; both paddles stop at the top and bottom walls; releasing all keys leaves the paddle where it was."

#### Acceptance criteria

- Both local players can move their paddles with the keyboard (`W`/`Z` and `S` for the left paddle; `Up` and `Down` for the right). Releasing a key leaves the paddle stationary; pressing the opposite-direction keys at the same time leaves it stationary.
- Paddle motion is framerate-independent: covered by the `PaddleMotion::stepCenterY` test from PR 1 and by a brief manual check that resizing the window (which changes the visual frame rate when V-Sync is on) does not change the perceived paddle speed.
- The paddle update step in `Application` reads input *only* through `IPaddleController`; no direct call to `SDL_GetKeyboardState`, `SDL_GetMouseState`, or any gamepad function. The only SDL input call in the milestone is `snapshotKeyboardState()` in `KeyboardState.cpp`.
- Paddles remain clamped inside the playfield at all times (covered by the clamping tests; verifiable manually by holding a direction key against the wall).
- The new `KeyboardPaddleController` tests pass locally and in CI on the three supported platforms; every test added by earlier PRs in this milestone still passes.
- `clang-format --dry-run --Werror` stays clean on all new and edited files.

#### Notes

- The "press both up and down" dead-key behavior is the simplest unambiguous rule; alternatives (last-pressed wins, up-wins, down-wins) all introduce state the controller does not need today. Revisit only if a user complains.
- The right-paddle binding deliberately omits a second up key: `Up` is unambiguous on every layout, and there is no equivalent of the AZERTY W/Z swap to compensate for. Keeping the binding asymmetric is honest about *why* the left paddle has two up keys.
- No mouse, gamepad, or AI controller in this milestone — those are explicit non-goals on the roadmap. The `IPaddleController` interface introduced in PR 2 is the contract those later milestones plug into; no further interface change is anticipated when they do.
- The placeholder random-score updater added during the static-playfield milestone is left in place. It is wholly thrown away by the **Scoring and match flow** milestone, and removing it now would create a momentary visual regression (the score readout would freeze at `0 0` until that milestone lands).
