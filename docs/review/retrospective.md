# M001 Milestone Retrospective

## Milestone: Definition and Architecture De-risking

**Objective:** Lock the Cubase-first VST3 model, establish poly_engine isolation boundary, and prove deterministic timing before feature work.

**Result:** All objectives achieved. The architecture is validated through implementation and testing.

---

## Slice Delivery Summary

| Slice | Title | Outcome |
|-------|-------|---------|
| S01 | VST3 Plugin Skeleton | Empty VST3 instrument builds cleanly with all warnings enabled. CMake project structure established with engine/plugin/tests separation. |
| S02 | Engine Domain Model + CLI Harness | Full data model in `poly/types.h`. Headless harness in `tools/harness/` exercises the engine without a DAW. Engine compiles without VST3 SDK. |
| S03 | Timing Determinism Prototype | `renderRange()` implemented with Euclidean rhythms, position-seeded RNG, PPQ-derived timing. 18 tests prove determinism under all transport stress scenarios. |
| S04 | Specification and Design | Five docs: PRD, engine-spec, cubase-workflow, automation-mapping, wireframes. All reference actual codebase types. |
| S05 | Milestone Review Documentation | Architecture decisions audit, test coverage analysis, API surface audit, retrospective (this document). |

## What Worked Well

### Engine Isolation
The decision to make `poly_engine` a pure C++ static library with zero VST3 dependencies paid off immediately. The entire engine development (S02–S03) proceeded without ever opening a DAW. Tests run in milliseconds.

### Position-Seeded RNG
Using `deterministicRand()` instead of a sequential PRNG was the single most impactful technical decision. It made block-size independence trivially correct rather than something that needs careful accumulator management.

### PPQ-Derived Timing
Absolute PPQ positioning eliminated an entire class of state-management bugs. The engine has no mutable state — `renderRange()` is a pure function. This made the golden tests straightforward to write.

### Data Model First
Defining the complete data model in S02 (before implementing `renderRange()` in S03) meant the implementation had a clear target. The types acted as a spec that the code had to satisfy.

### C++20 Designated Initializers
`{.steps = 4, .subdivision = 4}` in test code (golden_tests.cpp) is significantly more readable than positional construction. This validates C++20 for the jk.digital portfolio.

## What Could Improve

### Documentation Last
S04 (docs) was the fourth of five slices. Writing the engine spec after implementation meant the spec describes what was built rather than what should be built. For Phase 1, consider writing specs alongside or before implementation for complex features (envelopes, macros).

### Slice Titles in Roadmap
S01–S03 roadmap entries show "S01: S01", "S02: S02" rather than descriptive titles. The GSD tooling renders slice IDs as titles when no title is provided. Minor but reduces roadmap readability.

### No CI Yet
Tests are verified locally but there's no CI pipeline. A GitHub Actions workflow for macOS + Windows builds would catch platform-specific issues early.

### No Static Analysis
`jk_warnings.cmake` enables compiler warnings but clang-tidy is not yet integrated. For Phase 1's real-time safety requirements, a clang-tidy check for heap allocation in `renderRange()` would be valuable.

## Risks Retired

| Risk | How It Was Retired |
|------|-------------------|
| Engine might depend on VST3 SDK | Engine compiles and passes tests without SDK on link path |
| Block-size-dependent output | Golden tests prove identical output at 0.05, 0.5, 2.0 PPQ block sizes |
| Loop restart drift | Golden test proves bars 0–2 from loop = bars 0–2 from straight-through |
| Position jump inconsistency | Golden test proves jump-to-bar-2 = straight-through bar 2 |
| Tempo change affects PPQ events | Golden test proves 120 BPM and 90 BPM produce identical PPQ positions |
| Euclidean algorithm correctness | 10 unit tests including maximal evenness, rotation preservation, edge cases |

## Risks Remaining

| Risk | Severity | Mitigation |
|------|----------|------------|
| Envelope evaluation complexity | Medium | Design shapes and superposition carefully; test each shape independently |
| Macro coherence | Medium | Define exact parameter mappings before implementing; test macro = 0.0 and 1.0 extremes |
| Real-time safety violations | High | Add clang-tidy RT safety checks; audit every function called from `renderRange()` |
| VST3 state version migration | Medium | Test serialization round-trips in every build; never ship without version number |
| `NoteEventBuffer` overflow in dense patches | Low | Add diagnostic counter; test with maximum density scenarios |
| VSTGUI learning curve | Medium | Spike on a single view (lane overview) before committing to all three views |
| Cross-block note-off scheduling | Medium | Implement `NoteOffScheduler` early in Phase 1; test with short and long notes across block boundaries |

## Recommendations for Phase 1

1. **Implement accent masks first** — simplest unimplemented feature, validates the velocity pipeline
2. **Envelope evaluation second** — core differentiator, design the shape functions before coding
3. **Write a CI pipeline early** — catch Windows build issues before they accumulate
4. **Add clang-tidy RT safety checks** — automated guard against allocations in the audio path
5. **Spike VSTGUI** — build just the lane overview before committing to the full wireframe plan
6. **State serialization** — implement before any preset testing to avoid version zero tech debt
7. **Note-off scheduling** — required before the plugin can produce usable MIDI in Cubase

## Key Metrics

| Metric | Value |
|--------|-------|
| Slices completed | 5/5 |
| Tests passing | 18/18 |
| Engine source lines | ~80 |
| Test source lines | ~290 |
| Documentation pages | 10 (5 specs + 5 review) |
| Public API symbols | 21 (3 functions + 13 types + 5 constants) |
| Architecture decisions | 6 |
| Risks retired | 6 |
| Risks remaining | 7 |
