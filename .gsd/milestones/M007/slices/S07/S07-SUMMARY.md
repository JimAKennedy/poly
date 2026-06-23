---
id: S07
parent: M007
milestone: M007
provides:
  - (none)
requires:
  []
affects:
  []
key_files:
  - plugin/source/ui/phrase_edit_view.h
  - plugin/source/ui/phrase_edit_view.cpp
  - plugin/source/controller.cpp
  - plugin/resource/poly.uidesc
key_decisions: []
patterns_established:
  - (none)
observability_surfaces:
  - none
drill_down_paths:
  []
duration: ""
verification_result: passed
completed_at: 2026-06-22T19:41:58.898Z
blocker_discovered: false
---

# S07: Phrase UI Controls

**PHRASE UI section with lane tabs, knobs, schematic visualization, beat labels, dimming, and consistent decimal display**

## What Happened

Built PhraseEditView with 8 colored lane tabs and 3 knobs (Len, Gap, Ofs) per lane. Added phrase schematic bar showing scaled play/gap regions with beat labels at boundaries. Gap/Ofs knobs dim when Len=0. All values show consistent 1-decimal format in beats. Wired into controller, uidesc, and deployed to Cubase for UAT.

## Verification

All 197 tests pass. Cubase UAT confirms knob interaction, schematic visualization, beat labels, dimming behavior, and decimal display.

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
