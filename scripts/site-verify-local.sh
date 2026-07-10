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

echo "=== [5/10] Running S10 audio gate ==="
S10_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/site-audio-gate.spec.ts --project=chromium) \
    || S10_EXIT=$?

echo "=== [6/10] Running S11 preset-consistency gate ==="
S11_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/preset-consistency.spec.ts --project=chromium) \
    || S11_EXIT=$?

echo "=== [7/10] Running S13 Play↔Try It equivalence gate ==="
S13_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/dump-mode.spec.ts tests-e2e/equivalence.spec.ts --project=chromium) \
    || S13_EXIT=$?

echo "=== [8/10] Running S14 lane-mute gate ==="
S14_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/lane-mute.spec.ts --project=chromium) \
    || S14_EXIT=$?

echo "=== [9/10] Running S18 control-audit gate ==="
S18_CTRL_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/control-audit.spec.ts --project=chromium) \
    || S18_CTRL_EXIT=$?

echo "=== [10/10] Running S18 console-error gate ==="
S18_CONSOLE_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${PREVIEW_URL}" \
       npx playwright test tests-e2e/console-error-gate.spec.ts --project=chromium) \
    || S18_CONSOLE_EXIT=$?

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

# Local mirror of S18-remote-verify.json — same gate, local preview URL. If
# control-audit.spec.ts starts emitting its own summary we prefer that;
# otherwise synthesize a minimal exit-code stub so an artifact always exists.
S18_SRC="${SITE_DIR}/test-results/control-audit-summary.json"
S18_DST="${ARTIFACTS_DIR}/S18-local-verify.json"
if [ -f "${S18_SRC}" ]; then
    cp "${S18_SRC}" "${S18_DST}"
    echo "    wrote ${S18_DST}"
else
    S18_VERDICT="fail"
    [ "${S18_CTRL_EXIT}" = "0" ] && S18_VERDICT="pass"
    cat > "${S18_DST}" <<EOF
{
  "gate": "S18-control-audit",
  "url": "${PREVIEW_URL}",
  "exitCode": ${S18_CTRL_EXIT},
  "verdict": "${S18_VERDICT}",
  "spec": "site/tests-e2e/control-audit.spec.ts",
  "note": "synthesized: control-audit spec did not emit test-results/control-audit-summary.json"
}
EOF
    echo "    synthesized ${S18_DST} (no spec summary; verdict=${S18_VERDICT})"
fi

# Mirror control-audit summary handling for console-error: prefer the spec's
# JSON; synthesize an exit-code stub if it's missing so an artifact always exists.
S18_CE_SRC="${SITE_DIR}/test-results/console-error-summary.json"
S18_CE_DST="${ARTIFACTS_DIR}/S18-console-error-local-verify.json"
if [ -f "${S18_CE_SRC}" ]; then
    cp "${S18_CE_SRC}" "${S18_CE_DST}"
    echo "    wrote ${S18_CE_DST}"
else
    S18_CE_VERDICT="fail"
    [ "${S18_CONSOLE_EXIT}" = "0" ] && S18_CE_VERDICT="pass"
    cat > "${S18_CE_DST}" <<EOF
{
  "gate": "S18-console-error",
  "url": "${PREVIEW_URL}",
  "exitCode": ${S18_CONSOLE_EXIT},
  "verdict": "${S18_CE_VERDICT}",
  "spec": "site/tests-e2e/console-error-gate.spec.ts",
  "note": "synthesized: console-error-gate spec did not emit test-results/console-error-summary.json"
}
EOF
    echo "    synthesized ${S18_CE_DST} (no spec summary; verdict=${S18_CE_VERDICT})"
fi

# Combine exit codes so any gate failing fails the script.
GATE_EXIT=$(( S10_EXIT | S11_EXIT | S13_EXIT | S14_EXIT | S18_CTRL_EXIT | S18_CONSOLE_EXIT ))

if [ "${GATE_EXIT}" = "0" ]; then
    echo "=== PASS: local audio + preset-consistency + equivalence + lane-mute + S18 control-audit + console-error gates ==="
else
    echo "=== FAIL: local gates (S10 exit ${S10_EXIT}, S11 exit ${S11_EXIT}, S13 exit ${S13_EXIT}, S14 lane-mute exit ${S14_EXIT}, S18 control-audit exit ${S18_CTRL_EXIT}, S18 console-error exit ${S18_CONSOLE_EXIT}) ==="
fi
exit "${GATE_EXIT}"
