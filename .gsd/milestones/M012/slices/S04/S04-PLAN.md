# S04: Live Code Extraction Pipeline

**Goal:** CodeSnippet components extract real C++ code from engine source files at build time via region tags, replacing placeholder text with syntax-highlighted live code
**Demo:** CodeSnippet components in at least 3 chapters render syntax-highlighted C++ pulled from engine source files via region tags, with filename headers and GitHub line links

## Must-Haves

- All 5 CodeSnippet instances render syntax-highlighted C++ from actual engine source; astro build succeeds with zero errors; missing file reference fixed

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Add region tags to C++ engine source files** `est:15min`
  Add // region:name and // endregion:name comment pairs to 4 engine source files marking the 5 code regions referenced by CodeSnippet components. Fix 08-minimalism.mdx reference from nonexistent lane_state.cpp to engine.cpp.
  - Files: `engine/src/euclidean.cpp`, `engine/src/engine.cpp`, `engine/src/envelope.cpp`, `engine/src/macro.cpp`, `site/src/content/docs/08-minimalism.mdx`
  - Verify: grep -r 'region:' engine/src/ shows all 5 region pairs

- [x] **T02: Implement live code extraction in CodeSnippet.astro** `est:20min`
  Replace placeholder rendering with build-time file reading (fs.readFileSync), region extraction, and syntax-highlighted output using Astro Code component. Add GitHub source link in header.
  - Files: `site/src/components/CodeSnippet.astro`
  - Verify: cd site && npx astro build succeeds with zero errors

- [x] **T03: Verify build and all 5 CodeSnippet instances render** `est:5min`
  Run astro build and verify all 5 chapters with CodeSnippet render correctly with extracted C++ code.
  - Verify: cd site && npx astro build -- zero errors, 21 pages

## Files Likely Touched

- engine/src/euclidean.cpp
- engine/src/engine.cpp
- engine/src/envelope.cpp
- engine/src/macro.cpp
- site/src/content/docs/08-minimalism.mdx
- site/src/components/CodeSnippet.astro
