---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Migrate all chapters and appendices to MDX

Convert all 17 chapters plus 2 appendices to individual MDX files with PolyPatch, ListenFor, EuclideanDiagram, and CodeSnippet components

## Inputs

- `site/src/content/docs/index.mdx`

## Expected Output

- `site/src/content/docs/01-foundations.mdx`
- `site/src/content/docs/appendix-presets.mdx`

## Verification

astro build succeeds with 21 pages, zero errors
