#!/usr/bin/env bash
set -euo pipefail

# check-wasm-freshness.sh — sha256 the deployed WASM+glue against the locally
# built artifacts and fail if they diverge.
#
# Usage:
#   bash scripts/check-wasm-freshness.sh https://user.github.io/poly/
#   POLY_SITE_URL=https://user.github.io/poly/ bash scripts/check-wasm-freshness.sh
#
# Positional argument wins over $POLY_SITE_URL. Missing both is a hard fail
# (exit 2). Remote-only by design: running against a local preview URL that
# serves the same file we hash against would be tautological.
#
# We compare BOTH poly_engine.wasm and poly_engine.js because the emscripten
# glue exports must match the wasm binary — a stale glue with a fresh wasm
# (or vice versa) is a real deploy hazard that the S13 equivalence gate
# cannot catch (Play and Try It would still agree by construction; they'd
# just agree on stale behavior).

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

URL="${1:-${POLY_SITE_URL:-}}"
if [ -z "${URL}" ]; then
    cat >&2 <<'EOF'
=== FAIL: no URL provided ===
Usage: bash scripts/check-wasm-freshness.sh <URL>
       POLY_SITE_URL=<URL> bash scripts/check-wasm-freshness.sh
EOF
    exit 2
fi

# Detect sha256 tool: macOS ships `shasum`, Linux ships `sha256sum`.
if command -v sha256sum >/dev/null 2>&1; then
    SHA_CMD=(sha256sum)
elif command -v shasum >/dev/null 2>&1; then
    SHA_CMD=(shasum -a 256)
else
    echo "=== FAIL: neither sha256sum nor shasum available ===" >&2
    exit 3
fi

# Local artifacts must exist. The script's job is comparing them to remote;
# if they're missing, the caller forgot to `bash scripts/build-wasm.sh` first
# and no comparison is possible.
LOCAL_WASM="${PROJECT_DIR}/webui/poly_engine.wasm"
LOCAL_JS="${PROJECT_DIR}/webui/poly_engine.js"
if [ ! -f "${LOCAL_WASM}" ] || [ ! -f "${LOCAL_JS}" ]; then
    echo "=== FAIL: local artifacts missing at webui/poly_engine.{wasm,js} — run scripts/build-wasm.sh first ===" >&2
    exit 4
fi

# Strip trailing slash for consistent URL joining.
URL_TRIMMED="${URL%/}"

TMP_DIR="$(mktemp -d -t poly-wasm-freshness.XXXXXX)"
cleanup() {
    rm -rf "${TMP_DIR}"
}
trap cleanup EXIT INT TERM

fetch_and_hash() {
    local remote_path="$1"
    local local_file="$2"
    local label="$3"
    local remote_url="${URL_TRIMMED}${remote_path}"
    local remote_file
    remote_file="${TMP_DIR}/$(basename "${local_file}")"

    if ! curl -sfL -o "${remote_file}" "${remote_url}"; then
        echo "=== FAIL: could not fetch ${remote_url} ===" >&2
        return 5
    fi

    local remote_hash
    local local_hash
    remote_hash="$("${SHA_CMD[@]}" "${remote_file}" | awk '{print $1}')"
    local_hash="$("${SHA_CMD[@]}" "${local_file}" | awk '{print $1}')"

    if [ "${remote_hash}" != "${local_hash}" ]; then
        {
            echo "FAIL: WASM staleness detected — ${label}"
            echo "    path:   ${remote_path}"
            echo "    remote: ${remote_hash}  (${remote_url})"
            echo "    local:  ${local_hash}  (${local_file})"
        } >&2
        return 1
    fi

    echo "    ${label}: sha256 ${remote_hash} (match)"
    return 0
}

echo "=== check-wasm-freshness: comparing ${URL_TRIMMED}/webui/poly_engine.{wasm,js} against local build ==="

STATUS=0
fetch_and_hash "/webui/poly_engine.wasm" "${LOCAL_WASM}" "poly_engine.wasm" || STATUS=$?
fetch_and_hash "/webui/poly_engine.js" "${LOCAL_JS}" "poly_engine.js" || STATUS=$?

if [ "${STATUS}" = "0" ]; then
    echo "PASS: WASM freshness verified"
else
    echo "=== FAIL: WASM freshness gate (exit ${STATUS}) ===" >&2
fi

exit "${STATUS}"
