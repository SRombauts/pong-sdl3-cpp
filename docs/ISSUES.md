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

## Cross-cutting refactor: 2D vector type

> This section sits outside the roadmap milestones on purpose. The refactor cuts across the static-playfield code already in `main` (`TextRenderer`, `PlayfieldLayout`, `Playfield::`) and gates the first PR of the next milestone (which introduces the `Paddle` struct). Landing it before that PR avoids retrofitting paired-scalar fields one entity at a time.

### Introduce the `Vec2` aggregate and migrate paired X/Y parameters

#### Description

Several existing function signatures and every upcoming entity carry an X and a Y scalar that always travel together: `TextRenderer::drawText` takes `(originX, originY, …)`; `drawTextCentered` takes `(centerX, originY, …)`; `PlayfieldLayout::leftPaddle` / `rightPaddle` take `(paddleHalfWidth, paddleHalfHeight, …)`; `Playfield::kPaddleHalfWidth` / `kPaddleHalfHeight` are two separate constants; and the next milestone's `Paddle` struct is currently designed as `centerX, centerY, halfWidth, halfHeight, speed`. The pattern grows roughly linearly with the entity count — paddles today, ball tomorrow, mouse target / stick samples after that, menu anchors after the screens milestone — so the cheapest moment to migrate is before the second wave of paired-scalar fields ships.

Introduce a minimal `Vec2` POD aggregate covering 2D positions, sizes, half-extents, and anchors, and migrate the existing call sites that already exhibit the pattern. SDL's rectangle type is explicitly **not** replaced: an earlier roadmap draft proposed a pure `Geometry.h` wrapping `SDL_FRect` too, and the duplication at the SDL boundary (copy in, copy out, every draw) bought no testability that gameplay code did not already have. `Vec2` solves the "X and Y always travel together" pain without re-introducing a parallel rectangle type — rectangles continue to be `SDL_FRect`, used by value across the boundary as already documented in the game-code-architecture skill.

#### Tasks

- Add `src/Vec2.h` (header-only, no `.cpp`) declaring:
  ```
  struct Vec2 {
      float x = 0.0f;
      float y = 0.0f;
  };
  ```
  plus `constexpr [[nodiscard]]` free operators in the same header: `+` and `-` (binary), unary `-`, `*` (`Vec2 * float` and `float * Vec2`), `/` (`Vec2 / float`; no `Vec2 / Vec2` until a consumer needs it), `==`, `!=`, plus `+=`, `-=`, `*=`, `/=` for callers that mutate a stored `Vec2`. No `dot`, `length`, `lengthSquared`, `normalize`, `clamp`, or `cross` — those land in the **Ball and collisions** milestone alongside the tests that need them, not speculatively.
- The default-initialized members (`= 0.0f`) are load-bearing: `Vec2 v;` must produce the zero vector, matching the same pattern `Score` already uses (`int value = 0;`). The header comment records the choice and rejects the no-default-init alternative (which would leave `Vec2 v;` as uninitialized floats — a silent-bug source).
- Place `Vec2` at namespace scope (no enclosing namespace), matching the convention in this repo: namespaces are reserved for *modules of free functions* (`PlayfieldLayout::`, `TextRenderer::`); types (`Application`, `PlayfieldRenderer`, `Score`, the upcoming `Paddle`) sit at the top level.
- Document the design in a header-level comment, including the rejected alternatives recorded above (reimplementing `SDL_FRect` as `Rect{Vec2 topLeft, Vec2 size}`; `glm::vec2`; `std::pair<float, float>`; templating to `Vec2T<T>` to unlock a future `Vec2i`). Note that equality is exact float-bitwise on purpose — `Vec2` is a value snapshot, not a tolerance-aware approximation, and tolerance-aware comparison is the caller's job.
- Migrate `TextRenderer`:
  - `drawText`, `drawTextCentered`, `textGlyphRects`, and the internal `forEachGlyphRect` helper accept a `Vec2 origin` parameter instead of `float originX, float originY`. `drawTextCentered`'s position parameter becomes `Vec2 topCenter` — the function name still disambiguates which axis is left-edge versus centered, so the parameter just needs the right semantic name.
  - Update the layout-formula comment in `TextRenderer.h` so the formula reads in `origin.x` / `origin.y` rather than the old paired scalars.
  - Update `tests/TextRendererTest.cpp` to construct positions via aggregate-init (`Vec2{10.0f, 20.0f}`).
