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
GATE_EXIT=0
(cd "${SITE_DIR}" \
    && POLY_SITE_URL="${URL}" \
       npx playwright test tests-e2e/site-audio-gate.spec.ts --project=chromium) \
    || GATE_EXIT=$?

echo "=== Capturing summary artifact ==="
mkdir -p "${ARTIFACTS_DIR}"
SUMMARY_SRC="${SITE_DIR}/test-results/audio-gate-summary.json"
SUMMARY_DST="${ARTIFACTS_DIR}/S10-remote-verify.json"
if [ -f "${SUMMARY_SRC}" ]; then
    cp "${SUMMARY_SRC}" "${SUMMARY_DST}"
    echo "    wrote ${SUMMARY_DST}"
else
    echo "    (no summary file produced — spec may have crashed before writing it)" >&2
fi

if [ "${GATE_EXIT}" = "0" ]; then
    echo "=== PASS: remote audio gate ==="
else
    echo "=== FAIL: remote audio gate (exit ${GATE_EXIT}) ==="
fi
exit "${GATE_EXIT}"
