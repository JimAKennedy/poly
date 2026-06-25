# M001 Test Coverage Analysis

## Current Test Suite

**18 tests total** across 2 test files, all passing.

### Euclidean Algorithm Tests (10 tests)

| Test | What It Covers | Status |
|------|---------------|--------|
| FourOnTheFloor | E(4,4) = all hits | Pass |
| SinglePulse | E(1,4) = one hit at position 0 | Pass |
| Tresillo_3_8 | E(3,8) = 10010010 (known musical pattern) | Pass |
| Cinquillo_5_8 | E(5,8) = 5 hits in 8 steps | Pass |
| ThreeInSixteen | E(3,16) = sparse distribution | Pass |
| ZeroPulses | E(0,8) = no hits (edge case) | Pass |
| MorePulsesThanSteps | E(10,4) = all filled (clamped) | Pass |
| RotationShiftsPattern | Rotation changes pattern; full rotation wraps | Pass |
| RotationPreservesHitCount | All rotations maintain same hit count | Pass |
| MaximallyEvenSpacing | E(3,8) gaps differ by at most 1 | Pass |

**Assessment:** Euclidean algorithm thoroughly covered. Edge cases (0 hits, overflow, all rotations) tested. Maximal evenness property verified.

### Golden Determinism Tests (8 tests)

| Test | What It Proves | Status |
|------|---------------|--------|
| SamePatchSameSeed | Identical inputs → identical output | Pass |
| BlockSizeIndependence | 0.05, 0.5, 2.0 PPQ blocks → same events | Pass |
| LoopRestart | Bars 0–2 from loop = bars 0–2 from straight | Pass |
| PositionJump | Jump to bar 2 matches straight-through bar 2 | Pass |
| DifferentSeedDiffers | Different seed → different output | Pass |
| TempoIndependence | 120 BPM vs 90 BPM → same PPQ positions | Pass |
| NotPlaying | Transport stopped → zero events | Pass |
| PolymetricPhaseVariation | 5/16 ghost vs 4/4 kick shows phase drift | Pass |

**Assessment:** Core determinism contract comprehensively tested. The four transport stress scenarios (block-size, loop, jump, tempo) are the most valuable tests in the project.

## Test Infrastructure

- **Framework:** Google Test
- **Golden file:** `tests/golden/default_patch_4bars.txt` — reference output for regression detection
- **Test helpers:** `renderSorted()` renders across multiple blocks and sorts by PPQ for comparison; `serialize()` produces a comparable string representation

The `renderSorted()` helper is well-designed — it simulates real DAW behavior by splitting playback into configurable block sizes, then sorting events for position-stable comparison.

## Coverage Gaps

### Not Yet Testable (Features Not Implemented)

| Feature | Data Model | Implementation | Tests |
|---------|-----------|---------------|-------|
| Accent masks | `AccentMask` defined | Not applied in `renderRange()` | None |
| Emphasis probability | `emphasisProb` in `LaneConfig` | Not applied | None |
| Envelope evaluation | `Envelope`, `EnvelopeAssign` defined | Not applied | None |
| Humanization | `humanizeMs` in `LaneConfig` | Not applied | None |
| Macro resolution | `MacroValues` defined | Not applied | None |
| Ghost floor | `ghostFloor` in `LaneConfig` | Not applied | None |

These are Phase 1 implementation items. Tests should be written alongside implementation.

### Testable Now But Not Tested

| Gap | Risk | Recommended Test |
|-----|------|-----------------|
| Lane activation toggle | Low | Deactivated lane produces no events |
| Event buffer overflow | Medium | 8 lanes × 64 steps overflows `kMaxEventsPerBlock` |
| Negative PPQ positions | Low | Negative `ppqStart` handles correctly |
| Zero-length range | Low | `ppqStart == ppqEnd` produces no events |
| Very long ranges | Medium | 1000+ bar render doesn't crash |
| Single-step cycles | Low | Cycle with 1 step produces correct timing |
| Maximum lane count | Low | 8 active lanes render correctly |
| Velocity clamping boundaries | Low | `baseVelocity=0`, `baseVelocity=127`, `spread=1.0` |

### Recommended Phase 1 Test Additions

1. **Accent mask tests** — accented steps get boosted velocity; non-accented use base
2. **Envelope shape tests** — Sine, Ramp, Triangle produce expected modulation at known PPQ positions
3. **Envelope superposition tests** — two envelopes on same target combine correctly
4. **Macro coherence tests** — Density macro affects hitCount + probability together
5. **State serialization round-trip** — `getState()` → `setState()` preserves all fields
6. **Humanize determinism** — timing jitter is reproducible with same seed
7. **Real-time safety verification** — static analysis (clang-tidy) checks for allocation in `renderRange()`
8. **Preset version migration** — older state version loads correctly in newer code

## Test Quality Assessment

**Strengths:**
- Determinism tests are the project's most valuable safety net
- Block-size independence test catches a class of bugs that are hard to find manually
- Polymetric phase drift test validates the core musical promise
- Test helpers (`renderSorted`, `serialize`) are clean and reusable

**Weaknesses:**
- No negative/boundary value testing for transport parameters
- No overflow testing for `NoteEventBuffer`
- Golden file (`default_patch_4bars.txt`) is used for reference but not directly tested against in the suite (no `EXPECT_EQ(serialize(events), readFile(...))` pattern)

## Metrics

| Metric | Value |
|--------|-------|
| Test files | 2 |
| Total tests | 18 |
| Euclidean tests | 10 |
| Determinism tests | 8 |
| Lines of test code | ~290 |
| Lines of engine code | ~80 |
| Test:code ratio | ~3.6:1 |
