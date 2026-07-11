#!/usr/bin/env bash
# check-snippet-regions.sh
#
# Scans site/src/content/docs/**/*.mdx for <CodeSnippet file="X" region="Y" /> tags,
# then verifies for every (file, region) pair that:
#   1. `file` exists at the repo root, and
#   2. the source file contains matching start + end markers:
#        // region:Y   or   # region:Y
#        // endregion:Y or # endregion:Y
#
# Exits 0 clean; exits 1 with a printed list of offending pairs and their MDX source lines.
#
# Runs in pre-push (scripts/pre-push-check.sh) and pre-commit (.pre-commit-config.yaml).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
DOCS_DIR="$REPO_ROOT/site/src/content/docs"

if [ ! -d "$DOCS_DIR" ]; then
    echo "check-snippet-regions.sh: docs directory not found: $DOCS_DIR" >&2
    exit 2
fi

# Portable extraction: match tags on a single line. CodeSnippet usage is
# consistently one-line in this project (see roadmap; enforced by convention).
# Multi-line tags would need a more complex parser — flag it if it becomes real.
PATTERN='<CodeSnippet[[:space:]]+[^>]*file="[^"]+"[[:space:]]+[^>]*region="[^"]+"'

FAILURES=0
FAILURE_LOG=""

log_fail() {
    FAILURES=$((FAILURES + 1))
    FAILURE_LOG="${FAILURE_LOG}$1
"
}

# grep across all mdx files, then parse each match line.
# `-H` prefixes filename, `-n` prefixes line number. Format: `mdx:LINE:...`.
while IFS= read -r hit; do
    mdx_path="${hit%%:*}"
    rest="${hit#*:}"
    mdx_line="${rest%%:*}"
    tag_body="${rest#*:}"

    # Extract file="..." and region="..." from the tag body.
    file_attr=$(printf '%s' "$tag_body" | sed -n 's/.*file="\([^"]*\)".*/\1/p')
    region_attr=$(printf '%s' "$tag_body" | sed -n 's/.*region="\([^"]*\)".*/\1/p')

    if [ -z "$file_attr" ] || [ -z "$region_attr" ]; then
        log_fail "  ${mdx_path}:${mdx_line}: could not parse file/region from tag"
        continue
    fi

    src_path="$REPO_ROOT/$file_attr"
    if [ ! -f "$src_path" ]; then
        log_fail "  ${mdx_path}:${mdx_line}: source file missing: ${file_attr} (region=${region_attr})"
        continue
    fi

    # Two accepted marker styles per side; grep -F for literal match, anchored via -x on the trimmed line.
    # Using awk to trim leading/trailing whitespace before comparison so indented markers count.
    start_ok=$(awk -v r="$region_attr" '
        {
            line = $0
            sub(/^[[:space:]]+/, "", line)
            sub(/[[:space:]]+$/, "", line)
            if (line == "// region:" r || line == "# region:" r) { print "1"; exit }
        }
    ' "$src_path")

    end_ok=$(awk -v r="$region_attr" '
        {
            line = $0
            sub(/^[[:space:]]+/, "", line)
            sub(/[[:space:]]+$/, "", line)
            if (line == "// endregion:" r || line == "# endregion:" r) { print "1"; exit }
        }
    ' "$src_path")

    if [ "$start_ok" != "1" ]; then
        log_fail "  ${mdx_path}:${mdx_line}: (file=${file_attr}, region=${region_attr}) missing start marker in source"
    fi
    if [ "$end_ok" != "1" ]; then
        log_fail "  ${mdx_path}:${mdx_line}: (file=${file_attr}, region=${region_attr}) missing end marker in source"
    fi
done < <(grep -rHnE "$PATTERN" "$DOCS_DIR" --include='*.mdx' || true)

if [ "$FAILURES" -ne 0 ]; then
    printf 'check-snippet-regions.sh: %d broken CodeSnippet reference(s):\n' "$FAILURES" >&2
    printf '%s' "$FAILURE_LOG" >&2
    exit 1
fi

# Count total scanned references so a green run leaves useful log evidence.
COUNT=$(grep -rHnE "$PATTERN" "$DOCS_DIR" --include='*.mdx' 2>/dev/null | wc -l | tr -d ' ')
printf 'check-snippet-regions.sh: %s CodeSnippet reference(s) OK\n' "$COUNT"
