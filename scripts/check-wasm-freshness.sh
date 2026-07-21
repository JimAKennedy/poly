#!/usr/bin/env bash
set -euo pipefail

# check-wasm-freshness.sh — sha256 the deployed WASM+glue against the locally
# built artifacts and fail if they diverge.
#
# Usage:
#   bash scripts/check-wasm-freshness.sh https://poly.jk.digital/
#   POLY_SITE_URL=https://poly.jk.digital/ bash scripts/check-wasm-freshness.sh
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
#
# CI mode: emscripten is not reproducible cross-environment, so the CI-built
# WASM won't be byte-identical to the committed webui/poly_engine.wasm. In CI
# we pass the expected sha256 of the freshly-built artifacts through
# POLY_EXPECTED_WASM_SHA256 and POLY_EXPECTED_JS_SHA256 env vars; when both
# are set, they are the source of truth and the local webui/ files are not
# read. Locally (no overrides) the script hashes the committed artifacts.

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

EXPECTED_WASM_SHA="${POLY_EXPECTED_WASM_SHA256:-}"
EXPECTED_JS_SHA="${POLY_EXPECTED_JS_SHA256:-}"

# When both overrides are supplied we're in CI mode: trust the caller's shas
# and skip the local-artifact requirement entirely. Otherwise the script
# compares against the committed webui/ artifacts (local operator mode).
if [ -n "${EXPECTED_WASM_SHA}" ] && [ -n "${EXPECTED_JS_SHA}" ]; then
    LOCAL_MODE=0
else
    LOCAL_MODE=1
    LOCAL_WASM="${PROJECT_DIR}/webui/poly_engine.wasm"
    LOCAL_JS="${PROJECT_DIR}/webui/poly_engine.js"
    if [ ! -f "${LOCAL_WASM}" ] || [ ! -f "${LOCAL_JS}" ]; then
        echo "=== FAIL: local artifacts missing at webui/poly_engine.{wasm,js} — run scripts/build-wasm.sh first ===" >&2
        exit 4
    fi
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
    local expected_hash="$2"
    local expected_source="$3"
    local label="$4"
    local remote_url="${URL_TRIMMED}${remote_path}"
    local remote_file
    remote_file="${TMP_DIR}/$(basename "${remote_path}")"

    if ! curl -sfL -o "${remote_file}" "${remote_url}"; then
        echo "=== FAIL: could not fetch ${remote_url} ===" >&2
        return 5
    fi

    local remote_hash
    remote_hash="$("${SHA_CMD[@]}" "${remote_file}" | awk '{print $1}')"

    if [ "${remote_hash}" != "${expected_hash}" ]; then
        {
            echo "FAIL: WASM staleness detected — ${label}"
            echo "    path:     ${remote_path}"
            echo "    remote:   ${remote_hash}  (${remote_url})"
            echo "    expected: ${expected_hash}  (${expected_source})"
        } >&2
        return 1
    fi

    echo "    ${label}: sha256 ${remote_hash} (match)"
    return 0
}

if [ "${LOCAL_MODE}" = "1" ]; then
    echo "=== check-wasm-freshness: comparing ${URL_TRIMMED}/webui/poly_engine.{wasm,js} against local webui/ ==="
    EXPECTED_WASM_SHA="$("${SHA_CMD[@]}" "${LOCAL_WASM}" | awk '{print $1}')"
    EXPECTED_JS_SHA="$("${SHA_CMD[@]}" "${LOCAL_JS}" | awk '{print $1}')"
    WASM_SOURCE="${LOCAL_WASM}"
    JS_SOURCE="${LOCAL_JS}"
else
    echo "=== check-wasm-freshness: comparing ${URL_TRIMMED}/webui/poly_engine.{wasm,js} against POLY_EXPECTED_*_SHA256 overrides ==="
    WASM_SOURCE="POLY_EXPECTED_WASM_SHA256"
    JS_SOURCE="POLY_EXPECTED_JS_SHA256"
fi

STATUS=0
fetch_and_hash "/webui/poly_engine.wasm" "${EXPECTED_WASM_SHA}" "${WASM_SOURCE}" "poly_engine.wasm" || STATUS=$?
fetch_and_hash "/webui/poly_engine.js" "${EXPECTED_JS_SHA}" "${JS_SOURCE}" "poly_engine.js" || STATUS=$?

if [ "${STATUS}" = "0" ]; then
    echo "PASS: WASM freshness verified"
else
    echo "=== FAIL: WASM freshness gate (exit ${STATUS}) ===" >&2
fi

exit "${STATUS}"
