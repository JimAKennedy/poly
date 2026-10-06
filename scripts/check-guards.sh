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
run_guard "ledger-row-ids"            node scripts/check-ledger-row-ids.mjs
run_guard "version-source"            node --test scripts/check-version-source.mjs

# M007 S02 found this one: check-release-workflow.mjs locks the release
# workflow's shape in 27 tests, and no workflow runs it — release.yml names it
# only in comments. It was worse off than the eleven, which at least ran in CI.
run_guard "release-workflow contract"  node --test scripts/check-release-workflow.mjs

# open-source-launch M006/S01 (OS33): release-verify.yml is what a clean
# machine measures of a published Release; a verifier nothing checks is the
# gap this programme keeps finding.
run_guard "release-verify contract"    node --test scripts/check-release-verify-workflow.mjs

# open-source-launch M002/S01 (OS06, OS08): every workflow declares a top-level
# permissions block and no third-party checkout floats. The three jk-standards
# workflow checks pass on a tree that violates both, so this is the guard.
run_guard "workflow-hygiene contract"  node --test scripts/check-workflow-hygiene.mjs

# verifiable-references M006/S01 (VR16): the weekly dead-reference check is
# advisory only while its shape holds — scheduled, never failing a PR, writing
# issues only from scheduled and manual runs — and its classes (a bot refusal
# is blocked, never dead) are proved against a local HTTP server.
run_guard "reference-links contract"   node --test scripts/check-reference-links-workflow.mjs
run_guard "reference-links classes"    node --test scripts/reference-links.test.mjs

# open-source-launch M007/S01 (OS35): the macOS installer package installs the
# VST3 where a DAW looks, offers a per-user install, carries nothing but the
# bundle and no quarantine attribute. Built and expanded, never installed; skips
# with a stated reason off macOS.
run_guard "macOS package contents"     node --test scripts/packaging/build-macos-pkg.test.mjs

# open-source-launch M008/S01 (OS37): the Windows installer's WiX source — one
# per-machine MSI into Common Files\VST3, a fixed UpgradeCode, no custom
# actions. WiX builds only on Windows; this checks the source anywhere, and the
# package-windows CI job builds, signs, installs and removes the MSI.
run_guard "Windows MSI source"         node --test scripts/packaging/poly-wxs.test.mjs

# open-source-launch M004 (OS20, OS22): the roadmap links queries rather than
# numbers, and the README and CONTRIBUTING carry no internal identifier and
# keep the section order written for the person downloading.
run_guard "front-door contract"        node --test scripts/check-front-door.mjs

# open-source-launch M004/S03 (OS25): docs/README.md names every top-level
# document and directory under docs/ and nothing that is gone.
run_guard "docs-index"                 node --test scripts/check-docs-index.mjs

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
