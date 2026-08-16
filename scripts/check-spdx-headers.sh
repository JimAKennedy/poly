#!/usr/bin/env bash
# check-spdx-headers.sh
#
# Guards the OSS-maturity criterion "SPDX headers on all source files": every
# tracked first-party C / C++ / ObjC source file must BEGIN with
#   // SPDX-License-Identifier: GPL-3.0-or-later
# on its first line (D033 — repo LICENSE is GPLv3; identifier is the SPDX
# `-or-later` form). No vendored/third-party sources are tracked (the VST3 SDK
# and GoogleTest arrive via FetchContent), so every tracked source is first-party
# and must carry the header.
#
# Usage:
#   scripts/check-spdx-headers.sh            # scan all tracked sources (CI mode)
#   scripts/check-spdx-headers.sh FILE...    # scan exactly the named files (test mode)
#
# Exits 0 when every scanned file begins with the header, printing the count.
# Exits 1 and names each offending file when any file lacks the first-line header.
#
# Runs in .github/workflows/ci.yml (code-quality job). The companion proof
# scripts/check-spdx-headers.mjs asserts this wiring and drives the green/red
# behavior (anti-drift seam, MEM110).

set -euo pipefail

SPDX_LINE="// SPDX-License-Identifier: GPL-3.0-or-later"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# File set: explicit args (test mode) or all tracked C/C++/ObjC sources (CI mode).
FILES=()
if [ "$#" -gt 0 ]; then
    FILES=("$@")
else
    cd "$REPO_ROOT"
    while IFS= read -r f; do
        FILES+=("$f")
    done < <(git ls-files -- '*.c' '*.cc' '*.cpp' '*.cxx' '*.h' '*.hpp' '*.hh' '*.mm' '*.m')
fi

MISSING=0
MISSING_LOG=""
for f in "${FILES[@]}"; do
    if [ ! -f "$f" ]; then
        MISSING=$((MISSING + 1))
        MISSING_LOG="${MISSING_LOG}  ${f}: file not found
"
        continue
    fi
    # The header must be the FIRST line — "begins with" is the contract.
    if [ "$(head -n 1 "$f")" != "$SPDX_LINE" ]; then
        MISSING=$((MISSING + 1))
        MISSING_LOG="${MISSING_LOG}  ${f}
"
    fi
done

if [ "$MISSING" -ne 0 ]; then
    printf 'check-spdx-headers.sh: %d source file(s) missing the SPDX header on line 1:\n' "$MISSING" >&2
    printf '%s' "$MISSING_LOG" >&2
    printf 'Expected line 1 to be exactly: %s\n' "$SPDX_LINE" >&2
    exit 1
fi

printf 'check-spdx-headers.sh: %d source file(s) carry the SPDX header\n' "${#FILES[@]}"
