# S03: Content Migration from Guide to MDX

**Goal:** Migrate all guide content from monolithic markdown to individual MDX pages with custom components
**Demo:** All 17 sections plus 2 appendices render as navigable chapters with PolyPatch callouts replacing parameter tables, ListenFor boxes for listening guidance, and EuclideanDiagram calls for every E(k,n) reference

## Must-Haves

- Complete the planned slice outcomes.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Migrate all chapters and appendices to MDX** `est:2h`
  Convert all 17 chapters plus 2 appendices to individual MDX files with PolyPatch, ListenFor, EuclideanDiagram, and CodeSnippet components
  - Files: `site/src/content/docs/`
  - Verify: astro build succeeds with 21 pages, zero errors

## Files Likely Touched

- site/src/content/docs/
