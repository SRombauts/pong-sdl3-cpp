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

> The following deliverables turn the black SDL3 window into a recognizable Pong layout: a fixed logical resolution with letterboxed scaling, the static playfield elements (paddles, ball, dashed center line) driven by layout helpers, and a placeholder score rendered via a deliberately chosen text-rendering approach that the menus milestone will reuse. Each entry below is intended to map to one pull request.

### Pick a text-rendering approach and draw the placeholder score

#### Description

The static playfield is complete except for the score readout. The roadmap explicitly calls out that the same text-rendering approach must serve the **Screens and menus** milestone (Title, Pause, Game Over, mode selection). This issue is the decision point: pick an approach, document the trade-offs in code, and implement it for the placeholder score `0 0` shown above the playfield.

#### Decision: text-rendering approach

Three candidates from the roadmap, with trade-offs surfaced for the decision:

1. **Hand-drawn 7-segment digits**. Simplest: one segment-on/off table for `0..9`, a few `SDL_RenderFillRect` calls. Zero extra dependencies, no asset to ship, trivial to unit-test (one `SUBCASE` per digit). **Limitation**: digits only — menus that need words (`PAUSE`, `GAME OVER`, `1 PLAYER`) would have to be drawn as hand-placed sprites or migrated to a font later, which means this milestone's text stack is thrown away when menus land.

2. **Small hand-drawn bitmap font (3x5 or 5x7 grid of pixels per glyph)**. A `static constexpr` glyph table covering at minimum `0..9`, plus the letters used by the menus milestone (`P`, `A`, `U`, `S`, `E`, `G`, `M`, `O`, `V`, `R`, `1`, `2`, `L`, `Y`, `N`, …). More upfront work than 7-segment, but the resulting renderer is reusable end-to-end and avoids a second text-rendering decision later. Still zero dependencies and no asset to ship.

3. **`SDL3_ttf` with a small bundled TTF font**. Most flexibility, full Unicode and antialiasing. But adds a `FetchContent` dependency, CI dev-headers on Linux (likely `libfreetype-dev`, possibly `libharfbuzz-dev`), a runtime asset path that has to be resolved on three OSes, and a font-license note in the `README.md`. Heaviest option for an arcade-style game whose menus need a handful of fixed strings.

**Recommendation**: option 2 (small bitmap font). It costs marginally more than option 1 for this milestone and pays back immediately in the menus milestone — no second text stack to ship, no rework. Option 1 is acceptable if the project is willing to revisit text rendering when menus land; option 3 is over-engineering for the scope.

This recommendation is open for revision before the implementation PR — record the final choice in `src/TextRenderer.h` as the source of truth.

#### Tasks

- Add `src/TextRenderer.{h,cpp}` with a header-level comment recording the chosen approach and the rationale (one short paragraph).
- For **option 2 (recommended)**:
  - Define a `static constexpr` glyph table for `0..9` plus the alphabet needed by the menus milestone (at minimum the letters that spell `PONG`, `PAUSE`, `GAME OVER`, `1 PLAYER`, `2 PLAYER`, `RESTART`, `QUIT`). One bit per pixel, stored as `uint8_t` rows for a 5x7 glyph or `uint16_t` rows for a 7x9 glyph.
  - Provide a layout function returning the on-pixel rectangles for a string: `std::vector<SDL_FRect> textGlyphRects(std::string_view text, float originX, float originY, float pixelSize, float glyphSpacing)`.
  - Provide a thin renderer entry point in `TextRenderer.cpp` that calls `SDL_RenderFillRect` once per returned `SDL_FRect`.
- For **option 1** (if chosen instead), the tasks collapse to: a `static constexpr` segment-on/off table for `0..9`, a pure helper returning the per-segment rectangles for a digit drawn at a given top-left position with a given segment thickness, and the thin SDL renderer.
- For **option 3** (if chosen instead), additional tasks: pin an `SDL3_ttf` tag in `FetchContent`, update the Ubuntu CI step with the new dev-headers, bundle a permissively-licensed font under `assets/` and document its license in the `README.md`, resolve the asset path on all three OSes (`SDL_GetBasePath` is the usual answer).
- Draw `0 0` (a placeholder score) centered horizontally near the top of the playfield (e.g. `y = 24` logical pixels), with the left digit slightly to the left of the center line and the right digit slightly to the right. Use the same white as the other elements.
- Append the new sources/headers to `PONG_SRC` / `PONG_INC` in `CMakeLists.txt` and to the test target's source list in `tests/CMakeLists.txt`.
- Tests under `tests/TextRendererTest.cpp` matching the chosen approach:
  - Option 1: segment-on/off pattern for each digit `0..9` (one `SUBCASE` per digit, or one parametrised table), per-segment rectangle layout for a digit drawn at a given top-left position.
  - Option 2: per-glyph pattern for each supported character (a representative subset is fine; cover at least `0`, `9`, `P`, `A`, `space`), and a multi-character layout test asserting that the total width matches `len(text) * (glyphWidth + glyphSpacing) - glyphSpacing` (no trailing spacing) and that shifting the origin shifts every output rect by the same delta.
  - Option 3: a unit-testable wrapper around glyph-position math (TTF metrics are out of scope) — at minimum verify the centering formula used to place a string in a target rectangle.

#### Acceptance criteria

- The window shows a centered `0 0` placeholder score above the playfield, in the same white as the other static elements, at the documented logical-pixel y-offset.
- `TextRenderer.h` documents the chosen approach and the rationale. The chosen approach is structurally capable of rendering every menu string listed in the roadmap's `Screens and menus` milestone (at least digits + the menu alphabet) without a second text-rendering stack.
- All new unit tests pass locally and in CI on Windows, Linux, and macOS.
- `clang-format --dry-run --Werror` stays clean on the new and edited files.
- The previous milestones' tests still pass unchanged.

#### Notes

- Whichever option is picked, the SDL boundary is exactly one function (`SDL_RenderFillRect` in option 1/2, `SDL_RenderTexture` in option 3). Keep the boundary thin so the layout math stays unit-testable.
- The glyph table in option 2 can grow incrementally: ship the digits and the menu alphabet in this PR; later milestones can extend the table as new strings appear, without restructuring the renderer.
- The placeholder score's exact font size, kerning, and y-offset are tuning knobs, not invariants. Pick reasonable values, leave them as named constants, and move on; `Scoring and match flow` is the milestone that revisits the score visuals once they show real values.
