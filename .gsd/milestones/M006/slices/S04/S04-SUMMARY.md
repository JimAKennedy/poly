---
id: S04
parent: M006
milestone: M006
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - .github/workflows/ci.yml
key_decisions: []
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-21T01:07:40.618Z
blocker_discovered: false
---

# S04: CI Pipeline Integration

**CI runs interaction tests on all platforms, visual regression tests on macOS with diff artifact upload on failure**

## What Happened

Updated ci.yml build matrix to enable UI test CMake flags: -DBUILD_INTERACTION_TESTS=ON on all three platforms (macOS, Linux, Windows) and -DBUILD_VISUAL_TESTS=ON on macOS only (visual harness requires CoreGraphics/ImageIO). Interaction tests use GTEST_SKIP on non-macOS so they compile everywhere but only run on macOS. Added conditional artifact upload step that captures *_diff.png files from the visual test output directory when the build job fails on macOS, providing immediate visual feedback on regressions.

## Verification

ci.yml reviewed for correct matrix flags and conditional artifact upload logic

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
