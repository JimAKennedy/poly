---
id: T03
parent: S05
milestone: M003
key_files:
  - plugin/source/processor.h
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
  - plugin/source/plugids.h
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-21T03:21:22.419Z
blocker_discovered: false
---

# T03: Wired capture buffer into processor with RT-safe staging, export trigger param, and IMessage-based retrieval

**Wired capture buffer into processor with RT-safe staging, export trigger param, and IMessage-based retrieval**

## What Happened

Integrated MidiCaptureBuffer into PolyProcessor. Events pushed after renderRange (RT-safe). Buffer cleared on transport jump and setActive. Export trigger uses atomic staging pattern: process() copies events to staging array and sets atomic flag; processor's notify() handler responds to controller's RequestMidiExport message by sorting staged events, generating SMF, and sending binary data back via IMessage. Added kExportTrigger, kCaptureLength, kCaptureReady parameters in Export unit.

## Verification

Build succeeds, all 177 tests pass

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --test-dir build` | 0 | pass | 20000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `plugin/source/processor.h`
- `plugin/source/processor.cpp`
- `plugin/source/controller.cpp`
- `plugin/source/plugids.h`
