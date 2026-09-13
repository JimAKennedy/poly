#!/usr/bin/env bash
# check-doc-discipline.sh
#
# Runs the jk-standards doc checks the way CI runs them — with a base ref, so
# doc-drift actually executes.
#
# `jk-standards all` with no base ref prints
#   doc-drift: no --base or GITHUB_BASE_REF — skipped
# and exits 0. In a wall of green output that line reads like a pass. CI
# supplies GITHUB_BASE_REF and therefore runs the check; a developer running the
# doc-discipline token did not, and had no way to tell. M005 shipped a doc-drift
# violation through a green local validation set for exactly that reason, and
# M001 satisfied the same drift rule only incidentally.
#
# Supplying the base is the whole fix. An unresolvable base is NOT a silent-skip
# path: jk-standards exits 2 with "cannot resolve base ref", loudly. This script
# therefore does not carry a "prove it ran" assertion — an earlier draft did,
# built on a mis-measurement, and it could never have fired. Worse, the skip
# message itself begins "doc-drift", so the assertion would have passed on a
# skip: the very thing it claimed to prevent.
#
# Contract:
#   * a base IS determinable     -> run with it, so doc-drift executes
#   * a base is NOT determinable -> say so in plain words, then run the
#                                   remaining checks. A detached HEAD or a
#                                   shallow clone must stay usable, and
#                                   explaining is not the same as skipping in
#                                   silence.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "${SCRIPT_DIR}")"
cd "${PROJECT_DIR}"

# CI sets GITHUB_BASE_REF on pull_request events. Honour it rather than
# second-guessing the value CI already decided on.
BASE=""
if [ -n "${GITHUB_BASE_REF:-}" ]; then
    BASE="${GITHUB_BASE_REF}"
elif DEFAULT_REF="$(git symbolic-ref --quiet --short refs/remotes/origin/HEAD 2>/dev/null)"; then
    BASE="${DEFAULT_REF}"
elif git rev-parse --verify --quiet origin/main >/dev/null 2>&1; then
    BASE="origin/main"
fi

if [ -z "${BASE}" ]; then
    echo "=== doc-discipline: no base ref, so doc-drift cannot run ==="
    echo "Looked for GITHUB_BASE_REF, origin/HEAD and origin/main; found none."
    echo "Every other doc check still runs below. To include doc-drift, fetch"
    echo "the default branch: git fetch origin main"
    exec jk-standards all
fi

exec jk-standards all --base "${BASE}"
