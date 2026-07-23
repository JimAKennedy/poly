---
class: gated
---

# Poly Engine — Technical Specification

## Architecture

`poly_engine` is a pure C++ static library with zero VST3 or audio-thread dependencies. The plugin layer (`poly_plugin`) feeds it transport and parameter state, drains its `NoteEvent` output into the host's `IEventList`, and optionally drains an `EmissionEvent` classification stream for the UI desk overlay.

```
poly_plugin (VST3)          poly_engine (pure C++)
┌──────────────┐            ┌──────────────────────────┐
│ ProcessContext│──────────► │ TransportContext          │
│ Parameters   │            │ GrooveState / SceneState  │
│              │            │                           │
│ IEventList   │◄────────── │ NoteEventBuffer           │
│ (desk UI)    │◄────────── │ EmissionEventBuffer       │
└──────────────┘            │                           │
                            │ Engine::renderRange()     │
                            └──────────────────────────┘
```

The engine must compile and pass its full test suite without the VST3 SDK.

## Core API

```cpp
namespace poly {

class Engine {
public:
    void renderRange(const TransportContext& tc,
                     const GrooveState& state,
                     NoteEventBuffer& out,
                     EmissionEventBuffer* emissions = nullptr);
};

}
```

`renderRange()` is called once per audio process block. It is a **pure function** of its inputs — no internal mutable state, no accumulation between calls. The optional `emissions` sink receives one `EmissionEvent` per step considered, so the desk UI can distinguish base hits, mutation-added notes, mutation-dropped notes, and ghost velocities.

### Real-Time Safety Contract

`renderRange()` must never:
- Allocate heap memory (`new`, `malloc`, `std::vector::push_back` past capacity)
- Acquire locks or mutexes
- Throw exceptions
- Perform I/O (file, network, logging)

All working buffers are stack-allocated or use fixed-capacity `std::array`.

## Timing Model

### PPQ-Derived Positioning

All timing is derived from absolute PPQ (pulses per quarter note) position, never accumulated across blocks:

- **Step PPQ**: `4.0 / cycle.subdivision` — a quarter note is always 1.0 PPQ
- **Cycle PPQ**: `stepPpq * cycle.steps` — total PPQ length of one cycle
- **Absolute step index**: `ceil(ppqStart / stepPpq)` to `ceil(ppqEnd / stepPpq)`
- **Cycle-local step**: `absStep % stepsInCycle` (with negative modulo correction)

