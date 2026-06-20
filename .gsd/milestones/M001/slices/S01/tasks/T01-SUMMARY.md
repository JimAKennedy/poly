---
id: T01
parent: S01
milestone: M001
key_files:
  - CMakeLists.txt
  - cmake/jk_warnings.cmake
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-20T19:51:34.131Z
blocker_discovered: false
---

# T01: Top-level CMakeLists.txt with VST3 SDK v3.7.12 via FetchContent, C++20, jk_warnings.cmake

**Top-level CMakeLists.txt with VST3 SDK v3.7.12 via FetchContent, C++20, jk_warnings.cmake**

## What Happened

Created the root CMakeLists.txt that fetches the Steinberg VST3 SDK via FetchContent, sets C++20 standard, disables SDK examples/validator, enables VSTGUI support, and adds engine/ and plugin/ subdirectories. The jk_suppress_sdk_warnings() function was extended to also suppress deprecation warnings on sdk and sdk_common targets (they use deprecated std::wstring_convert internally).

## Verification

cmake -B build -DCMAKE_BUILD_TYPE=Debug completes without error; SDK is fetched and configured

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake -B build -DCMAKE_BUILD_TYPE=Debug` | 0 | pass | 17800ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `CMakeLists.txt`
- `cmake/jk_warnings.cmake`
