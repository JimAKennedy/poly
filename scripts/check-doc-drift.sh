#!/usr/bin/env bash
# check-doc-drift.sh — Enforce .github/docs-drift-map.yml on the PR diff.
#
# For each mapping in the drift map, if the PR touches any source glob the PR
# must also touch the mapped doc, OR any commit in the PR range must carry a
# `Docs-Not-Affected: <reason>` trailer.
#
# Base ref resolution:
#   1. --base <ref> flag if provided
#   2. GITHUB_BASE_REF env (set by GitHub Actions pull_request)
#   3. origin/main fallback
#
# Exit 0: no mapped sources touched, or every triggered mapping is satisfied.
# Exit 1: at least one mapping triggered without doc update or trailer bypass.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MAP_FILE="$REPO_ROOT/.github/docs-drift-map.yml"

BASE_OVERRIDE=""
while [ $# -gt 0 ]; do
  case "$1" in
    --base)
      BASE_OVERRIDE="$2"
      shift 2
      ;;
    --help|-h)
      sed -n '3,20p' "$0"
      exit 0
      ;;
    *)
      echo "Unknown arg: $1" >&2
      exit 2
      ;;
  esac
done

if [ ! -f "$MAP_FILE" ]; then
  echo "Drift map not found at $MAP_FILE" >&2
  exit 2
fi

# --- Resolve base ref ---
if [ -n "$BASE_OVERRIDE" ]; then
  BASE_REF="$BASE_OVERRIDE"
elif [ -n "${GITHUB_BASE_REF:-}" ]; then
  BASE_REF="origin/${GITHUB_BASE_REF}"
else
  BASE_REF="origin/main"
fi

# In CI the checkout may not have the base ref yet — fetch it just-in-time.
if ! git rev-parse --verify "$BASE_REF" >/dev/null 2>&1; then
  # Try to fetch it. If we're offline or the ref is malformed, this fails
  # loudly rather than silently skipping the check.
  echo "Fetching $BASE_REF (not present locally)..." >&2
  BARE_REF="${BASE_REF#origin/}"
  git fetch origin "$BARE_REF" --depth=100 >/dev/null 2>&1 || {
    echo "Cannot resolve base ref $BASE_REF — aborting drift check." >&2
    exit 2
  }
fi

MERGE_BASE="$(git merge-base "$BASE_REF" HEAD)"
CHANGED_FILES="$(git diff --name-only "$MERGE_BASE"...HEAD)"

if [ -z "$CHANGED_FILES" ]; then
  echo "No changed files vs $BASE_REF — drift check trivially passes."
  exit 0
fi

# --- Collect Docs-Not-Affected trailer bypasses ---
# Read commit messages for the PR range and pick out any Docs-Not-Affected: lines.
# Each line documents ONE bypass; the value string is the reason.
COMMIT_MSGS="$(git log --format=%B "$MERGE_BASE".."HEAD")"
BYPASSES="$(echo "$COMMIT_MSGS" | grep -E '^Docs-Not-Affected:' || true)"

if [ -n "$BYPASSES" ]; then
  echo "Found Docs-Not-Affected trailer(s):" >&2
  echo "$BYPASSES" | sed 's/^/  /' >&2
fi

# --- Parse map and check mappings ---
# Use python for portable YAML + glob matching. The script emits GitHub-format
# ::error annotations to stderr on violation and a plain summary to stdout.
export CHANGED_FILES BYPASSES MAP_FILE

python3 <<'PYEOF'
import fnmatch
import os
import sys
import yaml

with open(os.environ['MAP_FILE']) as f:
    data = yaml.safe_load(f)

changed = [line for line in os.environ['CHANGED_FILES'].splitlines() if line]
bypasses = [line for line in os.environ.get('BYPASSES', '').splitlines() if line]
has_bypass = bool(bypasses)

changed_set = set(changed)
violations = []
satisfied = []

for m in data['mappings']:
    doc = m['doc']
    reason = m['reason']
    triggered_sources = []
    for pattern in m['sources']:
        # fnmatch doesn't handle ** by itself; a two-level fallback covers
        # the ** case used for test globs. For anything more elaborate, use
        # pathspec.
        if '**' in pattern:
            base = pattern.split('**')[0]
            for f in changed:
                if f.startswith(base):
                    triggered_sources.append(f)
        else:
            for f in changed:
                if fnmatch.fnmatchcase(f, pattern):
                    triggered_sources.append(f)
    if not triggered_sources:
        continue
    if doc in changed_set:
        satisfied.append((doc, triggered_sources))
        continue
    if has_bypass:
        satisfied.append((doc, triggered_sources))  # bypass counts as satisfied
        continue
    violations.append({
        'doc': doc,
        'reason': reason,
        'sources': sorted(set(triggered_sources)),
    })

if violations:
    for v in violations:
        srcs = ', '.join(v['sources'][:5])
        more = f" (+{len(v['sources'])-5} more)" if len(v['sources']) > 5 else ''
        print(
            f"::error file={v['doc']}::Doc drift: PR touches {srcs}{more} "
            f"but not {v['doc']!r}. Reason: {v['reason']} — "
            f"either update {v['doc']} in this PR or add "
            f"'Docs-Not-Affected: <reason>' trailer to any commit.",
            file=sys.stderr,
        )
    print(
        f"\nFound {len(violations)} doc drift violation(s). Update the mapped "
        f"doc(s) or add a Docs-Not-Affected trailer.",
        file=sys.stderr,
    )
    sys.exit(1)

if satisfied:
    print(f"Doc drift check: {len(satisfied)} mapping(s) triggered, all satisfied.")
    for doc, srcs in satisfied:
        tag = 'bypassed' if has_bypass and doc not in {s.split(' ')[0] for s in changed} else 'doc-updated'
        print(f"  {doc} ({tag}) ← {len(srcs)} source(s)")
else:
    print("Doc drift check: no mapped sources touched.")
PYEOF
