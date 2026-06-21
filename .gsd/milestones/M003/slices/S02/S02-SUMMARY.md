---
id: S02
parent: M003
milestone: M003
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - engine/include/poly/scene.h
  - engine/src/scene.cpp
  - engine/include/poly/types.h
  - engine/include/poly/state_io.h
  - plugin/source/processor.h
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
  - tests/scene_tests.cpp
key_decisions:
  - Morph interpolation uses linear lerp for numeric fields, discrete snap at t=0.5 for enums/note values
  - State version bumped to v3 (v1→sceneA copy, v2→sceneA copy)
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-21T03:10:39.547Z
blocker_discovered: false
---

# S02: Scene and Snapshot System

**A/B scene system with crossfade morph interpolation, state v3 serialization with backwards compat**

## What Happened

Implemented complete A/B scene system: SceneState with two GrooveState slots, SceneSelect enum (A/B/Morph), morph amount (0.0-1.0). interpolateGrooveState lerps all numeric fields with discrete snap at t=0.5. Plugin integration adds kSceneSelect and kSceneMorph automatable parameters. State version bumped to v3 with backwards-compat read paths for v1 and v2. Comprehensive scene_tests cover interpolation boundaries, discrete field snapping, serialization round-trip, and backwards compatibility.

## Verification

All 156 tests pass including scene_tests covering interpolation, serialization round-trip, and backwards compat. Commit 3fabe58.

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

None.
