---
name: game-code-architecture
description: >-
  Design and review guidelines for this SDL3 Pong codebase: frame-loop constraints, testable layering, and design patterns.
  Use when designing new features, populating the ISSUES.md, implementing a task, reviewing a diff or branch, or when the user asks for an architecture or standards check.
---

# Game architecture & code standards

Cross-cutting rules that hold on every change. Testing strategy and milestone acceptance live in [`docs/ROADMAP.md`](../../../docs/ROADMAP.md); repo layout and file placement in [`repo-conventions`](../repo-conventions/SKILL.md).

## When to apply

1. **Roadmap planning** — architecture and technical choices.
2. **Before coding** — sketch where logic lives (pure helper vs SDL boundary vs `Application`) and what gets a `TEST_CASE`.
3. **While implementing** — keep `update()` / `render()` cheap; push math and policy into testable free functions.
4. **On review** — walk the checklist; flag violations with a one-line *why* and a concrete fix.

## Frame loop (`Application::run`)

`pollEvents` → `tickFrameClock` → `update(dt)` → `render()`. Treat **`update` and `render` as hot paths**. Per-frame budget at V-Sync 60 Hz is ~16.6 ms for all work combined; at 120 Hz ~8.3 ms.

- **No *recurring* per-frame heap allocation** in `update()` / `render()`. The smell is `new` / growing `vector` / `std::string` rebuilt every tick. A one-off alloc on an infrequent event (score change, level load, hot-plug) is fine — cache the result on that event and re-read it every frame. When derived state must change every frame and has no event to hook into (FPS readout, glyph rects), prefer a reusable stack/scratch buffer; an explicit per-frame arena is the last resort, never ad-hoc `new` per tick.
- **No unbounded or heavy algorithms** (deep search, nested loops over dynamic collections, recursion). Loops should be **O(1)** or **O(n)** with **small, compile-time-known n**.
- **Think about data layout** for anything touched every frame: contiguous structs, scalars, fixed arrays. Avoid pointer chasing, `std::function`, and virtual dispatch in inner loops without need.
- **Precompute static layout once** at construction / level load, not each frame.
- **Format text on the change event, not every frame.** Hold the formatted string as state and refresh it when the underlying value changes. If there is no clear change event (debug overlay, FPS counter), format into a stack buffer via `std::to_chars` into a `std::array<char, N>`.
- **Readability over cleverness**, but not at the cost of obvious per-frame waste.

## Layering

```
main.cpp                                   → thin entry
Application                                → SDL lifetime, event pump, frame loop; injects IClock + IRandomSource (+ later: IPaddleController, …)
PlayfieldLayout, FrameTiming, PaddleMotion → pure free functions (doctest, no SDL video)
PlayfieldRenderer, TextRenderer::draw*     → thin SDL draw boundaries
Playfield::*                               → logical constants + coordinate convention
```

- **SDL stays concentrated** in `Application` and thin adapters. Gameplay math must not call SDL. `SDL_FRect` returned by value is a POD, not an SDL call.
- **Single owner per SDL API** — exactly one wrapping adapter per primitive (`ClockSdlTicks` is the only caller of `SDL_GetTicksNS`). Keeps the rest testable and the mock surface one line.
- **Inject time, randomness, input via small interfaces with sensible defaults** — owners take `unique_ptr<IFoo> = nullptr` and fall back internally to the production impl; tests pass a fake. No `SDL_GetTicks`, `rand()`, or hidden globals in gameplay code.
- **Typed range helpers, not raw `next()`** — `IRandomSource::intInRange` / `doubleInRange` rather than a `uint32_t`-returning primitive.
- **Pure logic ships with tests.** See the testing strategy in `docs/ROADMAP.md`.
- **Logical pixels everywhere** — origin top-left, +Y down; tuning in `Playfield::` constants. Matches `SDL_SetRenderLogicalPresentation`.
- **Half-extents at layout APIs** — pass `paddleHalfWidth`, not full width, so call sites express "centered on" without scattering `/ 2`.
- **Controllers / policies as data + functions** — return a request struct consumed by a pure update step; no deep inheritance trees.

## C++ shape: `constexpr`, `const`, pure functions

Keep **data** (structs, tuning, cached rects) separate from **algorithms** (collision, clamping, layout, state transitions). Algorithms are free functions taking all inputs as parameters — no reaching into `Application` or hidden globals.

