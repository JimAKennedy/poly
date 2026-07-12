#!/usr/bin/env bash
set -euo pipefail

# site-verify-local.sh — end-to-end audio + UI gates against a locally-built site.
#
# Steps:
#   1. Guarantee emcc 3.1.61 on PATH (ensure-emsdk.sh — installs on first run).
#   2. Build WASM (scripts/build-wasm.sh).
#   3. Copy poly_engine.{js,wasm} into webui/ so `npm run copy-webui` finds them.
#   4. Build the Astro site (npm run copy-webui + npm run build).
#   5. Start `npx astro preview` in the background on POLY_PREVIEW_PORT (4322).
#   6. Poll until the preview URL responds.
#   7. Run each Playwright gate in turn, capturing its summary JSON into
#      .gsd/artifacts/ immediately after — Playwright clears test-results/ at
#      the start of every invocation, so late captures would clobber earlier
#      ones.
#   8. Tear down the preview server. Exit with the OR'd exit codes.

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

mkdir -p "${ARTIFACTS_DIR}"

# region:per-spec-capture
# Each `npx playwright test` clears test-results/ at start, so each gate's
# summary must be copied into .gsd/artifacts/ before the next spec runs.
capture_summary() {
    local src="$1" dst="$2"
    if [ -f "${src}" ]; then
        cp "${src}" "${dst}"
        echo "    wrote ${dst}"
    else
        echo "    (no summary at ${src} — spec may have crashed before writing it)" >&2
    fi
}
# endregion:per-spec-capture

capture_or_synthesize() {
    local src="$1" dst="$2" gate="$3" url="$4" exit_code="$5" spec="$6"
    if [ -f "${src}" ]; then
        cp "${src}" "${dst}"
        echo "    wrote ${dst}"
    else
        local verdict="fail"
        [ "${exit_code}" = "0" ] && verdict="pass"
        cat > "${dst}" <<EOF
{
  "gate": "${gate}",
  "url": "${url}",
  "exitCode": ${exit_code},
  "verdict": "${verdict}",
  "spec": "${spec}",
  "note": "synthesized: spec did not emit $(basename "${src}")"
}
EOF
        echo "    synthesized ${dst} (no spec summary; verdict=${verdict})"
    fi
}

echo "=== [5/13] Running S10 audio gate ==="
S10_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/site-audio-gate.spec.ts --project=chromium) \
    || S10_EXIT=$?
capture_summary \
    "${SITE_DIR}/test-results/audio-gate-summary.json" \
    "${ARTIFACTS_DIR}/S10-local-verify.json"

echo "=== [6/13] Running S11 preset-consistency gate ==="
S11_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/preset-consistency.spec.ts --project=chromium) \
    || S11_EXIT=$?
capture_summary \
    "${SITE_DIR}/test-results/preset-consistency-summary.json" \
    "${ARTIFACTS_DIR}/S11-local-verify.json"

echo "=== [7/13] Running S13 Play↔Try It equivalence gate ==="
S13_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/dump-mode.spec.ts tests-e2e/equivalence.spec.ts --project=chromium) \
    || S13_EXIT=$?

echo "=== [8/13] Running sample-selection equivalence gate (card vs Try It per-note file parity) ==="
SAMPLE_EQ_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/sample-equivalence.spec.ts --project=chromium) \
    || SAMPLE_EQ_EXIT=$?
capture_or_synthesize \
    "${SITE_DIR}/test-results/sample-equivalence-summary.json" \
    "${ARTIFACTS_DIR}/sample-equivalence-local-verify.json" \
    "sample-equivalence" "${PREVIEW_URL}" "${SAMPLE_EQ_EXIT}" \
    "site/tests-e2e/sample-equivalence.spec.ts"

echo "=== [9/13] Running S15-S16 macro-diff gate (complexity + density + swing) ==="
S15_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/macro-diff.spec.ts --project=chromium) \
    || S15_EXIT=$?
capture_or_synthesize \
    "${SITE_DIR}/test-results/macro-diff-summary.json" \
    "${ARTIFACTS_DIR}/S15-macro-diff-local-verify.json" \
    "S15-S16-macro-diff" "${PREVIEW_URL}" "${S15_EXIT}" \
    "site/tests-e2e/macro-diff.spec.ts"

echo "=== [10/13] Running S06 first-bar-parity gate (no synth fallback during first bar) ==="
S06_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/first-bar-parity.spec.ts --project=chromium) \
    || S06_EXIT=$?
capture_or_synthesize \
    "${SITE_DIR}/test-results/first-bar-parity-summary.json" \
    "${ARTIFACTS_DIR}/S06-first-bar-parity-local-verify.json" \
    "S06-first-bar-parity" "${PREVIEW_URL}" "${S06_EXIT}" \
    "site/tests-e2e/first-bar-parity.spec.ts"

echo "=== [11/13] Running S14 lane-mute gate ==="
S14_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/lane-mute.spec.ts --project=chromium) \
    || S14_EXIT=$?

echo "=== [12/13] Running S18 control-audit gate ==="
S18_CTRL_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/control-audit.spec.ts --project=chromium) \
    || S18_CTRL_EXIT=$?
capture_or_synthesize \
    "${SITE_DIR}/test-results/control-audit-summary.json" \
    "${ARTIFACTS_DIR}/S18-local-verify.json" \
    "S18-control-audit" "${PREVIEW_URL}" "${S18_CTRL_EXIT}" \
    "site/tests-e2e/control-audit.spec.ts"

echo "=== [13/13] Running S18 console-error gate ==="
S18_CONSOLE_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/console-error-gate.spec.ts --project=chromium) \
    || S18_CONSOLE_EXIT=$?
capture_or_synthesize \
    "${SITE_DIR}/test-results/console-error-summary.json" \
    "${ARTIFACTS_DIR}/S18-console-error-local-verify.json" \
    "S18-console-error" "${PREVIEW_URL}" "${S18_CONSOLE_EXIT}" \
    "site/tests-e2e/console-error-gate.spec.ts"

# Combine exit codes so any gate failing fails the script.
GATE_EXIT=$(( S10_EXIT | S11_EXIT | S13_EXIT | SAMPLE_EQ_EXIT | S15_EXIT | S06_EXIT | S14_EXIT | S18_CTRL_EXIT | S18_CONSOLE_EXIT ))

if [ "${GATE_EXIT}" = "0" ]; then
    echo "=== PASS: local audio + preset-consistency + equivalence + sample-equivalence + macro-diff (complexity + density + swing) + first-bar-parity + lane-mute + S18 control-audit + console-error gates ==="
else
    echo "=== FAIL: local gates (S10 exit ${S10_EXIT}, S11 exit ${S11_EXIT}, S13 exit ${S13_EXIT}, sample-equivalence exit ${SAMPLE_EQ_EXIT}, S15-S16 macro-diff exit ${S15_EXIT}, S06 first-bar-parity exit ${S06_EXIT}, S14 lane-mute exit ${S14_EXIT}, S18 control-audit exit ${S18_CTRL_EXIT}, S18 console-error exit ${S18_CONSOLE_EXIT}) ==="
fi
exit "${GATE_EXIT}"
