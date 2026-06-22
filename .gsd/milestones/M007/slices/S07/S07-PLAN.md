# S07: Phrase UI Controls

**Goal:** Add a PHRASE section to the UI with lane selector tabs, 3 knobs (Len, Gap, Ofs) per lane, phrase schematic visualization, beat labels, and dimmed controls when inactive
**Demo:** 

## Must-Haves

- PhraseEditView renders with 8 colored lane tabs, 3 draggable knobs, phrase schematic bar, beat labels, and dimmed Gap/Ofs when Len=0

## Proof Level

- This slice proves: Build + all tests pass + Cubase UAT

## Integration Closure

PhraseEditView registered in controller; .uidesc layout updated; window height 628px

## Verification

- Run the task and slice verification checks for this slice.

## Tasks

- [x] **T01: Create PhraseEditView with knobs and lane tabs** `est:45min`
  VSTGUI CView with 8 lane selector tabs and 3 drawn knobs (Len, Gap, Ofs). Corona arc + handle dot rendering. Vertical drag interaction.
  - Files: `plugin/source/ui/phrase_edit_view.h`, `plugin/source/ui/phrase_edit_view.cpp`
  - Verify: Build compiles

- [x] **T02: Wire into UI layout and deploy** `est:30min`
  Register in controller createCustomView, update uidesc layout, add to CMakeLists, build and deploy.
  - Files: `plugin/source/controller.cpp`, `plugin/resource/poly.uidesc`, `plugin/CMakeLists.txt`
  - Verify: cmake --build build && ctest --test-dir build

- [x] **T03: Phrase schematic, beat labels, dimming, decimal display** `est:1h`
  Scaled phrase schematic bar below lane tabs. Beat labels with overlap prevention. Dim Gap/Ofs when Len=0. Consistent 1-decimal display.
  - Files: `plugin/source/ui/phrase_edit_view.h`, `plugin/source/ui/phrase_edit_view.cpp`
  - Verify: Build + Cubase UAT confirms visuals

## Files Likely Touched

- plugin/source/ui/phrase_edit_view.h
- plugin/source/ui/phrase_edit_view.cpp
- plugin/source/controller.cpp
- plugin/resource/poly.uidesc
- plugin/CMakeLists.txt
