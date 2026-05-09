#!/usr/bin/env bash
# Run the pong-sdl3-cpp test suite via CTest on Unix-like systems.
#
# Thin wrapper around CTest. Assumes the project has already been
# configured and built (for example via scripts/build.sh). Fails fast
# with a clear message if the build directory does not exist, instead
# of silently triggering a build.
#
# Any arguments passed after a literal "--" are forwarded verbatim to
# ctest, so options like -R <regex>, -j <N>, --rerun-failed, etc. work
# without script changes.

set -euo pipefail

usage() {
    cat <<'EOF'
Usage: scripts/test.sh [options] [-- <ctest-args>...]

Options:
  -c, --config <cfg>      Build configuration to test (default: Debug).
                          Required for multi-config generators; harmless
                          for single-config generators.
  -b, --build-dir <dir>   Build directory (default: build).
  -h, --help              Show this help message.

Examples:
  scripts/test.sh
  scripts/test.sh --config Release
  scripts/test.sh -- -R smoke -j 4
EOF
}

config="Debug"
build_dir="build"
extra_args=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        -c|--config)
            [[ $# -ge 2 ]] || { echo "error: $1 requires a value" >&2; exit 2; }
            config="$2"
            shift 2
            ;;
        -b|--build-dir)
            [[ $# -ge 2 ]] || { echo "error: $1 requires a value" >&2; exit 2; }
            build_dir="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        --)
            shift
            extra_args+=("$@")
            break
            ;;
        *)
            echo "error: unknown argument: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd "$script_dir/.." && pwd)"

if [[ "$build_dir" = /* ]]; then
    build_path="$build_dir"
else
    build_path="$repo_root/$build_dir"
fi

if [[ ! -d "$build_path" ]]; then
    echo "error: build directory not found: $build_path" >&2
    echo "       build the project first (for example: scripts/build.sh)" >&2
    exit 1
fi

ctest_args=(--test-dir "$build_path" --output-on-failure -C "$config")
if [[ ${#extra_args[@]} -gt 0 ]]; then
    ctest_args+=("${extra_args[@]}")
fi

echo "Repository root : $repo_root"
echo "Build directory : $build_path"
echo "Configuration   : $config"
echo

echo "> ctest ${ctest_args[*]}"
ctest "${ctest_args[@]}"

echo
echo "Tests passed ($config)."
