#!/usr/bin/env bash
set -euo pipefail

# site-verify-remote.sh — run the S10 audio gate against a deployed URL.
#
# Usage:
#   bash scripts/site-verify-remote.sh https://user.github.io/poly/
#   POLY_SITE_URL=https://staging.example/poly/ bash scripts/site-verify-remote.sh
#
# Positional argument wins over $POLY_SITE_URL. Missing both is a hard fail (exit 2).
#
# Playwright config (site/playwright.config.ts) reads POLY_SITE_URL as its
# baseURL. Only the origin is used — spec paths start with /poly/... — so
# passing either "https://user.github.io/poly/" or "https://user.github.io"
# resolves the same way.

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

Example (GitHub Pages):
  bash scripts/site-verify-remote.sh https://user.github.io/poly/
EOF
    exit 2
fi

echo "=== Running S10 audio gate against ${URL} ==="
S10_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${URL}" \
       npx playwright test tests-e2e/site-audio-gate.spec.ts --project=chromium) \
    || S10_EXIT=$?

echo "=== Running S11 preset-consistency gate against ${URL} ==="
S11_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${URL}" \
       npx playwright test tests-e2e/preset-consistency.spec.ts --project=chromium) \
    || S11_EXIT=$?

echo "=== Running S13 Play↔Try It equivalence gate against ${URL} ==="
S13_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${URL}" \
       npx playwright test tests-e2e/dump-mode.spec.ts tests-e2e/equivalence.spec.ts --project=chromium) \
    || S13_EXIT=$?

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

echo "=== Capturing summary artifacts ==="
mkdir -p "${ARTIFACTS_DIR}"

S10_SRC="${SITE_DIR}/test-results/audio-gate-summary.json"
S10_DST="${ARTIFACTS_DIR}/S10-remote-verify.json"
if [ -f "${S10_SRC}" ]; then
    cp "${S10_SRC}" "${S10_DST}"
    echo "    wrote ${S10_DST}"
else
    echo "    (no S10 summary produced — spec may have crashed before writing it)" >&2
fi

S11_SRC="${SITE_DIR}/test-results/preset-consistency-summary.json"
S11_DST="${ARTIFACTS_DIR}/S11-remote-verify.json"
if [ -f "${S11_SRC}" ]; then
    cp "${S11_SRC}" "${S11_DST}"
    echo "    wrote ${S11_DST}"
else
    echo "    (no S11 summary produced — spec may have crashed before writing it)" >&2
fi

# S18-remote-verify.json must exist on both pass and fail paths (T01 plan
# verification). control-audit.spec.ts does not currently emit a summary of
# its own, so synthesize a minimal one from the gate exit code. If the spec
# ever grows a summary writer at test-results/control-audit-summary.json,
# that will be preferred and the synthesis path will be skipped.
S18_SRC="${SITE_DIR}/test-results/control-audit-summary.json"
S18_DST="${ARTIFACTS_DIR}/S18-remote-verify.json"
if [ -f "${S18_SRC}" ]; then
    cp "${S18_SRC}" "${S18_DST}"
    echo "    wrote ${S18_DST}"
else
    S18_VERDICT="fail"
    [ "${S18_CTRL_EXIT}" = "0" ] && S18_VERDICT="pass"
    cat > "${S18_DST}" <<EOF
{
  "gate": "S18-control-audit",
  "url": "${URL}",
  "exitCode": ${S18_CTRL_EXIT},
  "verdict": "${S18_VERDICT}",
  "spec": "site/tests-e2e/control-audit.spec.ts",
  "note": "synthesized: control-audit spec did not emit test-results/control-audit-summary.json"
}
EOF
    echo "    synthesized ${S18_DST} (no spec summary; verdict=${S18_VERDICT})"
fi

GATE_EXIT=$(( S10_EXIT | S11_EXIT | S13_EXIT | S14_EXIT | S18_CTRL_EXIT ))

if [ "${GATE_EXIT}" = "0" ]; then
    echo "=== PASS: remote audio + preset-consistency + equivalence + lane-mute + S18 control-audit gates ==="
else
    echo "=== FAIL: remote gates (S10 exit ${S10_EXIT}, S11 exit ${S11_EXIT}, S13 exit ${S13_EXIT}, S14 lane-mute exit ${S14_EXIT}, S18 control-audit exit ${S18_CTRL_EXIT}) ==="
fi
exit "${GATE_EXIT}"
