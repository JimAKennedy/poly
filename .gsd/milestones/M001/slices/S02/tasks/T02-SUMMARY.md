---
id: T02
parent: S02
milestone: M001
key_files:
  - tools/harness/main.cpp
  - tools/harness/CMakeLists.txt
  - CMakeLists.txt
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-20T19:56:38.406Z
blocker_discovered: false
---

# T02: Headless CLI harness that runs the engine over simulated transport and prints events in diffable text format

**Headless CLI harness that runs the engine over simulated transport and prints events in diffable text format**

## What Happened

Created tools/harness/ with a CLI program that: instantiates Engine, creates a 4-lane GrooveState with musically sensible defaults (kick 4-on-floor, snare backbeat, hi-hat 8ths, ghost tom in polymetric 5/16), iterates renderRange() over configurable transport blocks, and prints NoteEvents to stdout. The harness links only against poly_engine — no VST3 SDK dependency. Output format is diffable text suitable for golden tests in S03. Accepts CLI args for bars, tempo, and block size.

## Verification

poly_harness builds and runs, producing expected header and 0 events (engine stub).

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build --target poly_harness` | 0 | pass | 2000ms |
| 2 | `./build/tools/harness/poly_harness` | 0 | pass — 0 events from stub engine, correct header output | 50ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `tools/harness/main.cpp`
- `tools/harness/CMakeLists.txt`
- `CMakeLists.txt`
