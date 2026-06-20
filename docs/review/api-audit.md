# M001 Engine API Surface Audit

## Current Public API

### Headers

| Header | Purpose | Symbols |
|--------|---------|---------|
| `poly/engine.h` | Core engine class | `Engine::renderRange()` |
| `poly/types.h` | Data model | 14 types, 2 enums, 5 constants |
| `poly/euclidean.h` | Pattern generation | `euclidean()` |
| `poly/rng.h` | Deterministic randomness | `deterministicRand()` |

### Functions

| Function | Signature | Notes |
|----------|-----------|-------|
| `Engine::renderRange` | `(const TransportContext&, const GrooveState&, NoteEventBuffer&) → void` | Core processing entry point |
| `euclidean` | `(int k, int n, int rotation, array<bool,64>&) → void` | Pattern generation utility |
| `deterministicRand` | `(uint64_t seed, int laneId, int64_t absStep, uint32_t channel) → float` | Position-seeded hash (inline) |
| `NoteEventBuffer::push` | `(const NoteEvent&) → bool` | Returns false on overflow |
| `NoteEventBuffer::clear` | `() → void` | Reset count to zero |

### Types

| Type | Kind | Fields |
|------|------|--------|
| `TransportContext` | struct | ppqStart, ppqEnd, tempo, sampleRate, blockSize, playing, looping, jumped, loopStartPpq, loopEndPpq |
| `NoteEvent` | struct | ppqPosition, pitch, velocity, duration, channel |
| `NoteEventBuffer` | struct | events[256], count |
| `LaneConfig` | struct | 16 fields (see types.h) |
| `Cycle` | struct | steps, subdivision |
| `AccentMask` | struct | steps[64] |
| `Envelope` | struct | target, periodBars, shape, depth, phaseOffset |
| `EnvelopeAssign` | struct | envelope, active |
| `MacroValues` | struct | complexity, density, syncopation, swing, tension, humanize |
| `GrooveState` | struct | lanes[8], activeLaneCount, globalEnvelopes[8], globalEnvelopeCount, macros, seed |
| `Role` | enum class | 8 values |
| `EnvTarget` | enum class | 8 values |
| `Shape` | enum class | 5 values |

### Constants

| Constant | Value | Used By |
|----------|-------|---------|
| `kMaxLanes` | 8 | GrooveState, lane iteration |
| `kMaxSteps` | 64 | AccentMask, euclidean() |
| `kMaxEnvelopesPerLane` | 4 | LaneConfig |
| `kMaxGlobalEnvelopes` | 8 | GrooveState |
| `kMaxEventsPerBlock` | 256 | NoteEventBuffer |

## API Completeness vs PRD

### Implemented

| PRD Requirement | API Coverage |
|----------------|-------------|
| Lane cycle/subdivision/hit/rotation | `LaneConfig` fields + `euclidean()` + `renderRange()` iteration |
| Per-step probability | `LaneConfig::probability` + `deterministicRand` channel 0 |
| Base velocity + spread | `LaneConfig::baseVelocity/velocitySpread` + `deterministicRand` channel 1 |
| Deterministic output | Position-seeded RNG + PPQ-derived step positions |
| Transport handling | `TransportContext` with loop/jump fields |
| MIDI event output | `NoteEvent` with ppqPosition/pitch/velocity/duration/channel |

### Defined But Not Yet Applied

| PRD Requirement | Data Model | Missing Implementation |
|----------------|-----------|----------------------|
| Accent masks | `AccentMask` in `LaneConfig` | Not read in `renderRange()` |
| Emphasis probability | `emphasisProb` in `LaneConfig` | Not applied to velocity |
| Ghost floor | `ghostFloor` in `LaneConfig` | Not used as velocity minimum |
| Humanization | `humanizeMs` in `LaneConfig` | Not applied as PPQ jitter |
| Per-lane envelopes | `EnvelopeAssign[4]` in `LaneConfig` | No evaluation function |
| Global envelopes | `Envelope[8]` in `GrooveState` | No evaluation function |
| Macro resolution | `MacroValues` in `GrooveState` | No parameter-mapping logic |

### Missing From API Entirely

| PRD Requirement | What's Needed |
|----------------|--------------|
| Envelope shape evaluation | `float evaluateShape(Shape, float phase) → value` |
| Envelope modulation application | Logic to apply envelope value to target parameter |
| Macro-to-parameter mapping | Function mapping macro values to per-lane parameter deltas |
| State serialization | `serialize(const GrooveState&, ostream&)` / `deserialize(istream&) → GrooveState` |
| MIDI note-off scheduling | Currently note-on only; note-offs needed for host `IEventList` |
| Event sorting | `renderRange()` doesn't sort output by PPQ; tests sort externally |

## API Design Concerns

### 1. No Event Sorting

`renderRange()` outputs events in lane-iteration order, not PPQ order. The host expects events sorted by sample offset within a block. Either the engine should sort, or the plugin layer must sort after draining.

**Recommendation:** Sort in the plugin layer (processor.cpp), not the engine. This keeps the engine simpler and avoids a dependency on sample rate for the sort key.

### 2. No Note-Off Events

The engine outputs note-on events with a `duration` field, but the VST3 `IEventList` needs explicit note-off events at `ppqPosition + duration`. The plugin layer must synthesize note-offs.

**Recommendation:** Add a `NoteOffScheduler` in the plugin layer that tracks active notes and emits note-offs at the correct sample offset. This must handle notes that span block boundaries.

### 3. Engine Has No State

`Engine` is stateless — it has no member variables and `renderRange()` is effectively a free function. This is a strength for determinism but will need adjustment when:
- Notes span block boundaries (need to track which notes are "active")
- Humanization requires knowing the previous block's jitter decisions

**Recommendation:** Keep the engine stateless for core generation. Add a thin `EngineState` struct for cross-block concerns (active notes, lookahead buffer) that the plugin owns and passes in.

### 4. No Overflow Diagnostic

`NoteEventBuffer::push()` returns `false` on overflow but the engine ignores it. In dense patches, silent event dropping is hard to diagnose.

**Recommendation:** Add a `uint32_t droppedCount` field to `NoteEventBuffer`. Increment on failed push. The plugin can report this via a diagnostic parameter or log.

## Phase 1 API Additions Needed

| Priority | Addition | Description |
|----------|----------|-------------|
| High | `evaluateEnvelope()` | Shape evaluation: phase → modulation value |
| High | `applyAccents()` | Accent mask + emphasis probability → velocity boost |
| High | `resolveMacros()` | Macro values → per-lane parameter deltas |
| Medium | `applyHumanize()` | Timing jitter from deterministicRand channel 3 |
| Medium | Serialization functions | GrooveState ↔ binary stream with version |
| Low | `NoteEventBuffer::droppedCount` | Overflow diagnostic counter |
| Low | Event sorting utility | Sort buffer by ppqPosition for host output |
