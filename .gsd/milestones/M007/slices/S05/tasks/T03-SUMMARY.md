---
id: T03
parent: S05
milestone: M007
key_files:
  - engine/include/poly/state_io.h
  - tests/golden_tests.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-22T21:16:47.382Z
blocker_discovered: false
---

# T03: State version 9 serialization and 5 golden tests for kotekan

**State version 9 serialization and 5 golden tests for kotekan**

## What Happened

Bumped kCurrentStateVersion to 9 with kotekanSourceLane read/write in version >= 9 blocks. Added 5 golden tests: no-source matches baseline (T36), pair produces exact complement with correct hit counts (T37), different velocities per voice (T38), circular reference fallback (T39), and block-size independence (T40).

## Verification

All 216 tests pass including 5 new kotekan golden tests

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `ctest --test-dir build --output-on-failure` | 0 | 216/216 pass | 1500ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `engine/include/poly/state_io.h`
- `tests/golden_tests.cpp`
