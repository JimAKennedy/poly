---
id: T04
parent: S03
milestone: M001
key_files:
  - tests/golden_tests.cpp
  - tests/golden/default_patch_4bars.txt
  - tests/CMakeLists.txt
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-20T20:05:33.949Z
blocker_discovered: false
---

# T04: Built 8-test golden determinism suite proving byte-identical output under all transport stress scenarios

**Built 8-test golden determinism suite proving byte-identical output under all transport stress scenarios**

## What Happened

Created golden_tests.cpp with 8 determinism tests: (1) same-patch-same-seed reproducibility, (2) block-size independence (0.05/0.5/2.0 PPQ), (3) loop restart produces identical sub-range, (4) position jump+continue matches straight-through, (5) different seed produces different output, (6) tempo independence in PPQ space, (7) not-playing produces zero events, (8) polymetric phase variation proves 5/16 ghost lane drifts against 4/4. Also generated golden reference file for CI regression detection (79 sorted events for default patch over 4 bars). Full suite: 18 tests, all pass in <1ms.

## Verification

18/18 tests pass (10 Euclidean + 8 GoldenDeterminism)

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `build/tests/poly_tests` | 0 | 18 tests passed (10 Euclidean + 8 GoldenDeterminism) | 50ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `tests/golden_tests.cpp`
- `tests/golden/default_patch_4bars.txt`
- `tests/CMakeLists.txt`
