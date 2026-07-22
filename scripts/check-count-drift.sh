#!/usr/bin/env bash
# M048 S06: numeral drift lint.
#
# Enforces that facts of the form "N presets / N chapters / N lanes / N patches /
# N categories" appear in exactly one place — site/src/generated/counts.json —
# and that gated docs import them (`{counts.presets}` in MDX) or link to a
# source-of-truth doc instead of restating the number.
#
# Scans docs/**/*.md and site/src/content/docs/**/*.mdx. Skips:
#   - files with front-matter `class: archived`
#   - files with a whole-file `<!-- counts-ok: ... -->` marker before the first
#     heading (top-of-file, front-matter-region exemption)
#   - lines inside fenced code blocks (```)
#   - lines that are markdown table rows (`| ... |`)
#   - lines carrying `<!-- counts-ok` or `{/* counts-ok` on the same line
#   - lines whose immediately-preceding non-blank line is a counts-ok marker
#   - MDX template literals `{counts.<field>}` (intended pattern)
#
# Flags: an Arabic numeral (\d+) or spelled-out cardinal (one..twenty, thirty,
# forty, fifty, hundred, fourteen, forty-three) appearing within 30 chars of a
# trigger noun: presets?, patches?, chapters?, lanes?, categor(y|ies).
#
# Exit codes: 0 = clean, 1 = one or more unmarked drift candidates found.

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ERRORS=0

is_archived() {
  local f="$1"
  awk '
    BEGIN { in_fm=0; fm_seen=0 }
    /^---[[:space:]]*$/ {
      if (in_fm) { in_fm=0; fm_seen=1; exit }
      else if (NR<=3) { in_fm=1; next }
    }
    in_fm && /^class:[[:space:]]*archived[[:space:]]*$/ { print "yes"; exit }
  ' "$f"
}

has_whole_file_exemption() {
  local f="$1"
  awk '
    /^#/ { exit }
    /<!--[[:space:]]*counts-ok/ { print "yes"; exit }
  ' "$f"
}