- Migrate `PlayfieldLayout`:
  - `leftPaddle` / `rightPaddle` accept a `Vec2 paddleHalfExtents` instead of `float paddleHalfWidth, float paddleHalfHeight`. Update the docstring on the "half-extents at layout APIs" convention so the example is the new signature.
  - `PlayfieldLayout::ball` keeps the scalar `ballHalfSize` because the ball is uniform; promoting it to `Vec2` would imply non-uniform extents that the production code does not have. Document this asymmetry in the header comment.
  - Update `PlayfieldRenderer`'s constructor only if its signature changes; the dashed-line tuning stays as `dashWidth` / `dashHeight` / `gap` scalars (the three values do not compose as a 2D point — width and height are paired with `gap`, which is one-dimensional).
- Migrate `Playfield::`:
  - Replace `constexpr float kPaddleHalfWidth = 4.0f;` and `constexpr float kPaddleHalfHeight = 16.0f;` with `constexpr Vec2 kPaddleHalfExtents{4.0f, 16.0f};`. Update every reader (`PlayfieldLayout` call sites in `Application::render()`, the `tests/PlayfieldLayoutTest.cpp` test fixtures).
  - Leave `kBallHalfSize`, the dash sizes, and the score-position constants as-is (the score X positions are independent per side; they do not form a 2D anchor either).
- Wire `src/Vec2.h` into `PONG_INC` in the top-level `CMakeLists.txt` (header-only types belong on `PONG_INC` so the IDE surface and the `FILE_SET HEADERS` block stay correct).
- Add `tests/Vec2Test.cpp` covering:
  - Default construction yields `{0.0f, 0.0f}`; aggregate initialization yields the listed values.
  - Each operator returns the hand-computed result (`Vec2{1, 2} + Vec2{3, 4} == Vec2{4, 6}`, scalar multiplication commutes, unary minus negates both components, division by a scalar matches multiplication by its reciprocal within exact float arithmetic for representable values).
  - Each compound-assignment operator (`+=`, `-=`, `*=`, `/=`) leaves the LHS in the expected state and returns a reference to it.
  - Equality is exact float-bitwise: `Vec2{0.1f + 0.2f, 0.0f} != Vec2{0.3f, 0.0f}` is the canonical pin to lock the no-tolerance contract in place against a future reader who would "fix" it.
- Wire `tests/Vec2Test.cpp` into the test target's source list in `tests/CMakeLists.txt`.
- Amend the in-flight `Paddle` struct entry below (the first issue under **Milestone: Paddle controls**, "Introduce the `Paddle` struct and the pure paddle-motion helpers") so the struct definition reads `struct Paddle { Vec2 center; Vec2 halfExtents; float speed; };` and the surrounding text refers to `paddle.center.y` / `paddle.halfExtents.y` instead of `paddle.centerY` / `paddle.halfHeight`. `PaddleMotion`'s helpers stay single-axis (`stepPaddleCenterY`, `clampPaddleCenterY`) because the paddle moves on Y only; they continue to accept the relevant scalars (`halfExtents.y`, `center.y`) extracted by the caller from the new `Vec2` fields. The cross-axis collision math arrives with the **Ball and collisions** milestone and is the right moment to introduce vector-shaped motion helpers, not this PR.

#### Acceptance criteria

