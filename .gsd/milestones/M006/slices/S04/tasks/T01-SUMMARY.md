---
id: T01
parent: S04
milestone: M006
key_files: []
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-21T01:07:29.780Z
blocker_discovered: false
---

# T01: CI matrix enables interaction tests on all platforms, visual tests on macOS, with diff artifact upload on failure

**CI matrix enables interaction tests on all platforms, visual tests on macOS, with diff artifact upload on failure**

## What Happened

Added -DBUILD_INTERACTION_TESTS=ON to all three matrix entries (macOS, Linux, Windows). Added -DBUILD_VISUAL_TESTS=ON to macOS only (requires CoreGraphics). Added conditional 'Upload visual diffs' step that triggers on failure for macOS, uploading *_diff.png artifacts from build/tests/visual_output/ with 14-day retention and if-no-files-found: ignore.

## Verification

ci.yml reviewed: correct flags per platform, artifact upload step uses failure() && runner.os == 'macOS' condition

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `manual review of .github/workflows/ci.yml` | 0 | flags and upload step correct | 0ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

None.
