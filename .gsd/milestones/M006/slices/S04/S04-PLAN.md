# S04: CI Pipeline Integration

**Goal:** Enable interaction tests on all CI platforms and visual regression tests on macOS, with diff artifact upload on visual test failure
**Demo:** CI runs interaction tests on every push, visual tests on macOS with diff artifact upload on failure

## Must-Haves

- Complete the planned slice outcomes.

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Add UI test flags and visual diff upload to ci.yml** `est:10min`
  Add -DBUILD_INTERACTION_TESTS=ON to all matrix entries. Add -DBUILD_VISUAL_TESTS=ON to macOS only. Add conditional artifact upload step for visual test diffs on macOS failure.
  - Files: `.github/workflows/ci.yml`
  - Verify: Review ci.yml for correct matrix flags and artifact upload logic

## Files Likely Touched

- .github/workflows/ci.yml
