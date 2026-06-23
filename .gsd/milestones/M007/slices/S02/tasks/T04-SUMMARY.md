---
id: T04
parent: S02
milestone: M007
key_files:
  - tests/golden_tests.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-22T19:48:36.845Z
blocker_discovered: false
---

# T04: Added 4 golden tests covering mutation determinism, anchors, and block independence

**Added 4 golden tests covering mutation determinism, anchors, and block independence**

## What Happened

Added 4 test cases: (1) ZeroRateMatchesBaseline — mutationRate=0 byte-identical to default, (2) DeterministicVariations — mutationRate=0.3 is repeatable and differs from baseline, (3) RespectsAnchors — anchor steps produce identical output with/without mutation at rate=1.0, (4) BlockSizeIndependence — mutationRate=0.4 deterministic across 0.05/0.5/2.0 PPQ block sizes. Initial anchor test was too strict (expected exact baseVelocity, forgot velocity spread); fixed to compare anchor events between mutated and unmutated runs.

## Verification

All 4 new GoldenMutation tests pass; all 201 tests pass

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `ctest --test-dir build -R GoldenMutation --output-on-failure` | 0 | 4/4 mutation tests pass | 70ms |
| 2 | `ctest --test-dir build` | 0 | 201/201 tests pass | 1500ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `tests/golden_tests.cpp`
