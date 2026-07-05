#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_DIR}/build-wasm"

if ! command -v emcc &>/dev/null; then
    if [ -f "${EMSDK:-$HOME/emsdk}/emsdk_env.sh" ]; then
        source "${EMSDK:-$HOME/emsdk}/emsdk_env.sh" 2>/dev/null
    else
        echo "Error: emcc not found. Install emsdk or set EMSDK env var." >&2
        exit 1
    fi
fi

echo "=== Configuring WASM build ==="
emcmake cmake -B "$BUILD_DIR" -DPOLY_ENGINE_ONLY=ON "$PROJECT_DIR"

echo "=== Building ==="
cmake --build "$BUILD_DIR" --target poly_wasm -j "$(nproc 2>/dev/null || sysctl -n hw.ncpu)"

WASM_FILE="$BUILD_DIR/poly_engine.wasm"
JS_FILE="$BUILD_DIR/poly_engine.js"

if [ -f "$WASM_FILE" ] && [ -f "$JS_FILE" ]; then
    WASM_SIZE=$(wc -c < "$WASM_FILE" | tr -d ' ')
    JS_SIZE=$(wc -c < "$JS_FILE" | tr -d ' ')
    echo "=== Build complete ==="
    echo "  WASM: $WASM_FILE ($WASM_SIZE bytes)"
    echo "  JS:   $JS_FILE ($JS_SIZE bytes)"
else
    echo "Error: Build artifacts not found" >&2
    exit 1
fi
