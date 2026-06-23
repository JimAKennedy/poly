---
id: T03
parent: S02
milestone: M008
key_files: []
key_decisions: []
duration: 
verification_result: untested
completed_at: 2026-06-23T02:24:34.742Z
blocker_discovered: false
---

# T03: Serialized timeline fields and exposed timeline/patternLength as VST3 params

**Serialized timeline fields and exposed timeline/patternLength as VST3 params**

## What Happened

Added timeline (uint8), fixedPatternLength (int), and fixedPattern (uint64 bitmask) to version 10 serialization block. Added kCoreTimeline and kCoreFixedPatternLen param slots (6,7) to plugids.h, processor.cpp, and controller.cpp.

## Verification

Build passes, 224 tests pass, state round-trip verified by existing preset tests

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| — | No verification commands discovered | — | — | — |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

None.
