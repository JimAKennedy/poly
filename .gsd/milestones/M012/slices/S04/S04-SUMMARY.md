---
id: S04
parent: M012
milestone: M012
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - site/src/components/CodeSnippet.astro
  - engine/src/euclidean.cpp
  - engine/src/engine.cpp
  - engine/src/envelope.cpp
  - engine/src/macro.cpp
  - site/src/content/docs/08-minimalism.mdx
key_decisions:
  - Used Astro build-time fs.readFileSync rather than a Vite plugin — simpler, no build config changes needed
  - Region tags use // region:name comment style compatible with clang-format
  - Error state renders inline message rather than failing the build — graceful degradation
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-25T01:30:46.217Z
blocker_discovered: false
---

# S04: Live Code Extraction Pipeline

**CodeSnippet components now extract and syntax-highlight real C++ from engine source files at build time via region tags**

## What Happened

Implemented live code extraction for the Poly guide site. Added region comment markers (// region:name / // endregion:name) to 4 engine source files covering 5 code regions: bjorklund (euclidean.cpp), kotekan and drift-accumulator (engine.cpp), apply (envelope.cpp), and apply (macro.cpp). Rewrote CodeSnippet.astro to use Node fs.readFileSync at Astro build time, extract region content, strip common indentation, and render via Astro's built-in Code component (Shiki). Fixed a stale reference in 08-minimalism.mdx that pointed to nonexistent lane_state.cpp. All 5 CodeSnippet instances now render real syntax-highlighted C++ with GitHub source links. Documentation can never go stale — code changes are reflected on the next build.

## Verification

astro build: 21 pages, zero errors. All 5 CodeSnippet pages confirmed via grep to contain Shiki-highlighted code blocks with zero extraction errors. C++ build and 237 tests pass with region tag comments.

## Requirements Advanced

None.

## Requirements Validated

None.

## New Requirements Surfaced

None.

## Requirements Invalidated or Re-scoped

None.

## Operational Readiness

None.

## Deviations

None.

## Known Limitations

None.

## Follow-ups

None.

## Files Created/Modified

None.
