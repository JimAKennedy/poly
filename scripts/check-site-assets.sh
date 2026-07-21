#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DOCS_DIR="$REPO_ROOT/site/src/content/docs"
PUBLIC_DIR="$REPO_ROOT/site/public"
# Custom domain (poly.jk.digital) serves from root; base path was dropped in M047 S05a.
BASE=""
ERRORS=0
WARNINGS=0

if [ ! -d "$DOCS_DIR" ]; then
  echo "No site docs directory — skipping asset check"
  exit 0
fi

while IFS= read -r line; do
  file="$(echo "$line" | cut -d: -f1)"
  lineno="$(echo "$line" | cut -d: -f2)"
  src="$(echo "$line" | sed 's/.*src="\([^"]*\)".*/\1/')"
  [ -z "$src" ] && continue

  relfile="${file#"$REPO_ROOT"/}"
  basename="$(basename "$file")"

  # Skip component-demo (intentional placeholder)
  [[ "$basename" == "component-demo.mdx" ]] && continue

  # Check base prefix — this is always a bug
  if [[ "$src" != "$BASE/"* ]]; then
    echo "::error file=$relfile,line=$lineno::Missing base prefix: src=\"$src\" should start with \"$BASE/\""
    ERRORS=$((ERRORS + 1))
    continue
  fi

  # Check file exists — warn only (screenshots are added incrementally)
  asset_path="${src#"$BASE"/}"
  if [ ! -f "$PUBLIC_DIR/$asset_path" ]; then
    echo "::warning file=$relfile,line=$lineno::Asset not found: $asset_path (screenshot pending)"
    WARNINGS=$((WARNINGS + 1))
  fi
done < <(grep -rn 'src="[^"]*"' "$DOCS_DIR" --include='*.mdx' | grep 'PolyScreenshot' || true)

if [ "$ERRORS" -gt 0 ]; then
  echo ""
  echo "Found $ERRORS site asset error(s) and $WARNINGS warning(s)"
  exit 1
fi

if [ "$WARNINGS" -gt 0 ]; then
  echo "All paths valid — $WARNINGS screenshot(s) pending"
else
  echo "All site asset references valid"
fi
