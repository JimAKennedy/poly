#!/usr/bin/env bash
# check-doc-conformance.sh
#
# Single source of truth for the doc-conformance / audit-ledger guardrail
# suite. Invoked by BOTH .github/workflows/ci.yml (site-lint job) and
# scripts/pre-push-check.sh so the CI list and the pre-push list are one list
# and cannot drift.
#
# Runs the full guardrail node-test suite and exits non-zero if any test
# fails. `node --test` names the exact failing file/case in its output, so a
# reintroduced wrong Euclidean pattern, a dangling preset name, a dropped
# audit-ledger row, or a broken prose-claim helper surfaces the offending
# file directly.
#
# Two families of tests ride this runner:
#   * site/tests/**  — M071 doc-conformance guardrails (import js-yaml from
#                      site/node_modules). CI runs `npm ci` before this step;
#                      callers that cannot guarantee site/node_modules (the
#                      pre-push hook) should gate on its presence.
#   * docs/audits/** — M001 S01 audit-ledger completeness gates (parity
#                      matrix, gap-closure plan, theory-audit remediation
#                      ledger). Zero external module deps.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# The guardrail suite. Adding a guardrail test means adding it here —
# site/tests/doc-conformance-wiring.test.mjs asserts that every required file
# is named in this runner, so an omission fails that wiring test rather than
# silently dropping coverage.
TESTS=(
    site/tests/euclidean-claims.test.mjs
    site/tests/appendix-euclidean-claims.test.mjs
    site/tests/prose-pattern-claims.test.mjs
    site/tests/chapter-euclidean-guardrail.test.mjs
    site/tests/polypatch-preset-resolution.test.mjs
    site/tests/preset-table-conformance.test.mjs
    site/tests/preset-taxonomy-conformance.test.mjs
    site/tests/prose-conformance-claims.test.mjs
    site/tests/idiom-break-framing.test.mjs
    site/tests/theory-euclidean-guardrail.test.mjs
    site/tests/theory-patch-conformance.test.mjs
    site/tests/prose-claim-helpers.test.mjs
    site/tests/theory-audit-claims.test.mjs
    docs/audits/theory-audit-remediation.test.mjs
    docs/audits/parity-matrix.test.mjs
    docs/audits/gap-closure-plan.test.mjs
)

echo "check-doc-conformance.sh: running ${#TESTS[@]} guardrail test file(s):"
for t in "${TESTS[@]}"; do
    echo "  - $t"
done

cd "$REPO_ROOT"
node --test "${TESTS[@]}"
