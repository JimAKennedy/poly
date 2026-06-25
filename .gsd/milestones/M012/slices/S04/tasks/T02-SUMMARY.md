---
id: T02
parent: S04
milestone: M012
key_files:
  - site/src/components/CodeSnippet.astro
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-25T01:30:17.801Z
blocker_discovered: false
---

# T02: Replaced placeholder CodeSnippet with live extraction using fs.readFileSync and Astro Code component

**Replaced placeholder CodeSnippet with live extraction using fs.readFileSync and Astro Code component**

## What Happened

Rewrote CodeSnippet.astro to read C++ source files at build time via Node fs, extract content between region markers, strip common indentation, and render with Shiki syntax highlighting via Astro's built-in Code component. Added GitHub source link in header. Error state renders a styled message instead of build failure.

## Verification

astro build succeeds with 21 pages; HTML output contains syntax-highlighted C++ code with Shiki classes; no error messages in any of the 5 CodeSnippet pages

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cd site && npx astro build` | 0 | pass — 21 pages, 743ms, zero errors | 743ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `site/src/components/CodeSnippet.astro`