This guarantees:
- Loop restarts reproduce identically (no drift from accumulated phase)
- Tempo changes take effect immediately (PPQ positions don't change)
- Position jumps produce correct output (no state to invalidate)
- Block boundaries are invisible (same step index regardless of split point)

### Transport Context

```cpp
struct TransportContext {
    double ppqStart;        // Start of this block in PPQ
    double ppqEnd;          // End of this block in PPQ
    double tempo;           // BPM (for sample-position conversion only)
    double sampleRate;      // Hz
    int32_t blockSize;      // Samples in this block
    bool playing;           // Transport is running
    bool looping;           // Loop mode active
    bool jumped;            // Position discontinuity since last block
    bool wrappedLoop;       // True iff `jumped` is a natural loop wrap
    double loopStartPpq;    // Loop region start
    double loopEndPpq;      // Loop region end
};
```

When `playing` is false, `renderRange()` returns immediately with no output. `wrappedLoop` lets callers preserve continuity across natural repeats (capture buffer, scene chain, macro smoother) while still resetting on manual playhead jumps.

## Lane Processing Pipeline

For each active lane in `[0, activeLaneCount)`:

### 1. Pattern Generation (Euclidean)

```cpp
void euclidean(int k, int n, int rotation, std::array<bool, kMaxSteps>& out);
```

Distributes `k` pulses (hits) across `n` steps as evenly as possible using a Bresenham/Bjorklund algorithm. `rotation` shifts the pattern right by that many positions (wrapping).

- `euclidean(4, 16, 0, out)` → standard 4-on-the-floor
- `euclidean(3, 8, 0, out)` → Cuban tresillo `[x . . x . . x .]`
- `euclidean(5, 8, 0, out)` → `[x . x x . x x .]` cinquillo

The pattern is recomputed from `LaneConfig` each call — no cached state. Timeline-mode lanes (`timeline=true`) substitute `fixedPattern` for the Euclidean grid, and kotekan-paired lanes substitute the complement of a source lane's pattern.

### 2. Step Iteration

For each absolute step index in the block's PPQ range:

1. Compute PPQ position: `absStep * stepPpq`
2. Guard: position must be within `[ppqStart, ppqEnd)`
3. Map to cycle-local step: `absStep % stepsInCycle` (handles negative values; wraps at `fixedPatternLength` in timeline mode, `cellCount` in additive-cell mode)
4. Check pattern at that step position

### 3. Classification (mutation, activation, probability, fill)

Each on-pattern step is classified via `classifyStep()` into one of five outcomes:

| Outcome | Meaning |
|---------|---------|
| `Base` | Emitted, on-pattern, unmodified velocity |
| `Ghost` | Emitted, on-pattern, mutation floored to ghost velocity |
| `Add` | Emitted, off-pattern (mutation-added or fill-added) |
| `Drop` | Suppressed, on-pattern (mutation drop, probability cull, activation cull) |
| `Silent` | Off-pattern non-hit — not recorded (nothing to display) |

Anchor steps (`ConstraintConfig::anchorSteps`) bypass mutation/probability so protected positions always fire.

### 4. Velocity Computation

```cpp
float velBase = baseVelocity / 127.0f;
float velRand = deterministicRand(seed, laneId, absStep, /*channel=*/1);
float spread = velocitySpread * (velRand * 2.0f - 1.0f);  // bipolar
float vel = clamp(velBase + spread + envelope + accent, 0.0f, 1.0f);
```

Accent-mask steps add an emphasis boost gated by `emphasisProb`. Envelope evaluation contributes to velocity, density, probability, note length, timing, activation, and fill likelihood — see the `EnvTarget` enum below.

### 5. Event Output

```cpp
struct NoteEvent {
    double ppqPosition;   // absolute PPQ
    int16_t pitch;        // MIDI note (post-NoteMap remap for plugin path)
    float velocity;       // 0.0–1.0 normalized
    double duration;      // PPQ
    int16_t channel;      // MIDI channel (0-15, or lane index if auto)
    int16_t laneIndex;    // originating lane
};
```

Events are pushed to the fixed-capacity `NoteEventBuffer` (256 events per block; overflow bumps `droppedCount`).

## Emission Classification Stream

Optional per-step classification for the desk UI. Populated when `renderRange()` is called with a non-null `emissions` sink:

```cpp
enum class EmissionKind : uint8_t { Base, Ghost, Add, Drop };

struct EmissionEvent {
    double ppqPosition;
    int16_t cycleStep;
    int16_t laneIndex;
    uint8_t kind;    // EmissionKind
};
```

`Silent` outcomes are omitted (nothing to render). The buffer holds up to `kMaxEmissionsPerBlock=512` entries per block; overflow bumps `droppedCount` and is safe.

## Deterministic RNG

```cpp
float deterministicRand(uint64_t seed, int laneId, int64_t absStep, uint32_t channel);
```

Position-seeded hash using splitmix64 mixing. The four input dimensions ensure:

- **seed**: Global patch seed for reproducibility
- **laneId**: Lane isolation — changing one lane doesn't affect others (see also `sanitizeGrooveState`, which enforces `id == laneIndex` so hostile presets can't correlate lanes)
- **absStep**: Position in musical time — same step always gets same roll
- **channel**: Purpose isolation — each decision stream is statistically independent

### Channel Assignment

