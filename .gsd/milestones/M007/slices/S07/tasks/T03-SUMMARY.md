---
id: T03
parent: S07
milestone: M007
key_files:
  - plugin/source/ui/phrase_edit_view.h
  - plugin/source/ui/phrase_edit_view.cpp
key_decisions: []
duration: 
verification_result: passed
completed_at: 2026-06-22T19:41:43.838Z
blocker_discovered: false
---

# T03: Added phrase schematic visualization, beat labels, dimming, and consistent decimal display

**Added phrase schematic visualization, beat labels, dimming, and consistent decimal display**

## What Happened

Added scaled phrase schematic bar below lane tabs showing play/gap regions in lane color. Beat labels at phrase boundaries (0, len, len+gap) with overlap prevention. Gap/Ofs knobs dim to 25% opacity and show '--' when Len=0. All knob values show consistent 1-decimal format (e.g. '12.0 bt').

## Verification

Build + Cubase UAT confirms schematic, beat labels, dimming, and decimal display

## Verification Evidence

| # | Command | Exit Code | Verdict | Duration |
|---|---------|-----------|---------|----------|
| 1 | `cmake --build build && ctest --test-dir build` | 0 | pass | 15000ms |

## Deviations

None.

## Known Issues

None.

## Files Created/Modified

- `plugin/source/ui/phrase_edit_view.h`
- `plugin/source/ui/phrase_edit_view.cpp`