- New `src/Vec2.h` exists, is header-only, contains no `#include <SDL3/SDL.h>`, and lives at top-level namespace scope. The type is an aggregate with default-initialized `float x, y` and the documented operator set.
- No public signature in `TextRenderer` or `PlayfieldLayout` still carries paired `*X, *Y` floats for a single 2D position or 2D extent. `Vec2` is the type used in every such slot.
- `Playfield::kPaddleHalfExtents` is the single source of truth for the paddle's half-extent tuning; `kPaddleHalfWidth` / `kPaddleHalfHeight` no longer exist.
- The amendment to the next milestone's `Paddle` struct entry is visible in this file's diff, so the PR that introduces `Paddle` picks up the new type from its first commit and never carries paired scalars.
- The window still shows the same playfield as before — paddles, ball, dashed center line, placeholder score — at identical positions and sizes. The change is a pure refactor; a screenshot taken before and after the PR should diff to zero pixels.
- The new `tests/Vec2Test.cpp` passes and every previously-passing test still passes, locally and in CI on Windows, Linux, and macOS.
- `clang-format --dry-run --Werror` stays clean on all new and edited C++ files.

#### Notes

- This entry deliberately sits outside the roadmap milestones because `Vec2` is cross-cutting: it touches code from the completed **Static playfield** milestone (`TextRenderer`, `PlayfieldLayout`, `Playfield`), the in-flight **Paddle controls** milestone (the next PR's `Paddle` struct), and every later one (`Ball` position/velocity/half-extents, mouse target Y, gamepad stick samples, menu anchors, debug-overlay layout). Doing the migration up front costs one focused PR; deferring it costs touching each new entity's fields twice.
- `SDL_FRect` is intentionally untouched. The rejected `Geometry.h` proposal that wrapped the SDL rectangle paid for a parallel rectangle type with a boundary copy on every draw, with no gameplay-testability win (the rest of the codebase already keeps math pure and SDL-free per the layering rule in the game-code-architecture skill). The same reasoning still applies, so the scope of `Vec2` stops at "X and Y that always travel together" and does *not* extend to rectangles.
- Scope was kept deliberately small: no `Vec2i` (integer variant), no `template <typename T> struct Vec2T`, no `Size2` alias, no geometric helpers (`dot`, `length`, `normalize`, `clamp`). Each of these would be straightforward to add when its first real consumer appears; adding them speculatively would ship untested code paths and a wider type-design surface than the migration justifies right now. `Vec2i` in particular has zero present-day consumer — playfield dimensions are plain `int` and there is no proposal to change that.
- The `Vec2` member names are `x` / `y` even when the value is a size or a half-extent (so half-width reads as `paddleHalfExtents.x`). Introducing a `Size2` alias next to `Vec2` would double the type count without anything the compiler can enforce: `Size2` and `Vec2` would differ only in spelling. If a real bug ever traces to "I added a position to a size", revisit then.
- Defensive boundaries unchanged: callers that already returned early on degenerate inputs (`textGlyphRects` on `pixelSize <= 0.0f`, `centerDashSegments` on `segmentCount <= 0`) still do so; this PR does not introduce any new defensive layer, since `Vec2` itself has no invariant to defend (all `float` pairs are valid `Vec2` values, including NaNs — which is acceptable because the type is purely structural).

---

## Milestone: Paddle controls

> The following deliverables turn the static paddles from the previous milestone into player-controlled entities, behind an input abstraction that future milestones (mouse, gamepad, AI) can plug into without touching the paddle update step. Each entry below is intended to map to one pull request and to leave the game in a working, compilable state.

### Introduce the `Paddle` struct and the pure paddle-motion helpers

#### Description

Before any input layer lands, lock in the data shape paddles will carry for the rest of the project (position, half-extents, speed cap) and the pure motion math that the controller-driven update step will call into. Both the clamping helper and the per-tick update step are free functions in their own translation unit, with no SDL include, so they are unit-tested in isolation and reused unchanged by every later controller (keyboard, mouse, gamepad, AI).

This issue is intentionally the smallest cut in the milestone: it introduces zero behavior change for the player (the paddles still draw at their static default positions) and zero coupling to SDL input. The next issues add the input seam on top of this foundation.

#### Tasks

- Add `src/Paddle.h` (header-only) defining `struct Paddle { float centerX; float centerY; float halfWidth; float halfHeight; float speed; };`. Storing the *center* (not the top-left corner) lets the AABB collision math in the **Ball and collisions** milestone use `Paddle` values directly without scattering `+halfHeight` / `-halfHeight` arithmetic; document the convention in a header-level comment. Provide a small `SDL_FRect toFRect(const Paddle&)` free function in a `.cpp` (so the header stays SDL-free) for the renderer's convenience.
- Add `src/PaddleMotion.{h,cpp}` exposing pure free functions:
  - `struct PaddleControllerRequest { float axis; std::optional<float> targetY; };` — `axis` is in `[-1, +1]` with `+1` = down to match the `+Y` down convention documented in `Playfield.h`; `targetY` is in logical pixels and, when present, names an absolute center-Y the controller is aiming at.
  - `float clampPaddleCenterY(float centerY, float halfHeight, int playfieldHeight)` — keeps the paddle wholly inside the playfield. Cover the defensive case where `2 * halfHeight > playfieldHeight` (return the playfield's vertical midpoint so the paddle is centered rather than asserting or producing NaN).
  - `float stepPaddleCenterY(PaddleControllerRequest request, float currentCenterY, float halfHeight, float speed, int playfieldHeight, double dtSeconds)` — applies the request, caps motion at `speed * dtSeconds`, then clamps via `clampPaddleCenterY`. Precedence when both fields are set: `targetY` wins (it is the more specific intent — mouse and AI aim at a point; `axis` is only a velocity hint). Record the precedence both in the header comment and in the test names.
- Wire `src/Paddle.h` / `src/PaddleMotion.{h,cpp}` into `PONG_SRC` / `PONG_INC` in the top-level `CMakeLists.txt`. The implementation file for `Paddle::toFRect` (e.g. `src/Paddle.cpp`) goes into `PONG_SRC` as well, since it carries the only SDL include for this type.
- In `Application`, replace the two anonymous `PlayfieldLayout::leftPaddle()` / `rightPaddle()` call sites in `render()` with two `Paddle` member instances seeded from the same `Playfield::k…` constants. The paddles still draw at their default positions; no movement yet. Keep the layout helpers untouched — they are still the source of truth for the *default* center-Y; tests under `PlayfieldLayoutTest.cpp` keep covering them.
- Add `tests/PaddleMotionTest.cpp` covering:
  - `clampPaddleCenterY` — top edge (centerY < halfHeight), bottom edge (centerY > playfieldHeight - halfHeight), interior no-op, and the defensive paddle-larger-than-playfield case.
  - `stepPaddleCenterY` with `axis = ±1` and no target — velocity is `± speed`, displacement is `± speed * dt`, and the new center-Y is clamped to the playfield bounds (no escape on a single huge `dt`).
  - **Framerate independence**: one `dt = 0.1s` step and ten `dt = 0.01s` steps with the same axis produce the same final center-Y within a tight tolerance.
  - **No-teleport guarantee**: with `targetY` set arbitrarily far from `currentCenterY`, the displacement is still capped at `speed * dt`; only over many ticks does the paddle reach the target.
  - **Axis-vs-target precedence**: when both `axis = -1` (up) and `targetY = currentCenterY + 100` (down) are present, the paddle moves down (target wins). One `SUBCASE` per documented combination — axis only, target only, both present, neither (no-op).

#### Acceptance criteria

- The window still shows the same static playfield as the previous milestone — paddles, ball, dashed center line, placeholder score — with no visual change.
- `Paddle.h` and `PaddleMotion.h` contain no `#include <SDL3/SDL.h>`. Only `Paddle.cpp` and `Application.cpp` touch SDL types for this milestone.
- The new unit tests pass locally and in CI on Windows, Linux, and macOS. The existing tests (`Application`, `FrameTiming`, `PlayfieldLayout`, `PlayfieldRenderer`, `RandomSource`, `TextRenderer`) still pass unchanged.
- `clang-format --dry-run --Werror` stays clean on all new and edited C++ files.

#### Notes

- The `PaddleControllerRequest` lives in `PaddleMotion.h` rather than in the (later) `PaddleController.h` on purpose: the update step is the only consumer that needs the type, and putting it next to the function that interprets it keeps the precedence rule (target wins) one scroll away from the call site. The controller interface re-uses the same type without duplicating it.
- `std::optional<float>` is preferred over a sentinel value (`NaN`, `-1.0f`) for `targetY` because the precedence rule reads as `if (request.targetY) { … } else { … }` rather than `if (!std::isnan(request.targetY)) { … }`. The cost is one extra byte per request; this is not on a hot path.
- `speed` is stored *per Paddle* rather than as a global constant in `Playfield.h` so the **Analog and gamepad controls** and **One-player AI** milestones can give different paddles different caps (e.g. an Easy AI moves slower than the player) without retrofitting the data structure.

---

### Add the `IPaddleController` abstraction and the `KeyboardState` SDL adapter

#### Description

Introduce the seam between input devices and paddle motion. This issue adds the `IPaddleController` interface (one instance per paddle), a single concrete `NullPaddleController` that returns no input, and a small SDL-fronting `KeyboardState` value type that snapshots the keyboard each frame. After this PR, `Application` reads input through the abstraction instead of touching SDL keyboard APIs directly, but the player still cannot move the paddles because both controllers are null.

The PR is intentionally plumbing-only: it ships zero player-visible change. Keeping the interface and the SDL adapter in their own PR lets the next issue (`KeyboardPaddleController` + wiring) be a focused, easy-to-review slice that is *only* about the keyboard mapping and the integration point.

#### Tasks

- Add `src/PaddleController.h` declaring:
  ```
  class IPaddleController {
  public:
      virtual ~IPaddleController() = default;
      // Deleted copy/move, matching IClock / IRandomSource.
      virtual PaddleControllerRequest tick(const KeyboardState& keyboard) = 0;
  };
  ```
  Document why `tick` takes the keyboard state by reference even though the future mouse/gamepad controllers won't read it: it keeps the signature stable across implementations, mirrors how SDL itself exposes a per-frame snapshot, and lets the **Analog and gamepad controls** milestone extend the parameter list (`MouseState`, `GamepadState`) without breaking the keyboard controller. Alternatives considered (per-controller `update()` with no parameter; a variant input bag) are rejected for the reasons recorded in the header comment.
- Add `src/KeyboardState.{h,cpp}` defining `struct KeyboardState { /* … */ };` plus a free `KeyboardState snapshotKeyboardState()` that wraps `SDL_GetKeyboardState`. The `struct` exposes a single `bool isDown(SDL_Scancode) const` method so test fixtures can pre-populate a `KeyboardState` without going through SDL. Implementation choices to record in the header:
  - **Underlying storage**: a `std::array<bool, SDL_SCANCODE_COUNT>` copied from SDL's internal buffer. The copy is a few hundred bytes per frame — well below the rest of the per-frame budget — and isolates the controllers from SDL's internal buffer lifetime. Alternative considered: hold a `const Uint8*` pointer into SDL's buffer (zero-copy but couples the type's lifetime to SDL, and SDL is the only producer of that buffer).
  - **API surface**: a single `isDown(SDL_Scancode)` rather than a public array, so tests construct fixtures via a builder method (`KeyboardState::withKeysDown({SDL_SCANCODE_W, SDL_SCANCODE_S})`). Scancodes (physical key positions) rather than keycodes (layout-dependent characters) on purpose — W/Z and arrow keys must work identically on QWERTY, AZERTY, and Dvorak.
- Add `src/NullPaddleController.{h,cpp}` (or header-only) implementing `IPaddleController::tick` as `return {0.0f, std::nullopt};`. This is the default Application uses until the next PR wires the keyboard controller in. Tests live in `tests/NullPaddleControllerTest.cpp` and assert the no-input contract against a few fixture `KeyboardState` values.
- In `Application`:
  - Add two `std::unique_ptr<IPaddleController>` members, defaulted in the constructor to `NullPaddleController`. Accept overrides via constructor parameters (defaulted to `nullptr`, matching the `IClock` / `IRandomSource` injection pattern already in place) so tests can substitute a scripted fake.
  - In `update(dtSeconds)`, snapshot the keyboard once, call each controller's `tick`, then call `stepPaddleCenterY` on each paddle. Both controllers return a no-op request for this PR, so the paddles do not move — but the wiring is exercised at runtime, which avoids a "dead code" review concern.
- Wire the new sources / headers into `PONG_SRC` / `PONG_INC` (top-level `CMakeLists.txt`) and the test target's source list (`tests/CMakeLists.txt`).
- Add `tests/KeyboardStateTest.cpp` covering the `isDown` predicate (key present, key absent, all-keys-up default state, and the builder method round-tripping a known scancode set).
- Add `tests/NullPaddleControllerTest.cpp` covering the no-input contract under several fixture `KeyboardState` values (empty, all keys down, a few representative keys down).

#### Acceptance criteria

- The window still behaves exactly as in the previous PR: no visible change, no input response.
- `Application::update` no longer calls `SDL_GetKeyboardState` directly; the only consumer of that SDL API is `snapshotKeyboardState()` in `KeyboardState.cpp`.
- `IPaddleController::tick` takes `const KeyboardState&` and a controller can be substituted from outside `Application` for testing (verifiable by reading `Application`'s constructor signature).
- The new unit tests pass locally and in CI on the three supported platforms; existing tests pass unchanged.
- `clang-format --dry-run --Werror` stays clean on all new and edited files.

#### Notes

- `KeyboardState` could be a `class` with a private array; `struct` + a single accessor is preferred because the type is a value snapshot with no invariants to protect beyond "the array is the right size", and a `struct` reads more naturally at construction sites. If invariants grow (e.g. a "modifier-keys" sub-view), promote to a class then.
- The keyboard-only `tick` signature is a deliberate scope choice — mouse and gamepad come in the **Analog and gamepad controls** milestone and will *extend* the parameter list, not replace it. If the API churn becomes painful (more than two extensions), revisit by bundling the per-frame input into a single `FrameInputState` value type at that point, not now.
- `NullPaddleController` is not just a test stub: it is the default whenever an `Application` is constructed without an explicit controller (tests, future menu states where input is paused, debug tooling). Keep it under `src/`, not under `tests/`.

---

### Wire `KeyboardPaddleController` and make both paddles playable

#### Description

The first user-visible PR of the milestone: implement `KeyboardPaddleController`, replace both `NullPaddleController` instances with one keyboard controller per paddle, and verify that motion is framerate-independent end-to-end. After this PR, two local players can rally without a ball — the **Ball and collisions** milestone supplies the ball motion next.

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
- Paddle motion is framerate-independent: covered by the `stepPaddleCenterY` test from PR 1 and by a brief manual check that resizing the window (which changes the visual frame rate when V-Sync is on) does not change the perceived paddle speed.
- The paddle update step in `Application` reads input *only* through `IPaddleController`; no direct call to `SDL_GetKeyboardState`, `SDL_GetMouseState`, or any gamepad function. The only SDL input call in the milestone is `snapshotKeyboardState()` in `KeyboardState.cpp`.
- Paddles remain clamped inside the playfield at all times (covered by the clamping tests; verifiable manually by holding a direction key against the wall).
- The new `KeyboardPaddleController` tests pass locally and in CI on the three supported platforms; every test added by earlier PRs in this milestone still passes.
- `clang-format --dry-run --Werror` stays clean on all new and edited files.

#### Notes

- The "press both up and down" dead-key behavior is the simplest unambiguous rule; alternatives (last-pressed wins, up-wins, down-wins) all introduce state the controller does not need today. Revisit only if a user complains.
- The right-paddle binding deliberately omits a second up key: `Up` is unambiguous on every layout, and there is no equivalent of the AZERTY W/Z swap to compensate for. Keeping the binding asymmetric is honest about *why* the left paddle has two up keys.
- No mouse, gamepad, or AI controller in this milestone — those are explicit non-goals on the roadmap. The `IPaddleController` interface introduced in PR 2 is the contract those later milestones plug into; no further interface change is anticipated when they do.
- The placeholder random-score updater added during the static-playfield milestone is left in place. It is wholly thrown away by the **Scoring and match flow** milestone, and removing it now would create a momentary visual regression (the score readout would freeze at `0 0` until that milestone lands).
