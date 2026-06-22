# S01: Phrase System

**Goal:** Add phraseLength, phraseGap, and phraseOffset to LaneConfig so lanes cycle through independent play/rest periods, enabling Afro-House offset phrase behavior and Reich-style construction/reduction
**Demo:** Load Poly in Cubase — lanes play for N bars then rest for M bars, with different phrase lengths per lane creating offset polyrhythmic phrases

## Must-Haves

- Lanes respect phraseLength/phraseGap config; different phrase lengths per lane create offset polyrhythmic phrases; envelope phase stays correct across gaps; golden tests pass with phrase configs; state serialization handles new fields

## Proof Level

- This slice proves: Golden tests with phrase configs + Cubase UAT

## Integration Closure

Phrase state integrated into renderRange pipeline; envelope calculations account for gap periods; state serialization includes phrase params with version bump

## Verification

- Phase alignment view updated to show phrase boundaries

## Tasks

- [x] **T01: Add phrase fields to LaneConfig and types** `est:30min`
  Add phraseLength (float, bars, 0=continuous), phraseGap (float, bars, 0=no gap), phraseOffset (float, bars, 0=no offset) to LaneConfig in types.h. phraseLength=0 means continuous (backward compatible). Update kStateVersion.
  - Files: `engine/include/poly/types.h`
  - Verify: Build compiles with no warnings; existing tests pass unchanged (default values preserve backward compat)

- [x] **T02: Implement phrase gating in renderRange** `est:1h`
  In renderRange(), after computing absStep and ppq, compute phrase phase from absolute PPQ position:
  - phraseCyclePpq = (phraseLength + phraseGap) * 4.0 (bars to PPQ)
  - phrasePos = fmod(ppq - phraseOffset * 4.0, phraseCyclePpq)
  - if phrasePos >= phraseLength * 4.0, skip the step (we're in the gap)
  Must handle phraseLength=0 (continuous) as no-op. Must derive from absolute PPQ, not accumulate.
  - Files: `engine/src/engine.cpp`
  - Verify: Build + existing tests pass; manual inspection confirms phrase gating logic derives from absolute PPQ

- [x] **T03: State serialization for phrase params** `est:30min`
  Update processor.cpp getState()/setState() to serialize phraseLength, phraseGap, phraseOffset per lane. Bump kStateVersion. Branch on version in setState() for backward compat with old presets.
  - Files: `plugin/source/processor.cpp`, `plugin/source/processor.h`
  - Verify: Build compiles; RT safety check passes

- [x] **T04: Golden tests for phrase system** `est:1h`
  Add golden test scenarios:
  1. Single lane with phraseLength=2, phraseGap=1 - verify silence during gap bars
  2. Two lanes with different phraseLength values - verify offset phrase behavior
  3. Phrase with transport jump into gap region - verify correct silence
  4. Phrase with loop restart - verify deterministic output
  5. phraseLength=0 (continuous) produces identical output to current baseline
  - Files: `tests/golden_tests.cpp`, `tests/golden/`
  - Verify: ctest --test-dir build -R golden passes

- [x] **T05: Phase alignment view shows phrase boundaries** `est:45min`
  Update phase_alignment_view to draw phrase boundaries (play/gap regions) as colored bands behind the phase indicators. Play regions in lane color, gap regions dimmed/hatched.
  - Files: `plugin/source/ui/phase_alignment_view.cpp`, `plugin/source/ui/phase_alignment_view.h`
  - Verify: Build compiles; visual smoke test passes; Cubase UAT shows phrase boundaries in UI

## Files Likely Touched

- engine/include/poly/types.h
- engine/src/engine.cpp
- plugin/source/processor.cpp
- plugin/source/processor.h
- tests/golden_tests.cpp
- tests/golden/
- plugin/source/ui/phase_alignment_view.cpp
- plugin/source/ui/phase_alignment_view.h
