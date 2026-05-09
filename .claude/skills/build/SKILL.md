---
name: build
description: Build the project. Use to build, compile, test, or run the project.
---

# Build

Use build scripts from `scripts/`. They are thin CMake wrappers that produce an out-of-source build under `build/`. Default configuration is `Debug`.

## Windows (default: Visual Studio)

From PowerShell at the repo root:

```powershell
scripts\build.ps1
```

Useful flags: `-Config Release`, `-Clean`, `-Generator Ninja`, `-ConfigureOnly`.

From `cmd.exe` use the wrapper: `scripts\build.cmd`.

Resulting executable: `build\Debug\pong-sdl3-cpp.exe` (or `build\Release\...` for Release).

## Linux / macOS

```bash
./scripts/build.sh
```

Useful flags: `--config Release`, `--clean`, `--generator Ninja`, `--configure-only`, `--help`.

Resulting executable: `build/pong-sdl3-cpp`.

## Rules

- Do not invent custom build commands; call `scripts/build.ps1` or `scripts/build.sh`.
- A non-zero exit code means the build failed; surface the error rather than retrying blindly.
- Plain `cmake -S . -B build && cmake --build build` is an acceptable fallback if the scripts are unavailable.
