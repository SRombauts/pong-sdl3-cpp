---
name: format
description: Format and verify C/C++ source style with clang-format.
---

# Format

`.clang-format` at the repository root is the **source of truth** for C++ formatting (Allman braces, 4-space indent, 120-column limit, based on `LLVM`). `.editorconfig` covers editor-level settings (indent style, line endings, charset, final newline).

Do not invent formatting choices; let `clang-format` arbitrate.

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
