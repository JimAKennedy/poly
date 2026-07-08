#!/usr/bin/env bash
set -euo pipefail

# site-verify-local.sh — end-to-end audio gate against a locally-built site.
#
# Steps:
#   1. Guarantee emcc 3.1.61 on PATH (ensure-emsdk.sh — installs on first run).
#   2. Build WASM (scripts/build-wasm.sh).
#   3. Copy poly_engine.{js,wasm} into webui/ so `npm run copy-webui` finds them.
#   4. Build the Astro site (npm run copy-webui + npm run build).
#   5. Start `npx astro preview` in the background on POLY_PREVIEW_PORT (4322).
#   6. Poll until the preview URL responds.
#   7. Run the S10 audio gate against the local URL.
#   8. Capture test-results/audio-gate-summary.json into .gsd/artifacts/.
#   9. Tear down the preview server. Exit with the spec's exit code.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
SITE_DIR="${PROJECT_DIR}/site"
ARTIFACTS_DIR="${PROJECT_DIR}/.gsd/artifacts"
PREVIEW_PORT="${POLY_PREVIEW_PORT:-4322}"
PREVIEW_URL="http://localhost:${PREVIEW_PORT}"

# shellcheck disable=SC1091
source "${SCRIPT_DIR}/lib/ensure-emsdk.sh"

echo "=== [1/5] Building WASM ==="
bash "${SCRIPT_DIR}/build-wasm.sh"

echo "=== [2/5] Copying WASM artifacts into webui/ ==="
cp "${PROJECT_DIR}/build-wasm/poly_engine.js" "${PROJECT_DIR}/webui/poly_engine.js"
cp "${PROJECT_DIR}/build-wasm/poly_engine.wasm" "${PROJECT_DIR}/webui/poly_engine.wasm"

echo "=== [3/5] Building Astro site ==="
(cd "${SITE_DIR}" && npm run copy-webui && npm run build)

echo "=== [4/5] Starting astro preview on :${PREVIEW_PORT} ==="
PREVIEW_LOG="$(mktemp -t poly-preview.XXXXXX)"
(cd "${SITE_DIR}" && npx astro preview --port "${PREVIEW_PORT}" >"${PREVIEW_LOG}" 2>&1) &
PREVIEW_PID=$!

cleanup() {
    if kill -0 "${PREVIEW_PID}" 2>/dev/null; then
        kill "${PREVIEW_PID}" 2>/dev/null || true
        # Give it a beat to die cleanly, then reap.
        for _ in 1 2 3 4 5; do
            kill -0 "${PREVIEW_PID}" 2>/dev/null || break
            sleep 0.2
        done
        kill -9 "${PREVIEW_PID}" 2>/dev/null || true
        wait "${PREVIEW_PID}" 2>/dev/null || true
    fi
    rm -f "${PREVIEW_LOG}"
}
trap cleanup EXIT INT TERM

READY=0
for _ in $(seq 1 60); do
    if curl -sf -o /dev/null "${PREVIEW_URL}/poly/"; then
        READY=1
        break
    fi
    sleep 0.5
done
if [ "${READY}" != "1" ]; then
    echo "=== FAIL: astro preview did not become ready on :${PREVIEW_PORT} within 30s ===" >&2
    echo "--- preview log tail ---" >&2
    tail -n 40 "${PREVIEW_LOG}" >&2 || true
    exit 1
fi
echo "    preview ready at ${PREVIEW_URL}/poly/"

echo "=== [5/6] Running S10 audio gate ==="
S10_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/site-audio-gate.spec.ts --project=chromium) \
    || S10_EXIT=$?

echo "=== [6/6] Running S11 preset-consistency gate ==="
S11_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/preset-consistency.spec.ts --project=chromium) \
    || S11_EXIT=$?

echo "=== Capturing summary artifacts ==="
mkdir -p "${ARTIFACTS_DIR}"

S10_SRC="${SITE_DIR}/test-results/audio-gate-summary.json"
S10_DST="${ARTIFACTS_DIR}/S10-local-verify.json"
if [ -f "${S10_SRC}" ]; then
    cp "${S10_SRC}" "${S10_DST}"
    echo "    wrote ${S10_DST}"
else
    echo "    (no S10 summary produced — spec may have crashed before writing it)" >&2
fi

S11_SRC="${SITE_DIR}/test-results/preset-consistency-summary.json"
S11_DST="${ARTIFACTS_DIR}/S11-local-verify.json"
if [ -f "${S11_SRC}" ]; then
    cp "${S11_SRC}" "${S11_DST}"
    echo "    wrote ${S11_DST}"
else
    echo "    (no S11 summary produced — spec may have crashed before writing it)" >&2
fi

# Combine exit codes so either gate failing fails the script.
GATE_EXIT=$(( S10_EXIT | S11_EXIT ))

if [ "${GATE_EXIT}" = "0" ]; then
    echo "=== PASS: local audio + preset-consistency gates ==="
else
    echo "=== FAIL: local gates (S10 exit ${S10_EXIT}, S11 exit ${S11_EXIT}) ==="
fi
exit "${GATE_EXIT}"
