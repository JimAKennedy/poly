---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T03: Verify build and all 5 CodeSnippet instances render

Run astro build and verify all 5 chapters with CodeSnippet render correctly with extracted C++ code.

## Inputs

- `site/src/components/CodeSnippet.astro`

## Expected Output

- `Successful build output`

## Verification

cd site && npx astro build -- zero errors, 21 pages
