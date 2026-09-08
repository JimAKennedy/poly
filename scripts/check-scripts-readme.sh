#!/usr/bin/env bash
# check-scripts-readme.sh
#
# Guards the scripts-inventory doc: scripts/README.md must stay in sync with
# what actually lives in scripts/. A directory map that silently omits new
# scripts (or keeps listing deleted ones) is worse than no map — readers trust
# it precisely because it looks maintained.
#
# Two directions are enforced over the TOP LEVEL of scripts/ (subdirectories
# are covered as a unit — each carries its own README owning its contents):
#
#   1. COVERAGE — every top-level script (*.sh, *.mjs, *.js) and every
#      subdirectory must appear in the README as a backticked token:
#      `name.sh` for files, `name/` for directories.
#   2. NO STALE ENTRIES — every backticked token in the README that looks like
#      a script name (`name.sh|.mjs|.js`, no slash) or a directory (`name/`)
#      must resolve to a real file/directory in scripts/. Deleting or renaming
#      a script means updating its README line in the same change.
#
# Tokens that are not bare script/dir names (paths with interior slashes,
# commands, other filenames) are deliberately ignored — the README may cite
# `ci.yml` or `site/src/...` freely without this guard claiming them.
#
# Usage:
#   scripts/check-scripts-readme.sh                      # CI mode: real tree
#   scripts/check-scripts-readme.sh SCRIPTS_DIR README   # test mode: fixtures
#
# Exits 0 when README and directory agree, printing the entry count scanned.
# Exits 1 naming each violation. Exits 2 on usage error.
#
# Runs in .github/workflows/ci.yml (code-quality job). The companion proof
# scripts/check-scripts-readme.mjs asserts this wiring and drives the
# green/red behavior (same seam as check-site-readme / check-personal-paths).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

if [ "$#" -eq 0 ]; then
    SCRIPTS_DIR="$REPO_ROOT/scripts"
    README="$REPO_ROOT/scripts/README.md"
elif [ "$#" -eq 2 ]; then
    SCRIPTS_DIR="$1"
    README="$2"
else
    echo "Usage: $(basename "$0") [SCRIPTS_DIR README_FILE]" >&2
    exit 2
fi

if [ ! -f "$README" ]; then
    echo "check-scripts-readme.sh: README not found at ${README}" >&2
    exit 1
fi

OFFENDERS=0
OFFENDER_LOG=""

# --- Direction 1: every top-level script and subdirectory is listed ----------
SCANNED=0
for entry in "$SCRIPTS_DIR"/*; do
    name="$(basename "$entry")"
    if [ -d "$entry" ]; then
        token="\`${name}/\`"
    else
        case "$name" in
            README.md) continue ;;
            *.sh | *.mjs | *.js) token="\`${name}\`" ;;
            *) continue ;;
        esac
    fi
    SCANNED=$((SCANNED + 1))
    if ! grep -qF "$token" "$README"; then
        OFFENDERS=$((OFFENDERS + 1))
        OFFENDER_LOG="${OFFENDER_LOG}  ${name}: present in ${SCRIPTS_DIR} but not listed in README (add a ${token} entry)
"
    fi
done

# --- Direction 2: every listed script/dir token still exists -----------------
while IFS= read -r tok; do
    inner="${tok#\`}"
    inner="${inner%\`}"
    if [[ "$inner" =~ ^[A-Za-z0-9._-]+\.(sh|mjs|js)$ ]]; then
        if [ ! -f "$SCRIPTS_DIR/$inner" ]; then
            OFFENDERS=$((OFFENDERS + 1))
            OFFENDER_LOG="${OFFENDER_LOG}  ${inner}: listed in README but no such file in ${SCRIPTS_DIR} (stale entry — remove or fix it)
"
        fi
    elif [[ "$inner" =~ ^[A-Za-z0-9._-]+/$ ]]; then
        if [ ! -d "$SCRIPTS_DIR/${inner%/}" ]; then
            OFFENDERS=$((OFFENDERS + 1))
            OFFENDER_LOG="${OFFENDER_LOG}  ${inner}: listed in README but no such directory in ${SCRIPTS_DIR} (stale entry — remove or fix it)
"
        fi
    fi
done < <(grep -o '`[^`]*`' "$README" | sort -u)

if [ "$OFFENDERS" -ne 0 ]; then
    printf 'check-scripts-readme.sh: %d README/directory sync violation(s):\n' "$OFFENDERS" >&2
    printf '%s' "$OFFENDER_LOG" >&2
    printf 'scripts/README.md is the directory map for scripts/ — every top-level\n' >&2
    printf 'script and subdirectory needs a one-line entry, and entries for removed\n' >&2
    printf 'scripts must go in the same change that removes them.\n' >&2
    exit 1
fi

printf 'check-scripts-readme.sh: %d top-level entr(y/ies) all listed; no stale README entries\n' "$SCANNED"
