#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
BUILD_TYPE="${BUILD_TYPE:-Release}"
VST3_BUNDLE="$BUILD_DIR/VST3/$BUILD_TYPE/poly_plugin.vst3"
INSTALL_DIR="$HOME/Library/Audio/Plug-Ins/VST3"

usage() {
    cat <<EOF
Usage: $(basename "$0") [options]

Options:
  --clean       Remove build directory and reconfigure from scratch
  --configure   Force CMake reconfigure (picks up asset changes)
  --no-deploy   Build only, skip copying to VST3 folder
  --debug       Build Debug instead of Release
  --test        Run tests after build
  -j N          Parallel jobs (default: auto)
  -h, --help    Show this help

Without flags, does an incremental build + deploy.
EOF
    exit 0
}

CLEAN=0
FORCE_CONFIGURE=0
DEPLOY=1
RUN_TESTS=0
PARALLEL=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        --clean)       CLEAN=1; shift ;;
        --configure)   FORCE_CONFIGURE=1; shift ;;
        --no-deploy)   DEPLOY=0; shift ;;
        --debug)       BUILD_TYPE=Debug; shift ;;
        --test)        RUN_TESTS=1; shift ;;
        -j)            PARALLEL="$2"; shift 2 ;;
        -h|--help)     usage ;;
        *)             echo "Unknown option: $1"; usage ;;
    esac
done

VST3_BUNDLE="$BUILD_DIR/VST3/$BUILD_TYPE/poly_plugin.vst3"
PARALLEL_FLAG=""
if [[ -n "$PARALLEL" ]]; then
    PARALLEL_FLAG="--parallel $PARALLEL"
else
    PARALLEL_FLAG="--parallel"
fi

if [[ $CLEAN -eq 1 ]]; then
    echo "=== Clean build ==="
    rm -rf "$BUILD_DIR"
fi

if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]] || [[ $FORCE_CONFIGURE -eq 1 ]]; then
    echo "=== Configure ($BUILD_TYPE) ==="
    cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE" -DPOLY_WEB_UI=ON
fi

echo "=== Build ($BUILD_TYPE) ==="
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" $PARALLEL_FLAG

if [[ $RUN_TESTS -eq 1 ]]; then
    echo "=== Test ==="
    ctest --test-dir "$BUILD_DIR" --build-config "$BUILD_TYPE" --output-on-failure
fi

if [[ $DEPLOY -eq 1 ]]; then
    if [[ -d "$VST3_BUNDLE" ]]; then
        echo "=== Deploy ==="
        mkdir -p "$INSTALL_DIR"
        rm -rf "$INSTALL_DIR/poly_plugin.vst3"
        cp -R "$VST3_BUNDLE" "$INSTALL_DIR/"
        echo "Installed: $INSTALL_DIR/poly_plugin.vst3"
    else
        echo "WARNING: Bundle not found at $VST3_BUNDLE — skipping deploy"
    fi
fi

echo "=== Done ==="
