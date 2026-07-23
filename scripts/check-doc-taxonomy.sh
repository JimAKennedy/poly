#!/usr/bin/env bash
set -euo pipefail

# check-doc-taxonomy.sh — enforce M048 S01 doc taxonomy.
#
# Every doc under docs/ and site/src/content/docs/ must carry a YAML
# front-matter `class:` field with a value in {generated, gated, archived}:
#
#   generated — content produced by a build step (renderer / codegen)
#   gated     — content is prose that a drift-map entry (M048 S02) will
#               force to update when its mapped source glob changes
#   archived  — frozen historical record (superseded planning docs, dated
#               review artifacts); not maintained; MUST carry a reader-
#               facing banner too
#
# The script fails (exit 1) if any doc lacks the field or uses an unknown
# value. It emits ::error file=…::-style annotations so GitHub Actions
# surfaces them inline in PRs (same convention as check-site-assets.sh).

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VALID_CLASSES=("generated" "gated" "archived")

collect_docs() {
    # docs/*.md and docs/reviews/*.md
    find "$REPO_ROOT/docs" -type f -name '*.md' 2>/dev/null || true
    # site/src/content/docs/*.mdx
    find "$REPO_ROOT/site/src/content/docs" -type f -name '*.mdx' 2>/dev/null || true
    # Root-level historical planning docs (M048 S12): these are outside
    # docs/ but still shape reader expectations, so they carry the same
    # class: frontmatter contract. Note: internal-docs/ is gitignored so
    # is not enforced here (kept local-only per project convention).
    for path in \
        "$REPO_ROOT/IMPLEMENTATION_PLAN.md"; do
        [ -f "$path" ] && echo "$path"
    done
}

# Extract the value of `class:` from the first YAML front-matter block.
# Returns empty string if the file has no front-matter or no class field.
extract_class() {
    local file="$1"
    awk '
        BEGIN { in_fm = 0; found = 0 }
        NR == 1 && /^---[[:space:]]*$/ { in_fm = 1; next }
        in_fm && /^---[[:space:]]*$/ { exit }
        in_fm && /^class:[[:space:]]*/ {
            sub(/^class:[[:space:]]*/, "")
            gsub(/[[:space:]]+$/, "")
            gsub(/^["'\'']|["'\'']$/, "")
            print
            found = 1
            exit
        }
    ' "$file"
}

is_valid_class() {
    local value="$1"
    local v
    for v in "${VALID_CLASSES[@]}"; do
        [ "$value" = "$v" ] && return 0
    done
    return 1
}

ERRORS=0
CHECKED=0

while IFS= read -r file; do
    [ -z "$file" ] && continue
    CHECKED=$((CHECKED + 1))
    relfile="${file#"$REPO_ROOT"/}"
    class_value="$(extract_class "$file")"

    if [ -z "$class_value" ]; then
        echo "::error file=$relfile,line=1::Missing 'class:' front-matter field. Add one of: ${VALID_CLASSES[*]}"
        ERRORS=$((ERRORS + 1))
        continue
    fi

    if ! is_valid_class "$class_value"; then
        echo "::error file=$relfile,line=1::Invalid class '$class_value'. Must be one of: ${VALID_CLASSES[*]}"
        ERRORS=$((ERRORS + 1))
    fi
done < <(collect_docs)

if [ "$ERRORS" -gt 0 ]; then
    echo ""
    echo "Found $ERRORS doc taxonomy error(s) across $CHECKED doc(s)"
    exit 1
fi

echo "All $CHECKED docs carry a valid 'class:' front-matter field"
