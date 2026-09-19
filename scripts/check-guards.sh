#!/usr/bin/env bash
# check-guards.sh
#
# Runs the repo guards that CI enforces and that no other local command
# reaches. Before M007 these eleven ran in the code-quality and site-lint jobs
# and nowhere a developer could invoke, so a complete local validation run could
# go green while CI went red — which happened on three consecutive ships:
# doc-drift on M005, check-scripts-readme on M006, and generate-params-json on
# M002, the last masked by a stale emitter.
#
# One command rather than two mirroring the CI jobs: a developer asking what CI
# will run on their change should get one answer. Each guard names itself as it
# runs, which is how a failure is identified without re-running them singly.
#
# Deliberately NOT `set -e`. A developer wants every broken guard from one run,
# not one per invocation, so a failure is recorded and the run continues.
set -uo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "${SCRIPT_DIR}")"
cd "${PROJECT_DIR}"

FAILED=()

RAN=0

run_guard() {
    local label="$1"
    shift
    RAN=$((RAN + 1))
    printf '=== %s ===\n' "${label}"
    if "$@"; then
        return 0
    fi
    FAILED+=("${label}")
    return 0
}

# code-quality job
run_guard "spdx-headers"            bash scripts/check-spdx-headers.sh
run_guard "spdx-headers contract"   node --test scripts/check-spdx-headers.mjs
run_guard "personal-paths"          bash scripts/check-personal-paths.sh
run_guard "personal-paths contract" node --test scripts/check-personal-paths.mjs
run_guard "site-readme"             bash scripts/check-site-readme.sh
run_guard "site-readme contract"    node --test scripts/check-site-readme.mjs
run_guard "scripts-readme"          bash scripts/check-scripts-readme.sh
run_guard "scripts-readme contract" node --test scripts/check-scripts-readme.mjs

# site-lint job. check-sample-manifest.sh is invoked twice in CI with different
# flags; running it once here would check less than CI does.
run_guard "sample-manifest (strict)"   bash scripts/check-sample-manifest.sh --strict
run_guard "sample-manifest (coverage)" bash scripts/check-sample-manifest.sh --coverage
run_guard "site-assets"                bash scripts/check-site-assets.sh
run_guard "bridge-schema-coverage"     node scripts/check-bridge-schema-coverage.mjs
run_guard "ascii-diagrams"             node scripts/check-ascii-diagrams.mjs

# M007 S02 found this one: check-release-workflow.mjs locks the release
# workflow's shape in 27 tests, and no workflow runs it — release.yml names it
# only in comments. It was worse off than the eleven, which at least ran in CI.
run_guard "release-workflow contract"  node --test scripts/check-release-workflow.mjs

echo
if [ "${#FAILED[@]}" -eq 0 ]; then
    echo "=== check-guards.sh: ${RAN} guard invocation(s) passed ==="
    exit 0
fi

echo "=== check-guards.sh: ${#FAILED[@]} guard(s) FAILED ===" >&2
for f in "${FAILED[@]}"; do
    echo "  - ${f}" >&2
done
exit 1