- **`constexpr` for compile-time facts** — resolutions, half-extents, dash counts, glyph tables, win scores. Prefer named constants over magic numbers; `static_cast` when mixing `int` and `float`.
- **Do not `constexpr` what must stay runtime** — injected clock/random, SDL handles, vectors built from caller parameters.
- **Pure functions** = same inputs → same outputs, no I/O, no globals. `[[nodiscard]]` on pure helpers.
- **Structs hold state; free functions implement policy** — `Paddle{position, half-size, speed}` + `PaddleMotion::stepCenterY(request, …)`; `Score{value, text}` + `setScore(score, newValue)`; never a deep `Paddle::update()` that reads SDL or controllers. When two fields share an invariant (`Score::text == std::to_string(Score::value)`), expose only the setter that maintains it and treat direct field mutation as a review-flagged bug.
- **Const-correct interfaces, value types at boundaries** — `const` query methods; `const&` or pass-by-value for small PODs; return fresh values rather than mutating caller state unless the API is explicitly in-out.
- **`std::string_view` for non-owning string params.** Use `const std::string&` only when you genuinely need a `std::string` API inside.
- **Delete copy / move on owners and polymorphic bases.** Mark all four special members explicitly — slicing and accidental copies of SDL handles are bugs, not features.
- **Defend at the boundary** — pure helpers handle degenerate inputs and document the choice in the header once, then test it (`FrameTiming::secondsBetween` → `0.0` on non-monotonic input; `centerDashSegments(0, …)` → empty vector).

**Smells:** a helper that reads `Application`, SDL, or file-scope mutable state; a member function that mixes input polling with motion math; a `constexpr` on something that still allocates; an owner with implicit copy/move; `const std::string&` where `std::string_view` would do; a new SDL API called from more than one TU.

## Patterns worth copying

- **Free function + explicit inputs** — testable without a window (`FrameTiming::secondsBetween`, layout helpers).
- **Cache at construction** — `PlayfieldRenderer` builds `m_centerDashes` once; `draw()` only blits.
- **Header stays SDL-free when cheap** — forward-declare SDL types; hide heavy includes behind `unique_ptr`.
- **Separate "compute rects" from "draw rects"** — the compute half is pure and tested; the draw half is a thin SDL loop. Same shape works as a sink/callback when even the intermediate `vector` would allocate per frame (`TextRenderer::drawText` passes a `[renderer](const SDL_FRect&){…}` lambda to the shared inner helper).
- **Top-level hot-path functions read at intent level** — mechanical detail (formatting, byte packing, inner draw loops) lives in a named helper one level down. `Application::render()` reads cached `Score.text` and issues one `drawTextCentered` call per side; the `to_string`, the cache invalidation, and the per-pixel loop all live in their own helpers.
- **Seed mutable caches in `init`, not lazily** — `m_lastTickNs = m_clock->now()` so the first frame's `dt` is real.
- **Non-fatal SDL setup logs and keeps going** — V-Sync / LogicalPresentation failures fall back gracefully rather than aborting.
- **Linear scan for small lookup tables is fine when documented** — name the obvious future optimisation in a comment rather than reaching for `std::map` reflexively.
- **Placeholder code is labeled with the replacing milestone** — inline when it is a few lines, behind a clearly-named private helper (`Application::placeholderScoreDriver`) when it grows beyond that, but never a separate class or type. The name itself flags the deletion target for the future milestone.

## Review checklist

- [ ] New gameplay logic is **pure and tested**, or justified as SDL-only with a **manual smoke** note.
- [ ] `update` / `render` add **no recurring per-frame alloc** and **no unbounded work**.
- [ ] Hot-path state is **structs/scalars**, not growable containers rebuilt each frame.
- [ ] SDL surface area did not spread into math/AI/collision — each new SDL API has **one** wrapping adapter.
- [ ] Time / randomness / input go through **injected** abstractions; production defaults stay one-line.
- [ ] Constants live in **`Playfield::`** (or the entity's header), not magic numbers in the frame loop.
- [ ] Tuning uses **`constexpr`** where appropriate; gameplay math stays **pure** (explicit inputs, no globals/SDL).
- [ ] APIs are **const-correct**; structs hold state, **free functions** hold algorithms; owners / polymorphic bases delete copy/move.
- [ ] Pure helpers **defend at the boundary** and document degenerate inputs.
- [ ] Non-owning string params use `std::string_view`; non-owning byte ranges `std::span`.
- [ ] Change is **minimal** for the milestone — no speculative frameworks.