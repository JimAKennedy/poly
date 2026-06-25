---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Implement live code extraction in CodeSnippet.astro

Replace placeholder rendering with build-time file reading (fs.readFileSync), region extraction, and syntax-highlighted output using Astro Code component. Add GitHub source link in header.

## Inputs

- `engine/src/euclidean.cpp`
- `engine/src/engine.cpp`
- `engine/src/envelope.cpp`
- `engine/src/macro.cpp`

## Expected Output

- `Updated CodeSnippet.astro with live extraction`

## Verification

cd site && npx astro build succeeds with zero errors
