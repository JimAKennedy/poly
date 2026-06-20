---
id: T02
parent: S03
milestone: M001
key_files:
  - engine/src/engine.cpp
key_decisions:
  - Phase derived from absolute PPQ, never accumulated
  - Velocity expressed as 0-1 float (baseVelocity/127)
  - Note duration set to half step length
  - Event ordering within a block follows lane index order — golden tests should sort by PPQ+pitch
duration: 
verification_result: passed
completed_at: 2026-06-20T20:03:11.658Z
blocker_discovered: false
---

# T02: Implemented core cycle timing in renderRange() with PPQ-based absolute positioning

**Implemented core cycle timing in renderRange() with PPQ-based absolute positioning**

## What Happened

Implemented the core rendering loop in Engine::renderRange(). For each active lane: computes step PPQ from subdivision, maps absolute step indices into the Euclidean pattern via cycle modulo, and emits NoteEvents for hits falling within [ppqStart, ppqEnd). All phase computation derives from absolute PPQ position — no accumulators. Verified block-size independence: identical event sets across block sizes 0.1, 0.5, and 1.0 PPQ (within-block lane ordering varies but event set is identical when sorted).

## Verification

Harness produces 47 events over 2 bars with 4 lanes. Sorted event sets identical across three different block sizes (0.1, 0.5, 1.0 PPQ).

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `poly_harness 2 120 (0.5|0.1|1.0) | sort | diff` | 0 | Identical event sets across block sizes | 100ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/src/engine.cpp`
