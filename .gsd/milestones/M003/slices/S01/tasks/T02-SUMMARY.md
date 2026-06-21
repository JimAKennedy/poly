---
id: T02
parent: S01
milestone: M003
key_files:
  - engine/include/poly/types.h
  - engine/src/envelope.cpp
  - engine/include/poly/envelope.h
  - engine/include/poly/state_io.h
key_decisions:
  - State version bumped 1->2; v1 read path preserved for backwards compat
  - Curve uses exp-based easing rather than power curve for smoother control
  - StepList uses 16 fixed entries (kMaxStepListEntries) to avoid heap allocation
duration: 
verification_result: passed
completed_at: 2026-06-21T01:26:10.104Z
blocker_discovered: false
---

# T02: Implemented Curve (exponential ease) and StepList (quantized lookup) shapes with new Envelope fields and state version bump

**Implemented Curve (exponential ease) and StepList (quantized lookup) shapes with new Envelope fields and state version bump**

## What Happened

Added curvature (float), stepValues (array of 16 floats), and stepCount (int) to the Envelope struct. Curve shape uses exp() for positive curvature (ease-in) and inverted exp() for negative curvature (ease-out), falling back to linear when curvature is near zero. StepList quantizes phase to stepCount bins and returns the corresponding value. Added evaluateShapeFull() that takes the full Envelope struct for shapes needing extra parameters. Bumped kCurrentStateVersion to 2 with backwards-compatible v1 read path.

## Verification

cmake --build build && ctest --test-dir build: 133/133 pass including new shape tests

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `ctest --test-dir build --output-on-failure` | 0 | 133/133 tests pass | 1630ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/include/poly/types.h`
- `engine/src/envelope.cpp`
- `engine/include/poly/envelope.h`
- `engine/include/poly/state_io.h`
