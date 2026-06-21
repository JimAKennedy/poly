---
estimated_steps: 1
estimated_files: 1
skills_used: []
---

# T01: Add UI test flags and visual diff upload to ci.yml

Add -DBUILD_INTERACTION_TESTS=ON to all matrix entries. Add -DBUILD_VISUAL_TESTS=ON to macOS only. Add conditional artifact upload step for visual test diffs on macOS failure.

## Inputs

- `.github/workflows/ci.yml`

## Expected Output

- `.github/workflows/ci.yml with UI test integration`

## Verification

Review ci.yml for correct matrix flags and artifact upload logic