| Channel | Purpose |
|---------|---------|
| 0 | Probability gate |
| 1 | Velocity spread |
| 2 | Accent emphasis gate |
| 3 | Humanize timing jitter |
| 4 | Fill likelihood |
| 5 | Activation weight |
| 8 | Mutation-fire gate (per-cycle-step) |
| 9 | Mutation type (drop / ghost / add) |

Returns `[0.0, 1.0)`. The hash combines all inputs via XOR with golden-ratio-derived constants, then applies three rounds of splitmix64 mixing.

## Data Model

### GrooveState (Serializable Patch)

```cpp
struct GrooveState {
    std::array<LaneConfig, kMaxLanes> lanes;
    int activeLaneCount;                            // 4–8
    std::array<Envelope, kMaxGlobalEnvelopes> globalEnvelopes;
    int globalEnvelopeCount;
    MacroValues macros;
    uint64_t seed;
    int globalDensityCeiling;                       // 0 = disabled
};
```

### SceneState (What the Plugin Actually Serializes)

```cpp
struct SceneState {
    GrooveState sceneA;
    GrooveState sceneB;
    SceneSelect select;         // A | B | Morph
    float morphAmount;          // 0.0–1.0 (A↔B blend when Morph)
    NoteMap noteMap;            // 128-entry post-render pitch remap
    SceneChainConfig chain;     // ordered scene playback
};
```

Under `SceneSelect::Morph`, the render path materializes an interpolated `GrooveState` from A and B via `interpolateGrooveState()` — the `MorphAmount` parameter controls the blend.

### LaneConfig

<!-- BEGIN GENERATED: laneconfig -->
| Field | Type | Default | Description |
|-------|------|---------|-------------|
| id | int | 0 | Lane identifier (used in RNG keying) |
| role | Role | Custom | Semantic role for UI grouping |
| midiNote | int16 | 36 | MIDI pitch (drum map) |
| cycle | Cycle | Steps × Subdivision defaults | Cycle length and subdivision |
| hitCount | int | 4 | Euclidean pulse count |
| rotation | int | 0 | Pattern rotation offset |
| probability | float | 100% | Per-step trigger probability |
| baseVelocity | uint8 | 100 | Nominal velocity (0–127) |
| accents | AccentMask | all false | Step positions with emphasis |
| emphasisProb | float | 50% | Accent expression probability |
| ghostFloor | uint8 | 30 | Minimum ghost-note velocity |
| velocitySpread | float | 5% | Velocity randomization range |
| humanizeMs | float | 0 | Timing jitter in milliseconds |
| active | bool | On | Lane on/off |
| envelopes | EnvelopeAssign[4] | — | Per-lane envelope assignments |
| envelopeCount | int | 0 | Active envelope count |
<!-- END GENERATED: laneconfig -->

The table above is generated from `engine/include/poly/types.h` by `scripts/generate-param-docs.mjs` (M048 S05). Do not hand-edit — CI's `check-generated-docs.sh` rejects any divergence. Additional `LaneConfig` fields not yet exposed via the generator: `midiChannel`, `swingAmount`, `noteDuration`, `phraseLength`/`phraseGap`/`phraseOffset`, `mutationRate`, `driftRate`, `timingOffsetMs`, `syncopationOffset`, `tempoMultiplier`, `kotekanSourceLane`, `cellCount`/`cellSizes`, `timeline`/`fixedPattern`/`fixedPatternLength`, `microTimingMs`, `constraints`.

### MacroValues (Six Musical-Intent Controls)

```cpp
struct MacroValues {
    float complexity;   // adds Euclidean rotation + envelope depth
    float density;      // scales probability + hitCount
    float syncopation;  // pushes even steps late
    float swing;        // per-lane swing amount
    float tension;      // envelope depth scaling
    float humanize;     // timing looseness
};
```

