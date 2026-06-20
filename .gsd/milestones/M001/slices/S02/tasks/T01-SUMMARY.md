---
id: T01
parent: S02
milestone: M001
key_files:
  - engine/include/poly/types.h
  - engine/include/poly/engine.h
  - engine/src/engine.cpp
  - plugin/source/processor.h
key_decisions:
  - Used fixed-size std::array everywhere instead of std::vector for RT safety
  - Added sampleRate/blockSize/playing/loopStartPpq/loopEndPpq to TransportContext for future timing work
  - LaneConfig.active flag enables partial lane activation without resizing
duration: 
verification_result: passed
completed_at: 2026-06-20T19:55:34.816Z
blocker_discovered: false
---

# T01: Full engine domain model (LaneConfig, Envelope, GrooveState, MacroValues) with fixed-size containers for RT safety

**Full engine domain model (LaneConfig, Envelope, GrooveState, MacroValues) with fixed-size containers for RT safety**

## What Happened

Expanded types.h with the complete domain model from IMPLEMENTATION_PLAN.md §3: Role enum (8 roles), Cycle struct, AccentMask (fixed array), EnvTarget (8 targets), Shape (5 shapes), Envelope, EnvelopeAssign, LaneConfig with all fields, MacroValues (6 macros), and GrooveState. All containers use std::array with compile-time limits (kMaxLanes=8, kMaxSteps=64, kMaxEnvelopesPerLane=4, kMaxGlobalEnvelopes=8) — no std::vector anywhere in the hot path. Updated Engine::renderRange() to accept const GrooveState&. Added engine_/grooveState_/noteBuffer_ members to PolyProcessor for future wiring.

## Verification

poly_engine and poly_plugin both build clean. One expected -Wunused-private-field warning on engine_ (not wired until M002 S05).

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build --target poly_engine` | 0 | pass | 1200ms |
| 2 | `cmake --build build --target poly_plugin` | 0 | pass (1 expected unused field warning) | 8000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/include/poly/types.h`
- `engine/include/poly/engine.h`
- `engine/src/engine.cpp`
- `plugin/source/processor.h`
