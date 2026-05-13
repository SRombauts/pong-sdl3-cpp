---
name: test
description: Run the unit-test suite. Use to run, filter, or disable tests.
---

# Test

Tests are built by default alongside the main executable, registered with CTest via `doctest_discover_tests()`. Build the project first (see the `build` skill); the test scripts deliberately do not trigger a build.

## Run all tests

```powershell
scripts\test.ps1
```

```bash
./scripts/test.sh
```

Both default to `Debug` (`-Config Release` / `--config Release` to override). Raw fallback:

```bash
ctest --test-dir build --output-on-failure
```

## Run an individual test target or case

Filter via CTest (regex over registered test names):

```bash
ctest --test-dir build -R smoke --output-on-failure
```

Or invoke the test executable directly with doctest filters:

- Linux/macOS: `build/pong-sdl3-cpp-tests -tc=smoke`
- Windows (Visual Studio generator): `build\tests\Debug\pong-sdl3-cpp-tests.exe -tc=smoke`

## Disable testing entirely

Pass `-DBUILD_TESTING=OFF` at configure time. CMake's standard option (set by `include(CTest)`); skips both the test target and the `FetchContent` download of `doctest`:

```bash
cmake -S . -B build -DBUILD_TESTING=OFF
```

In this configuration `scripts/test.{ps1,sh}` will exit non-zero with "No tests were found": the wrappers pass `--no-tests=error` to CTest so a build without a test target fails loudly instead of silently reporting "Tests passed" on zero discovered tests. To opt out (e.g. a sanity-check run on a `BUILD_TESTING=OFF` build), forward `--no-tests=ignore`:

```powershell
scripts\test.ps1 -ExtraArgs '--no-tests=ignore'
```

```bash
./scripts/test.sh -- --no-tests=ignore
```

## Rules

- Always build before running tests; the scripts fail fast if `build/` does not exist.
- Add new tests under `tests/<TypeName>Test.cpp` (CamelCase, mirroring the type or area being tested) and let `doctest_discover_tests()` pick them up; do not edit CTest registration by hand.
- A non-zero exit from CTest means a real failure — surface it rather than re-running blindly. "No tests were found" is one such failure: it usually means the build was configured with `BUILD_TESTING=OFF`, or the test target was never built into the current `build/` (partial / stale state). Re-run `scripts/build.{ps1,sh}` rather than retrying `ctest`.