Applied via `resolveMacros(GrooveState) → GrooveState` in `engine/src/macro.cpp` before the lane render loop. Called from both the plugin processor and the WASM host so preset macros affect audible output.

### NoteMap (Global Pitch Remap)

```cpp
struct NoteMap {
    std::array<int16_t, 128> map;   // identity by default
    int16_t apply(int16_t note) const;
    void reset();
};
```

Applied by the plugin layer between engine output and MIDI emission. Lets a preset re-slot its lanes onto the user's actual drum kit without changing lane configuration.

### SceneChain

```cpp
struct SceneChainConfig {
    std::array<SceneChainEntry, kMaxChainEntries> entries;
    int entryCount;
    ChainMode mode;             // OneShot | Loop | PingPong
    bool enabled;
};
```

`SceneChainState::update(config, ppqPosition)` returns the current `SceneSelect` for a given PPQ, advancing at bar boundaries. Used by both the plugin processor and the WASM host so chain playback matches between DAW and site.

## Enum Reference

### Role (Lane Semantics)

`AnchorPulse`, `Backbeat`, `Shimmer`, `Accent`, `Ghost`, `Ornament`, `Fill`, `Custom`. Used for UI grouping and preset organization — not a functional gate on the render path.

### EnvTarget (Envelope Destinations)

`Velocity`, `Density`, `Probability`, `AccentBias`, `NoteLength`, `TimingLooseness`, `ActivationWeight`, `FillLikelihood`.

### Shape (Envelope Curves)

`Ramp`, `Sine`, `Triangle`, `Curve`, `StepList` (up to `kMaxStepListEntries=16` values).

### SceneSelect / ChainMode

`SceneSelect`: `A`, `B`, `Morph`. `ChainMode`: `OneShot`, `Loop`, `PingPong`.

## Capacity Constants

| Constant | Value | Rationale |
|----------|-------|-----------|
| kMaxLanes | 8 | UI complexity ceiling |
| kMaxSteps | 64 | Supports up to 64-step cycles |
| kMaxEnvelopesPerLane | 4 | Multi-timescale without overload |
| kMaxGlobalEnvelopes | 8 | Global modulation slots |
| kMaxStepListEntries | 16 | StepList envelope resolution |
| kMaxEventsPerBlock | 256 | Bounded note-output buffer |
| kMaxEmissionsPerBlock | 512 | Bounded classification stream (each step may emit) |
| kMaxChainEntries | 16 | Bounded scene chain length |

## Related Subsystems

Behaviors described in the data model above are implemented across the engine's
supporting modules — refer to those sources for evaluation semantics:

- **Accent expression** (`AccentMask`, `emphasisProb`) — evaluated in
  `engine/src/engine.cpp` via `effectiveEmphasis` gating against
  `deterministicRand` channel 2.
- **Envelope evaluation** (`EnvelopeAssign`, per-lane and global) —
  `engine/src/envelope.cpp:computeEnvelopePhase` derives phase from absolute
  PPQ; shape functions in the same file map phase to modulation value.
- **Humanization** (`humanizeMs`) — applied in `engine/src/engine.cpp` as a
  ms-to-PPQ timing offset seeded by `deterministicRand` channel 3.
- **Macro resolution** (`MacroValues`) — `engine/src/macro.cpp:resolveMacros`
  maps macros to per-lane parameter deltas before the lane render loop; called
  from both the plugin processor and the WASM host.
- **Scene morph / chain** — `engine/src/scene.cpp:interpolateGrooveState` +
  `SceneChainState::update`; the plugin's `PolyProcessor::process` chooses A,
  B, or a morph blend per block based on `SceneSelect` and optional chain state.
- **State I/O** — `engine/include/poly/state_io.h` versions preset serialization
  at `kCurrentStateVersion=15`; `sanitizeGrooveState` enforces engine-safe
  ranges on every deserialize so a corrupted preset can never index past a
  fixed-size array or produce a zero-length cycle.
