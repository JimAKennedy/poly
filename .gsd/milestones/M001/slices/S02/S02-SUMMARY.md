---
id: S02
parent: M001
milestone: M001
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - engine/include/poly/types.h
  - engine/include/poly/engine.h
  - engine/src/engine.cpp
  - tools/harness/main.cpp
  - tools/harness/CMakeLists.txt
  - plugin/source/processor.h
key_decisions:
  - Fixed-size std::array everywhere instead of std::vector for RT safety
  - Harness output format designed for golden test diffing
  - GrooveState added as member to PolyProcessor for future process() wiring
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-20T19:57:15.832Z
blocker_discovered: false
---

# S02: Engine Domain Model and Headless Harness

**Full domain model with RT-safe fixed-size containers, headless CLI harness, and verified engine isolation**

## What Happened

Implemented the complete engine domain model from IMPLEMENTATION_PLAN.md §3: Role (8 roles), Cycle, AccentMask, EnvTarget (8 targets), Shape (5 shapes), Envelope, EnvelopeAssign, LaneConfig, MacroValues (6 macros), and GrooveState. All containers use std::array with compile-time limits — no std::vector anywhere in the domain model. Built a headless CLI harness (tools/harness/) that simulates transport and prints events in a diffable text format for golden tests. Verified complete engine isolation: zero Steinberg symbols in libpoly_engine.a or poly_harness.

## Verification

Engine and harness build clean. Harness runs and produces correct output. nm/grep confirm zero VST3 dependency in engine.

## Requirements Advanced

None.

## Requirements Validated

None.

## New Requirements Surfaced

None.

## Requirements Invalidated or Re-scoped

None.

## Operational Readiness

None.

## Deviations

None.

## Known Limitations

None.

## Follow-ups

None.

## Files Created/Modified

- `engine/include/poly/types.h` — Full domain model: Role, Cycle, AccentMask, EnvTarget, Shape, Envelope, LaneConfig, MacroValues, GrooveState
- `engine/include/poly/engine.h` — renderRange() now accepts const GrooveState&
- `engine/src/engine.cpp` — Updated signature to match
- `plugin/source/processor.h` — Added Engine, GrooveState, NoteEventBuffer members
- `tools/harness/main.cpp` — New headless CLI harness
- `tools/harness/CMakeLists.txt` — Harness build target
- `CMakeLists.txt` — Added tools/harness subdirectory
