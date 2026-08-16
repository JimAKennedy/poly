#!/usr/bin/env bash
# check-personal-paths.sh
#
# Guards the OSS-maturity criterion "No personal-machine paths in committed
# files": no git-tracked file may embed an absolute per-user home path that
# leaks a specific developer's machine layout, e.g.
#   /Users/<name>/...     (macOS home)
#   /home/<name>/...      (Linux home)
#   C:\Users\<name>\...   (Windows home)
#
# This is a REGRESSION GUARD, not a one-time scrub (D034): recon proved the
# tree is already clean of real personal paths, so there is nothing to remove.
# The guard exists to keep it clean — it fails the build the moment a personal
# path is committed.
#
# Portable per-user references are NOT personal paths and are deliberately
# allowed: the `~` home abbreviation (`~/Library/...`, `~/.cache/...`) names
# no specific machine, and generic CI service accounts are shared infra.
#
# ALLOWLIST — three benign absolute-path hits exist in tracked docs/config.
# Each entry is greppable (search `personal-paths:allow`) and reasoned, so the
# audit is `grep personal-paths:allow scripts/check-personal-paths.sh`:
#   1. freesound.org/home/   — third-party URL path, not a filesystem home dir
#                              (docs/sample-sourcing.md attribution link).
#   2. C:\Users\polyci       — `polyci` is the generic Windows CI service
#                              account, not a personal machine
#                              (docs/windows-test-runner-setup.md).
#   3. /Users/jim/...        — the .gitignore rationale comment illustrating why
#                              .mcp.json (which holds absolute prefixes) is
#                              gitignored; the literal `...` ellipsis marks it
#                              as illustrative, not a real committed path.
# A real personal path (e.g. /Users/jim/dev/poly) does NOT match any allowlist
# entry and still fails.
#
# Usage:
#   scripts/check-personal-paths.sh            # scan all tracked files (CI mode)
#   scripts/check-personal-paths.sh FILE...    # scan exactly the named files (test mode)
#
# Exits 0 when no tracked file carries a non-allowlisted personal path.
# Exits 1 and names each offending file:line:content when one is found.
#
# Runs in .github/workflows/ci.yml (code-quality job). The companion proof
# scripts/check-personal-paths.mjs asserts this wiring and drives the green/red
# behavior (anti-drift seam, MEM110).

set -euo pipefail

# Absolute per-user home paths. `[A-Za-z0-9._-]+` requires a name segment, so a
# bare `/home` or `/Users` (no user) does not match.
PERSONAL_PATH_RE='(/Users/|/home/)[A-Za-z0-9._-]+|[Cc]:\\Users\\[A-Za-z0-9._-]+'

# personal-paths:allow — benign lines exempted by content (reasons in header).
ALLOW_RE='freesound\.org/home/|[Cc]:\\Users\\polyci|/Users/jim/\.\.\.'

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# File set: explicit args (test mode) or all tracked files (CI mode). In CI mode
# the guard and its behavioral proof are excluded: they necessarily embed path
# patterns as allowlist definitions and red-behavior test fixtures, so scanning
# them would flag their own data.
FILES=()
if [ "$#" -gt 0 ]; then
    FILES=("$@")
else
    cd "$REPO_ROOT"
    while IFS= read -r f; do
        case "$f" in
        scripts/check-personal-paths.sh | scripts/check-personal-paths.mjs) continue ;;
        esac
        FILES+=("$f")
    done < <(git ls-files)
fi

OFFENDERS=0
OFFENDER_LOG=""
for f in "${FILES[@]}"; do
    if [ ! -f "$f" ]; then
        continue
    fi
    # grep exits 1 when there are no matches; guard with `|| true` under set -e.
    while IFS= read -r hit; do
        [ -n "$hit" ] || continue
        # Drop allowlisted lines. grep -E over the whole matched line content.
        if printf '%s\n' "$hit" | grep -Eq "$ALLOW_RE"; then
            continue
        fi
        OFFENDERS=$((OFFENDERS + 1))
        OFFENDER_LOG="${OFFENDER_LOG}  ${f}:${hit}
"
    done < <(grep -nE "$PERSONAL_PATH_RE" -- "$f" 2>/dev/null || true)
done

if [ "$OFFENDERS" -ne 0 ]; then
    printf 'check-personal-paths.sh: %d personal-machine path(s) in tracked files:\n' "$OFFENDERS" >&2
    printf '%s' "$OFFENDER_LOG" >&2
    printf 'A personal path leaks a developer machine layout. Use a portable form\n' >&2
    printf '(the `~` home abbreviation), or if the hit is genuinely benign add a\n' >&2
    printf 'reasoned `personal-paths:allow` entry to scripts/check-personal-paths.sh.\n' >&2
    exit 1
fi

printf 'check-personal-paths.sh: scanned %d tracked file(s); no personal-machine paths\n' "${#FILES[@]}"
