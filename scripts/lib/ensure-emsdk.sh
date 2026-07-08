#!/usr/bin/env bash
# ensure-emsdk.sh — sourceable helper that guarantees emcc 3.1.61 is on PATH.
#
# Usage (from a project script):
#   source "$(dirname "$0")/lib/ensure-emsdk.sh"
#
# Logic:
#   1. If `emcc --version` reports 3.1.61 → return.
#   2. Else if ${EMSDK:-$HOME/emsdk}/emsdk_env.sh exists → source, retry (1).
#   3. Else clone emsdk to .gsd/tools/emsdk (shallow), install + activate 3.1.61,
#      source ./emsdk_env.sh.
#   4. Verify emcc --version reports 3.1.61 — fail hard on mismatch.
#
# The pin (3.1.61) matches .github/workflows/{ci,deploy-site}.yml. A locally
# installed 4.x drifts from CI and can reintroduce the growable-memory
# ArrayBuffer bug that the S10 T01 memory-growth=0 change just fixed.

POLY_EMSDK_VERSION="3.1.61"

_ensure_emsdk_check_version() {
    command -v emcc >/dev/null 2>&1 || return 1
    local ver
    ver=$(emcc --version 2>/dev/null | head -n1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' | head -n1)
    [ "$ver" = "$POLY_EMSDK_VERSION" ]
}

_ensure_emsdk_main() {
    if _ensure_emsdk_check_version; then
        echo "[ensure-emsdk] emcc ${POLY_EMSDK_VERSION} already on PATH" >&2
        return 0
    fi

    local emsdk_home="${EMSDK:-$HOME/emsdk}"
    if [ -f "${emsdk_home}/emsdk_env.sh" ]; then
        # shellcheck disable=SC1091
        source "${emsdk_home}/emsdk_env.sh" >/dev/null 2>&1 || true
        if _ensure_emsdk_check_version; then
            echo "[ensure-emsdk] sourced ${emsdk_home} (emcc ${POLY_EMSDK_VERSION})" >&2
            return 0
        fi
        echo "[ensure-emsdk] ${emsdk_home} present but wrong version — falling through to project install" >&2
    fi

    local script_dir root_dir emsdk_dir
    script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    root_dir="$(cd "${script_dir}/../.." && pwd)"
    emsdk_dir="${root_dir}/.gsd/tools/emsdk"

    if [ -f "${emsdk_dir}/emsdk_env.sh" ]; then
        # shellcheck disable=SC1091
        source "${emsdk_dir}/emsdk_env.sh" >/dev/null 2>&1 || true
        if _ensure_emsdk_check_version; then
            echo "[ensure-emsdk] sourced ${emsdk_dir} (emcc ${POLY_EMSDK_VERSION})" >&2
            return 0
        fi
    fi

    if [ ! -d "${emsdk_dir}" ]; then
        echo "[ensure-emsdk] cloning emsdk to ${emsdk_dir} ..." >&2
        mkdir -p "$(dirname "${emsdk_dir}")"
        if ! git clone --depth 1 https://github.com/emscripten-core/emsdk.git "${emsdk_dir}" >&2; then
            echo "[ensure-emsdk] FAILED: git clone" >&2
            return 1
        fi
    fi

    if ! (cd "${emsdk_dir}" && ./emsdk install "${POLY_EMSDK_VERSION}" && ./emsdk activate "${POLY_EMSDK_VERSION}") >&2; then
        echo "[ensure-emsdk] FAILED: install/activate ${POLY_EMSDK_VERSION}" >&2
        return 1
    fi

    # shellcheck disable=SC1091
    source "${emsdk_dir}/emsdk_env.sh" >/dev/null 2>&1

    if ! _ensure_emsdk_check_version; then
        echo "[ensure-emsdk] FAILED: emcc version mismatch after install" >&2
        command -v emcc >/dev/null 2>&1 && emcc --version | head -n1 >&2
        return 1
    fi
    echo "[ensure-emsdk] activated project emsdk at ${emsdk_dir} (emcc ${POLY_EMSDK_VERSION})" >&2
    return 0
}

_ensure_emsdk_main
