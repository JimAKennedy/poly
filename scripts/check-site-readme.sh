#!/usr/bin/env bash
# check-site-readme.sh
#
# Guards the OSS-maturity criterion "site README is project-specific, not
# Starlight template". The Poly guide site (site/) was scaffolded from
# `npm create astro -- --template starlight`, whose default docs are generic
# boilerplate. Recon (M031 S06) confirmed site/README.md shipped verbatim as
# "Starlight Starter Kit: Basics" and site/CLAUDE.md + site/AGENTS.md as the
# stock Astro stub. S06 T01 rewrote all three to describe the Poly guide site.
#
# This is a REGRESSION GUARD, not a one-time scrub (D034/MEM110): the rewrite
# already happened, so there is nothing to remove. The guard fails the build the
# moment the template boilerplate creeps back — e.g. a re-scaffold, a copy-paste
# from the Starlight docs, or an accidental revert.
#
# It enforces two things over the three contributor-facing site docs
# (site/README.md, site/CLAUDE.md, site/AGENTS.md):
#   1. NEGATIVE — none may contain the verbatim template boilerplate phrases
#      below. These are distinctive template strings, NOT the framework names:
#      the rewritten docs legitimately say "Starlight" / "Astro" and link to
#      starlight.astro.build, and none of those match the forbidden set.
#   2. POSITIVE (CI mode only) — site/README.md must carry the project marker
#      `poly.jk.digital`, proving it describes the Poly guide site.
#
# Forbidden markers and why each is template-only, never legitimate prose:
#   Starlight Starter Kit  — the scaffold README's H1 title.
#   Seasoned astronaut     — from "Seasoned astronaut? Delete this file."
#   npm create astro       — the scaffold-generation command.
#   docs.astro.build       — the Astro-docs link the stub CLAUDE/AGENTS carried;
#                            the real docs link the framework's own site
#                            (starlight.astro.build), which does not match.
#
# Usage:
#   scripts/check-site-readme.sh            # scan the three site docs (CI mode)
#   scripts/check-site-readme.sh FILE...    # scan exactly the named files (test mode)
#
# Exits 0 when no scanned doc carries template boilerplate (and, in CI mode, the
# README carries the project marker), printing the count.
# Exits 1 and names each offending file:line:content when a violation is found.
#
# Runs in .github/workflows/ci.yml (code-quality job). The companion proof
# scripts/check-site-readme.mjs asserts this wiring and drives the green/red
# behavior (anti-drift seam, MEM110).

set -euo pipefail

# Verbatim Starlight/Astro template boilerplate. Distinctive template strings
# only — the framework names "Starlight"/"Astro" are deliberately NOT here.
FORBIDDEN_RE='Starlight Starter Kit|Seasoned astronaut|npm create astro|docs\.astro\.build'

# Project marker the guide README must carry (CI-mode positive check).
REQUIRED_MARKER='poly.jk.digital'
README='site/README.md'

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# File set: explicit args (test mode) or the three tracked site docs (CI mode).
# CI mode also runs the positive README-marker check; test mode scans only the
# named files for forbidden markers so red-behavior fixtures stay self-contained.
CI_MODE=0
FILES=()
if [ "$#" -gt 0 ]; then
    FILES=("$@")
else
    CI_MODE=1
    cd "$REPO_ROOT"
    FILES=("$README" site/CLAUDE.md site/AGENTS.md)
fi

OFFENDERS=0
OFFENDER_LOG=""
for f in "${FILES[@]}"; do
    if [ ! -f "$f" ]; then
        OFFENDERS=$((OFFENDERS + 1))
        OFFENDER_LOG="${OFFENDER_LOG}  ${f}: file not found
"
        continue
    fi
    # grep exits 1 when there are no matches; guard with `|| true` under set -e.
    while IFS= read -r hit; do
        [ -n "$hit" ] || continue
        OFFENDERS=$((OFFENDERS + 1))
        OFFENDER_LOG="${OFFENDER_LOG}  ${f}:${hit}
"
    done < <(grep -nE "$FORBIDDEN_RE" -- "$f" 2>/dev/null || true)
done

# Positive check: the guide README must name the site (CI mode only).
if [ "$CI_MODE" -eq 1 ]; then
    if ! grep -qF "$REQUIRED_MARKER" -- "$README" 2>/dev/null; then
        OFFENDERS=$((OFFENDERS + 1))
        OFFENDER_LOG="${OFFENDER_LOG}  ${README}: missing required project marker '${REQUIRED_MARKER}'
"
    fi
fi

if [ "$OFFENDERS" -ne 0 ]; then
    printf 'check-site-readme.sh: %d site-doc template-boilerplate violation(s):\n' "$OFFENDERS" >&2
    printf '%s' "$OFFENDER_LOG" >&2
    printf 'The site docs must describe the Poly guide site, not the Starlight\n' >&2
    printf 'template. Rewrite the offending prose to be project-specific; keep the\n' >&2
    printf 'project marker %s in %s.\n' "$REQUIRED_MARKER" "$README" >&2
    exit 1
fi

printf 'check-site-readme.sh: scanned %d site doc(s); no template boilerplate\n' "${#FILES[@]}"
