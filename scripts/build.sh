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
  --deploy      Build + install the VST3 to the user plugin folder for Cubase
                testing ($HOME/Library/Audio/Plug-Ins/VST3). This is the default,
                so the flag is mainly for explicitness in Cubase-testing workflows.
  --clean       Remove build directory and reconfigure from scratch
  --configure   Force CMake reconfigure (picks up asset changes)
  --no-deploy   Build only, skip copying to VST3 folder
  --debug       Build Debug instead of Release
  --test        Run tests after build
  -j N          Parallel jobs (default: auto)
  -h, --help    Show this help

Without flags, does an incremental build + deploy (same as --deploy).
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
        --deploy)      DEPLOY=1; shift ;;
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
    cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
fi

echo "=== Build ($BUILD_TYPE) ==="
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" $PARALLEL_FLAG

if [[ $RUN_TESTS -eq 1 ]]; then
    echo "=== Test ==="
    ctest --test-dir "$BUILD_DIR" --build-config "$BUILD_TYPE" --output-on-failure
fi

if [[ $DEPLOY -eq 1 ]]; then
    if [[ -d "$VST3_BUNDLE" ]]; then
        echo "=== Deploy (Cubase) ==="
        mkdir -p "$INSTALL_DIR"
        rm -rf "$INSTALL_DIR/poly_plugin.vst3"
        cp -R "$VST3_BUNDLE" "$INSTALL_DIR/"
        # Report both sizes so a stale install (the M073 "disappearing column"
        # scare — a fresh build that never reached the system folder) is caught
        # here instead of during a confusing Cubase session. These must match.
        built_sz=$(find "$VST3_BUNDLE" -type f -exec cat {} + | wc -c | tr -d ' ')
        inst_sz=$(find "$INSTALL_DIR/poly_plugin.vst3" -type f -exec cat {} + | wc -c | tr -d ' ')
        echo "Installed: $INSTALL_DIR/poly_plugin.vst3"
        echo "  built size:     $built_sz bytes"
        echo "  installed size: $inst_sz bytes"
        if [[ "$built_sz" != "$inst_sz" ]]; then
            echo "ERROR: installed size does not match built bundle — install may be stale" >&2
            exit 1
        fi
        echo "Rescan Poly in Cubase to pick up this build."
    else
        echo "WARNING: Bundle not found at $VST3_BUNDLE — skipping deploy"
    fi
fi

echo "=== Done ==="
