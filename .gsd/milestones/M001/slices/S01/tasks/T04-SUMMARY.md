---
id: T04
parent: S01
milestone: M001
key_files:
  - .gitignore
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-20T19:51:54.800Z
blocker_discovered: false
---

# T04: Full clean build verified: zero project warnings, .vst3 bundle present, engine isolation confirmed

**Full clean build verified: zero project warnings, .vst3 bundle present, engine isolation confirmed**

## What Happened

Ran cmake --build --clean-first and verified: (1) zero warnings from poly_engine and poly_plugin source files (-Wall -Wextra enabled), (2) SDK warnings suppressed via jk_suppress_sdk_warnings(), (3) .vst3 bundle produced at build/VST3/Debug/poly_plugin.vst3, (4) poly_engine has zero VST3 SDK references.

## Verification

cmake --build --clean-first with warning grep returns zero project warnings; find confirms .vst3 bundle

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build --clean-first 2>&1 | grep 'warning:' | grep -v '_deps/' | grep -v 'duplicate libraries' | grep -v 'Xcode.app'` | 1 | pass - zero project warnings | 45000ms |
| 2 | `find build -name '*.vst3' -type d` | 0 | pass - bundle exists | 50ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `.gitignore`
