#!/usr/bin/env bash
set -euo pipefail

# site-verify-remote.sh — run the S10 audio gate against a deployed URL.
#
# Usage:
#   bash scripts/site-verify-remote.sh https://poly.jk.digital/
#   POLY_SITE_URL=https://staging.example/ bash scripts/site-verify-remote.sh
#
# Positional argument wins over $POLY_SITE_URL. Missing both is a hard fail (exit 2).
#
# Playwright config (site/playwright.config.ts) reads POLY_SITE_URL as its
# baseURL. Spec paths are root-relative (e.g. /03-afro-cuban/) since the
# custom-domain deploy serves at the root — passing the origin with or
# without trailing slash resolves the same way.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
SITE_DIR="${PROJECT_DIR}/site"
ARTIFACTS_DIR="${PROJECT_DIR}/.gsd/artifacts"

URL="${1:-${POLY_SITE_URL:-}}"
if [ -z "${URL}" ]; then
    cat >&2 <<'EOF'
=== FAIL: no URL provided ===
Usage: bash scripts/site-verify-remote.sh <URL>
       POLY_SITE_URL=<URL> bash scripts/site-verify-remote.sh

Example (custom domain):
  bash scripts/site-verify-remote.sh https://poly.jk.digital/
EOF
    exit 2
fi

mkdir -p "${ARTIFACTS_DIR}"

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

echo "=== Running S10 audio gate against ${URL} ==="
S10_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${URL}" \
       npx playwright test tests-e2e/site-audio-gate.spec.ts --project=chromium) \
    || S10_EXIT=$?
capture_summary \
    "${SITE_DIR}/test-results/audio-gate-summary.json" \
    "${ARTIFACTS_DIR}/S10-remote-verify.json"

# Runs BEFORE S11/S13/S14/S18 so a stale-deploy failure is diagnosed at the
# right layer — otherwise downstream specs would either fail on symptoms
# ("morph doesn't reach engine") or, worse, pass silently against stale
# behavior that Play↔Try It equivalence cannot detect (both surfaces load
# the same stale WASM). Remote-only per T02 plan.
echo "=== Running S18 WASM freshness gate against ${URL} ==="
S18_WASM_EXIT=0
bash "${SCRIPT_DIR}/check-wasm-freshness.sh" "${URL}" || S18_WASM_EXIT=$?

echo "=== Running S11 preset-consistency gate against ${URL} ==="
S11_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${URL}" \
       npx playwright test tests-e2e/preset-consistency.spec.ts --project=chromium) \
    || S11_EXIT=$?
capture_summary \
    "${SITE_DIR}/test-results/preset-consistency-summary.json" \
    "${ARTIFACTS_DIR}/S11-remote-verify.json"

echo "=== Running S13 Play↔Try It equivalence gate against ${URL} ==="
S13_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${URL}" \
       npx playwright test tests-e2e/dump-mode.spec.ts tests-e2e/equivalence.spec.ts --project=chromium) \
    || S13_EXIT=$?

echo "=== Running sample-selection equivalence gate against ${URL} ==="
SAMPLE_EQ_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${URL}" \
       npx playwright test tests-e2e/sample-equivalence.spec.ts --project=chromium) \
    || SAMPLE_EQ_EXIT=$?
capture_or_synthesize \
    "${SITE_DIR}/test-results/sample-equivalence-summary.json" \
    "${ARTIFACTS_DIR}/sample-equivalence-remote-verify.json" \
    "sample-equivalence" "${URL}" "${SAMPLE_EQ_EXIT}" \
    "site/tests-e2e/sample-equivalence.spec.ts"

echo "=== Running S15-S16 macro-diff gate (complexity + density + swing) against ${URL} ==="
S15_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${URL}" \
       npx playwright test tests-e2e/macro-diff.spec.ts --project=chromium) \
    || S15_EXIT=$?
capture_or_synthesize \
    "${SITE_DIR}/test-results/macro-diff-summary.json" \
    "${ARTIFACTS_DIR}/S15-macro-diff-remote-verify.json" \
    "S15-S16-macro-diff" "${URL}" "${S15_EXIT}" \
    "site/tests-e2e/macro-diff.spec.ts"

echo "=== Running S06 first-bar-parity gate (no synth fallback during first bar) against ${URL} ==="
S06_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${URL}" \
       npx playwright test tests-e2e/first-bar-parity.spec.ts --project=chromium) \
    || S06_EXIT=$?
capture_or_synthesize \
    "${SITE_DIR}/test-results/first-bar-parity-summary.json" \
    "${ARTIFACTS_DIR}/S06-first-bar-parity-remote-verify.json" \
    "S06-first-bar-parity" "${URL}" "${S06_EXIT}" \
    "site/tests-e2e/first-bar-parity.spec.ts"

echo "=== Running S14 lane-mute gate against ${URL} ==="
S14_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${URL}" \
       npx playwright test tests-e2e/lane-mute.spec.ts --project=chromium) \
    || S14_EXIT=$?

echo "=== Running S18 control-audit gate against ${URL} ==="
S18_CTRL_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${URL}" \
       npx playwright test tests-e2e/control-audit.spec.ts --project=chromium) \
    || S18_CTRL_EXIT=$?
capture_or_synthesize \
    "${SITE_DIR}/test-results/control-audit-summary.json" \
    "${ARTIFACTS_DIR}/S18-remote-verify.json" \
    "S18-control-audit" "${URL}" "${S18_CTRL_EXIT}" \
    "site/tests-e2e/control-audit.spec.ts"

echo "=== Running S18 console-error gate against ${URL} ==="
S18_CONSOLE_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${URL}" \
       npx playwright test tests-e2e/console-error-gate.spec.ts --project=chromium) \
    || S18_CONSOLE_EXIT=$?
capture_or_synthesize \
    "${SITE_DIR}/test-results/console-error-summary.json" \
    "${ARTIFACTS_DIR}/S18-console-error-remote-verify.json" \
    "S18-console-error" "${URL}" "${S18_CONSOLE_EXIT}" \
    "site/tests-e2e/console-error-gate.spec.ts"

GATE_EXIT=$(( S10_EXIT | S18_WASM_EXIT | S11_EXIT | S13_EXIT | SAMPLE_EQ_EXIT | S15_EXIT | S06_EXIT | S14_EXIT | S18_CTRL_EXIT | S18_CONSOLE_EXIT ))

if [ "${GATE_EXIT}" = "0" ]; then
    echo "=== PASS: remote audio + WASM freshness + preset-consistency + equivalence + sample-equivalence + macro-diff (complexity + density + swing) + first-bar-parity + lane-mute + S18 control-audit + console-error gates ==="
else
    echo "=== FAIL: remote gates (S10 exit ${S10_EXIT}, S18 WASM freshness exit ${S18_WASM_EXIT}, S11 exit ${S11_EXIT}, S13 exit ${S13_EXIT}, sample-equivalence exit ${SAMPLE_EQ_EXIT}, S15-S16 macro-diff exit ${S15_EXIT}, S06 first-bar-parity exit ${S06_EXIT}, S14 lane-mute exit ${S14_EXIT}, S18 control-audit exit ${S18_CTRL_EXIT}, S18 console-error exit ${S18_CONSOLE_EXIT}) ==="
fi
exit "${GATE_EXIT}"