scan_file() {
  local f="$1"
  local relfile="${f#"$REPO_ROOT"/}"

  if [ "$(is_archived "$f")" = "yes" ]; then
    return 0
  fi
  if [ "$(has_whole_file_exemption "$f")" = "yes" ]; then
    return 0
  fi

  python3 - "$f" "$relfile" <<'PY'
import re, sys

path = sys.argv[1]
relfile = sys.argv[2]

with open(path, "r", encoding="utf-8") as fh:
    lines = fh.read().splitlines()

SPELLED = (
    r"one|two|three|four|five|six|seven|eight|nine|ten|eleven|twelve|thirteen|"
    r"fourteen|fifteen|sixteen|seventeen|eighteen|nineteen|twenty|thirty|forty|"
    r"fifty|sixty|seventy|eighty|ninety|hundred|thousand|forty-three"
)

# {counts.<anything>} — MDX template literal is the intended pattern; never flag.
COUNTS_TEMPLATE_RE = re.compile(r"\{\s*counts\.\w+\s*\}")

# Line-scoped exemption on same line.
LINE_MARKER_RE = re.compile(r"<!--\s*counts-ok|{/\*\s*counts-ok|/\*\s*counts-ok")

in_fence = False
prev_nonblank_had_marker = False
errors = 0

for i, raw in enumerate(lines, start=1):
    # Fenced code block state.
    stripped = raw.strip()
    if stripped.startswith("```"):
        in_fence = not in_fence
        prev_nonblank_had_marker = False
        continue
    if in_fence:
        prev_nonblank_had_marker = False
        continue

    # Skip markdown table rows.
    if stripped.startswith("|"):
        prev_nonblank_had_marker = False
        continue

    # Line-scoped marker on THIS line exempts it.
    this_line_marker = bool(LINE_MARKER_RE.search(raw))
    # Or on the immediately-preceding non-blank line.
    exempt_this_line = this_line_marker or prev_nonblank_had_marker

    if not stripped:
        # Blank line: preserve marker state so `<!-- counts-ok -->\n\nnumber prose`
        # still exempts. But drop it if the next non-blank isn't reachable — safest
        # to reset only after a non-marker non-blank line.
        continue

    # Update marker-state trailer for NEXT iteration BEFORE we return on exemption.
    next_marker = this_line_marker

    if exempt_this_line:
        prev_nonblank_had_marker = next_marker
        continue

    # Strip {counts.X} placeholders so we don't examine "counts.presets" as prose.
    scrubbed = COUNTS_TEMPLATE_RE.sub(" ", raw)

    # We only care about drift on INVENTORY-total claims:
    #   "N factory presets"     "N presets shipped"  "N presets in total"
    #   "all N presets"          "N presets total"    "N categories"
    #   "N-preset catalogue"     "N of the presets"
    # NOT: "three lanes at prime cycle lengths"  (compositional example)
    #      "Lane 5's probability"                 (specific-index reference)
    #      "Chapter 7: Balkan"                    (specific chapter heading)
    #      "Afrobeat 12/8 preset"                 (time-signature adjacent)
    #
    # Pattern: number followed within a few tokens by an INVENTORY-scoped trigger
    # phrase (not just the bare noun). This is deliberately conservative — it
    # catches the M047-flagged L4/L6 class ("14 factory presets") and future
    # regressions of the same form, while ignoring compositional prose.
    INVENTORY_TRIGGERS = (
        r"factory\s+presets?",
        r"presets?\s+(?:in\s+total|total|shipped|are\s+grouped|exist)",
        r"presets?\s+across\s+\d+\s+categor",
        r"of\s+the\s+presets?",
        r"presets?\s+built\s+into",
        r"categories?\s+total",
        r"categories?\s+in\s+total",
        r"chapters?\s+in\s+total",
        r"chapters?\s+total",
        r"(?:the|all)\s+categories",
    )
    ALTERNATION = "|".join(INVENTORY_TRIGGERS)

    CLAIM_RE = re.compile(
        rf"\b(?P<num>\d{{1,4}}|{SPELLED})"
        rf"(?:[-\s]+(?:[A-Za-z][A-Za-z-]*\s+){{0,3}})?"
        rf"(?P<phrase>{ALTERNATION})",
        re.I,
    )

    for m in CLAIM_RE.finditer(scrubbed):
        num_str = m.group("num")
        phrase_str = m.group("phrase")

        snippet = raw.strip()
        if len(snippet) > 120:
            snippet = snippet[:117] + "..."
        print(
            f"::error file={relfile},line={i}::Hardcoded inventory count '{num_str} ... {phrase_str}': "
            f"replace with {{counts.X}} from counts.json or add "
            f"<!-- counts-ok: reason -->  [{snippet}]"
        )
        errors += 1
        break

    prev_nonblank_had_marker = next_marker

sys.exit(errors)
PY
}

# Discover targets: docs/**/*.md and site/src/content/docs/**/*.mdx.
# Iterate via a while-read loop for portability (mapfile is bash-4+, macOS ships 3.2).
while IFS= read -r f; do
  [ -z "$f" ] && continue
  # scan_file's python3 subprocess returns its own error count as its exit code.
  # We only care whether it's >0.
  file_errors=0
  scan_file "$f" || file_errors=$?
  ERRORS=$((ERRORS + file_errors))
done < <(
  find "$REPO_ROOT/docs" -type f -name '*.md' 2>/dev/null
  find "$REPO_ROOT/site/src/content/docs" -type f -name '*.mdx' 2>/dev/null
)

if [ "$ERRORS" -gt 0 ]; then
  echo ""
  echo "Found $ERRORS count-drift error(s). Fix by replacing hardcoded numerals"
  echo "with {counts.X} imports from site/src/generated/counts.json, or add"
  echo "<!-- counts-ok: reason --> on the line (or immediately above it) to"
  echo "document why the hardcoded value is legitimate."
  exit 1
fi

echo "All docs pass count-drift check"
