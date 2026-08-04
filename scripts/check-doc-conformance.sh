#!/usr/bin/env bash
# check-doc-conformance.sh
#
# Single source of truth for the M071 doc-conformance guardrail suite.
# Invoked by BOTH .github/workflows/ci.yml (site-lint job) and
# scripts/pre-push-check.sh so the CI list and the pre-push list are one list
# and cannot drift.
#
# Runs the full conformance / doc-arithmetic guardrail node-test suite from
# site/ and exits non-zero if any test fails. `node --test` names the exact
# failing file/case in its output, so a reintroduced wrong Euclidean pattern or
# a dangling preset name surfaces the offending file directly.
#
# Requires site/node_modules (the claim tests import js-yaml). CI runs `npm ci`
# before this step; callers that cannot guarantee node_modules (the pre-push
# hook) should gate on its presence before invoking — this script assumes deps
# are installed.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SITE_DIR="$REPO_ROOT/site"

# The M071 conformance / guardrail suite. Adding a guardrail test to the suite
# means adding it here — site/tests/doc-conformance-wiring.test.mjs asserts that
# every required file is named in this runner, so an omission fails that wiring
# test rather than silently dropping coverage.
TESTS=(
    tests/euclidean-claims.test.mjs
    tests/appendix-euclidean-claims.test.mjs
    tests/prose-pattern-claims.test.mjs
    tests/chapter-euclidean-guardrail.test.mjs
    tests/polypatch-preset-resolution.test.mjs
    tests/preset-table-conformance.test.mjs
    tests/preset-taxonomy-conformance.test.mjs
    tests/prose-conformance-claims.test.mjs
    tests/idiom-break-framing.test.mjs
    tests/theory-euclidean-guardrail.test.mjs
    tests/theory-patch-conformance.test.mjs
)

echo "check-doc-conformance.sh: running ${#TESTS[@]} doc-conformance guardrail test file(s):"
for t in "${TESTS[@]}"; do
    echo "  - site/$t"
done

cd "$SITE_DIR"
node --test "${TESTS[@]}"
