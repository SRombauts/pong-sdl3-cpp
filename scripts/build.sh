#!/usr/bin/env bash
# Configure and build pong-sdl3-cpp on Unix-like systems.
#
# Thin wrapper around CMake that performs an out-of-source build.
# Defaults to a Debug build under build/. Pass --generator to pick a
# specific CMake generator (for example Ninja).

set -euo pipefail

usage() {
    cat <<'EOF'
Usage: scripts/build.sh [options]

Options:
  -c, --config <cfg>      Build configuration (default: Debug).
                          Common values: Debug, Release, RelWithDebInfo, MinSizeRel.
  -b, --build-dir <dir>   Build directory (default: build).
  -g, --generator <name>  CMake generator (default: CMake's choice).
      --clean             Remove the build directory before configuring.
      --configure-only    Only run the CMake configure step.
  -h, --help              Show this help message.

Any further arguments after a literal "--" are forwarded to the CMake
configure step verbatim.

Examples:
  scripts/build.sh
  scripts/build.sh --config Release
  scripts/build.sh --generator Ninja --config Release
  scripts/build.sh --clean
EOF
}

config="Debug"
build_dir="build"
generator=""
clean=0
configure_only=0
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
        -g|--generator)
            [[ $# -ge 2 ]] || { echo "error: $1 requires a value" >&2; exit 2; }
            generator="$2"
            shift 2
            ;;
        --clean)
            clean=1
            shift
            ;;
        --configure-only)
            configure_only=1
            shift
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

if [[ "$clean" -eq 1 && -d "$build_path" ]]; then
    echo "Removing build directory: $build_path"
    rm -rf -- "$build_path"
fi

configure_args=(-S "$repo_root" -B "$build_path" "-DCMAKE_BUILD_TYPE=$config")
if [[ -n "$generator" ]]; then
    configure_args+=(-G "$generator")
fi
if [[ ${#extra_args[@]} -gt 0 ]]; then
    configure_args+=("${extra_args[@]}")
fi

echo "Repository root : $repo_root"
echo "Build directory : $build_path"
echo "Configuration   : $config"
[[ -n "$generator" ]] && echo "Generator       : $generator"
echo

echo "> cmake ${configure_args[*]}"
cmake "${configure_args[@]}"

if [[ "$configure_only" -eq 1 ]]; then
    echo "Configure complete. Skipping build (--configure-only)."
    exit 0
fi

build_args=(--build "$build_path" --config "$config")
echo "> cmake ${build_args[*]}"
cmake "${build_args[@]}"

echo
echo "Build succeeded ($config)."
