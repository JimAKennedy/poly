---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T02: Pin VST3 SDK FetchContent to commit hash

cmake-fetchcontent-pinning (amber/medium) flags that VST3 SDK uses tag `v3.7.12_build_20` instead of a commit hash in CMakeLists.txt:48. Resolve the tag to its commit SHA and use GIT_TAG with the full hash for reproducibility.

## Inputs

- `CMakeLists.txt`

## Expected Output

- `CMakeLists.txt with VST3 SDK pinned to commit hash`

## Verification

cmake -S . -B build-check (configure succeeds with commit hash)
