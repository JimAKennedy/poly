#!/usr/bin/env bash
# check-doc-discipline.sh
#
# Runs the jk-standards doc checks the way CI runs them, and refuses to report
# success for a check that did not execute.
#
# `jk-standards all` skips doc-drift unless it is given a base ref, and it says
# so on one line in a wall of green output — which reads exactly like a pass.
# CI supplies GITHUB_BASE_REF and therefore runs the check; a developer running
# `jk-standards all` does not, and cannot tell. M005 shipped a doc-drift
# violation through a green local validation set for that reason, and M001
# satisfied the same drift rule only incidentally.
#
# Supplying `--base` alone is not sufficient. With a base that cannot be
# resolved, jk-standards prints no doc-drift line at all and still exits 0 — so
# a token reading `jk-standards all --base origin/main` would pass while the
# check never ran, on any clone where that ref is not fetched. This script
# therefore asserts the check ran, which is the one arm that cannot reproduce
# the bug it exists to remove.
#
# Contract:
#   * a base IS determinable   -> run the checks, then fail unless doc-drift
#                                 actually reported
#   * a base is NOT determinable -> run the checks, say plainly that doc-drift
#                                 could not run and why, and exit on the run's
#                                 own status. A detached HEAD or a shallow
#                                 clone must stay usable; explaining is not the
#                                 same as skipping in silence.
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
    echo "=== doc-discipline: no base ref ==="
    echo "doc-drift cannot run: no GITHUB_BASE_REF, no origin/HEAD, and no"
    echo "origin/main in this clone. Every other doc check still runs below."
    echo "Fetch the default branch (git fetch origin main) to include doc-drift."
    exec jk-standards all
fi

OUTPUT_FILE="$(mktemp -t poly-doc-discipline.XXXXXX)"
cleanup() { rm -f "${OUTPUT_FILE}"; }
trap cleanup EXIT INT TERM

set +e
jk-standards all --base "${BASE}" 2>&1 | tee "${OUTPUT_FILE}"
STATUS="${PIPESTATUS[0]}"
set -e

if [ "${STATUS}" -ne 0 ]; then
    exit "${STATUS}"
fi

# The point of this script. A zero exit from jk-standards does not mean
# doc-drift ran, so do not report success until it is known to have.
if ! grep -q '^doc-drift' "${OUTPUT_FILE}"; then
    echo "=== FAIL: doc-drift did not run ===" >&2
    echo "jk-standards exited 0 against base '${BASE}' but reported no doc-drift" >&2
    echo "line, which means the check was skipped rather than passed. Refusing to" >&2
    echo "report success for a check that did not execute. A base ref that does" >&2
    echo "not resolve is the usual cause — check that '${BASE}' exists." >&2
    exit 1
fi
