---
name: format
description: Format and verify C/C++ source style with clang-format.
---

# Format

`.clang-format` at the repository root is the **source of truth** for C++ formatting (Allman braces, 4-space indent, 120-column limit, based on `LLVM`). `.editorconfig` covers editor-level settings (indent style, line endings, charset, final newline).

Do not invent formatting choices; let `clang-format` arbitrate.

## Line length (120 columns)

`ColumnLimit: 120` applies to **code and comments**. LLVM's default `ReflowComments: Always` is in effect, so `clang-format -i` will reflow `//` and `/* */` comment lines toward that limit when it can.

When writing or editing comments by hand, wrap prose to the same **120-column** limit as code — do not use a narrower ~72-column habit. Account for leading indentation and the `// ` prefix (e.g. four spaces + `// ` leaves ~113 characters of text on a typical indented line in a `.cpp` function body).

"Short comment" in `repo-conventions` means **few words and no noise**, not artificially narrow lines.

## Function arguments and parameters

Two shapes are allowed, picked by line length:

1. **All on one line**, when the call or declaration fits inside `ColumnLimit: 120`:

    ```cpp
    SDL_FRect ball(int playfieldWidth, int playfieldHeight, float ballHalfSize);
    ```

2. **One per line, aligned after the open paren**, as soon as the one-line form would overflow:

    ```cpp
    SDL_FRect leftPaddle(int playfieldWidth,
                         int playfieldHeight,
                         float paddleHalfWidth,
                         float paddleHalfHeight,
                         float wallInset);
    ```

The intermediate "pack as many as fit, wrap the rest" shape (LLVM's default `BinPackParameters: true`) is deliberately rejected: the resulting ragged wrap is harder to scan in a diff and shifts every continuation line whenever an argument is added or renamed. The "all arguments on the next line as a block" fallback is also disabled, so these are the only two shapes `clang-format` produces.

This is enforced by `.clang-format` via `BinPackArguments: false`, `BinPackParameters: false`, `AllowAllArgumentsOnNextLine: false`, `AllowAllParametersOfDeclarationOnNextLine: false`. Do not hand-format wrapped argument lists; let the formatter pick the shape from those four switches.

## Apply formatting in place

```bash
clang-format -i src/main.cpp tests/*.cpp
```

The tool walks up from each file looking for a `.clang-format`, so running it from anywhere inside the repository will pick up the right config.

## Verify without rewriting (used by CI)

```bash
clang-format --dry-run --Werror src/main.cpp tests/*.cpp
```

Exit code 0 means clean. Any non-zero exit is a real diff that CI will reject.

## Scope

The formatter targets C/C++ source under `src/` and `tests/`. As more files are added (e.g. `src/<TypeName>.{h,cpp}`, `tests/<TypeName>Test.cpp`), include them in the same command.

## Rules

- `.clang-format` is the source of truth. Change the file, not individual sources, when adjusting style.
- Run `clang-format -i` before committing C++ changes; CI runs the dry-run check on every push and pull request and will fail on any unformatted hunk.
- Do not pass `--style=...` on the command line; the repository's `.clang-format` must always win.
- Editor settings (indent, EOL) come from `.editorconfig`; respect both files together.
