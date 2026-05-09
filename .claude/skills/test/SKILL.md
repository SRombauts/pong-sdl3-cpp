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

## Rules

- Always build before running tests; the scripts fail fast if `build/` does not exist.
- Add new tests under `tests/test_<area>.cpp` and let `doctest_discover_tests()` pick them up; do not edit CTest registration by hand.
- A non-zero exit from CTest means a real failure — surface it rather than re-running blindly.
