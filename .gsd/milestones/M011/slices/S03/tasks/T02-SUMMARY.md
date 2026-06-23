---
id: T02
parent: S03
milestone: M011
key_files:
  - CMakeLists.txt
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-23T14:20:49.386Z
blocker_discovered: false
---

# T02: Pinned VST3 SDK FetchContent to commit hash cc2adc903

**Pinned VST3 SDK FetchContent to commit hash cc2adc903**

## What Happened

Resolved v3.7.12_build_20 tag to commit cc2adc90382dded9e347caf74e4532f1458715db using git ls-remote. Replaced the tag with the full hash in CMakeLists.txt, keeping the tag as a trailing comment. CMake configure and build both succeed with the hash.

## Verification

cmake -S . -B build configures successfully; cmake --build build succeeds

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake -S . -B build` | 0 | pass | 700ms |
| 2 | `cmake --build build` | 0 | pass | 15000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `CMakeLists.txt`
