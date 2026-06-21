---
id: T04
parent: S01
milestone: M003
key_files:
  - tests/golden_tests.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-21T01:26:21.622Z
blocker_discovered: false
---

# T04: Golden tests verified unchanged for default patch; added ExtendedEnvelopeTargetsBlockIndependence golden test

**Golden tests verified unchanged for default patch; added ExtendedEnvelopeTargetsBlockIndependence golden test**

## What Happened

Confirmed existing golden reference file unchanged (default patch has no envelopes active). Added test 15 (ExtendedEnvelopeTargetsBlockIndependence) that exercises AccentBias on kick, NoteLength on snare, TimingLooseness on hi-hat, ActivationWeight on ghost lane, and global FillLikelihood envelope simultaneously — verified identical output across 0.05, 0.5, and 2.0 PPQ block sizes.

## Verification

ctest --test-dir build -R Golden: 15/15 pass

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `ctest --test-dir build --output-on-failure -R Golden` | 0 | 15/15 golden tests pass | 80ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `tests/golden_tests.cpp`
