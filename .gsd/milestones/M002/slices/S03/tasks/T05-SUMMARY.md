---
id: T05
parent: S03
milestone: M002
key_files:
  - tests/swing_humanize_tests.cpp
  - tests/golden_tests.cpp
  - tests/CMakeLists.txt
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-20T21:33:50.022Z
blocker_discovered: false
---

# T05: Added 13 swing/humanize/duration unit tests plus golden determinism test for block independence

**Added 13 swing/humanize/duration unit tests plus golden determinism test for block independence**

## What Happened

Created tests/swing_humanize_tests.cpp with 13 tests covering: swing offset at various amounts, swing determinism, humanize jitter bounds and determinism, humanize zero/different-seed, combined swing+humanize, and note duration (default, custom, staccato). Added SwingHumanizeBlockIndependence golden test to golden_tests.cpp verifying deterministic output across 0.05/0.5/2.0 PPQ block sizes.

## Verification

All 64 tests pass (50 existing + 14 new)

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `./build/tests/poly_tests` | 0 | 64 tests passed | 2ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `tests/swing_humanize_tests.cpp`
- `tests/golden_tests.cpp`
- `tests/CMakeLists.txt`
