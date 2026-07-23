---
class: gated
---

# Poly Engine — Technical Specification

## Architecture

`poly_engine` is a pure C++ static library with zero VST3 or audio-thread dependencies. The plugin layer (`poly_plugin`) feeds it transport and parameter state, drains its `NoteEvent` output into the host's `IEventList`.

```
poly_plugin (VST3)          poly_engine (pure C++)
┌──────────────┐            ┌──────────────────────────┐
│ ProcessContext│──────────► │ TransportContext          │
│ Parameters   │            │ GrooveState               │
│              │            │                           │
│ IEventList   │◄────────── │ NoteEventBuffer           │
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
                     NoteEventBuffer& out);
};

}
```

`renderRange()` is called once per audio process block. It is a **pure function** of its inputs — no internal mutable state, no accumulation between calls.

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
    double ppqStart;       // Start of this block in PPQ
    double ppqEnd;         // End of this block in PPQ
    double tempo;          // BPM (for sample-position conversion only)
    double sampleRate;     // Hz
    int32_t blockSize;     // Samples in this block
    bool playing;          // Transport is running
    bool looping;          // Loop mode active
    bool jumped;           // Position discontinuity since last block
    double loopStartPpq;   // Loop region start
    double loopEndPpq;     // Loop region end
};
```

When `playing` is false, `renderRange()` returns immediately with no output.

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

The pattern is recomputed from `LaneConfig` each call — no cached state.

### 2. Step Iteration

For each absolute step index in the block's PPQ range:

1. Compute PPQ position: `absStep * stepPpq`
2. Guard: position must be within `[ppqStart, ppqEnd)`
3. Map to cycle-local step: `absStep % steps` (handles negative values)
4. Check Euclidean pattern at that step position

### 3. Probability Gate

```cpp
float probRoll = deterministicRand(seed, laneId, absStep, /*channel=*/0);
if (probRoll >= probability) continue;  // step filtered out
```

Channel 0 of the deterministic RNG is reserved for probability decisions.

### 4. Velocity Computation

```cpp
float velBase = baseVelocity / 127.0f;
float velRand = deterministicRand(seed, laneId, absStep, /*channel=*/1);
float spread = velocitySpread * (velRand * 2.0f - 1.0f);  // bipolar
float vel = clamp(velBase + spread, 0.0f, 1.0f);
```

Channel 1 of the deterministic RNG is reserved for velocity spread.

### 5. Event Output

```cpp
NoteEvent {
    ppqPosition = ppq,        // absolute PPQ
    pitch       = midiNote,   // lane's drum map slot
    velocity    = vel,         // 0.0–1.0 normalized
    duration    = stepPpq/2,   // half a step length
    channel     = 0            // MIDI channel
};
```

Events are pushed to the fixed-capacity `NoteEventBuffer` (256 events max per block).

## Deterministic RNG

```cpp
float deterministicRand(uint64_t seed, int laneId, int64_t absStep, uint32_t channel);
```

Position-seeded hash using splitmix64 mixing. The four input dimensions ensure:

- **seed**: Global patch seed for reproducibility
- **laneId**: Lane isolation — changing one lane doesn't affect others
- **absStep**: Position in musical time — same step always gets same roll
- **channel**: Purpose isolation — probability and velocity use independent streams

Returns `[0.0, 1.0)`. The hash combines all inputs via XOR with golden-ratio-derived constants, then applies three rounds of splitmix64 mixing.

## Data Model

### GrooveState (Serializable Patch)

```cpp
struct GrooveState {
    std::array<LaneConfig, 8> lanes;
    int activeLaneCount;                    // 4–8
    std::array<Envelope, 8> globalEnvelopes;
    int globalEnvelopeCount;
    MacroValues macros;
    uint64_t seed;
};
```

### LaneConfig

<!-- Regenerate via `node scripts/generate-param-docs.mjs`. Do not hand-edit content between markers. -->

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

### Capacity Constants

| Constant | Value | Rationale |
|----------|-------|-----------|
| kMaxLanes | 8 | UI complexity ceiling |
| kMaxSteps | 64 | Supports up to 64-step cycles |
| kMaxEnvelopesPerLane | 4 | Multi-timescale without overload |
| kMaxGlobalEnvelopes | 8 | Global modulation slots |
| kMaxEventsPerBlock | 256 | Bounded output buffer |

## Related Subsystems

Behaviors described in the data model above are implemented across the engine's
supporting modules — refer to those sources for evaluation semantics:

- **Accent expression** (`AccentMask`, `emphasisProb`) — evaluated in
  `engine/src/engine.cpp` via `effectiveEmphasis` gating against
  `deterministicRand`.
- **Envelope evaluation** (`EnvelopeAssign`, per-lane and global) —
  `engine/src/envelope.cpp:computeEnvelopePhase` derives phase from absolute
  PPQ; shape functions in the same file map phase to modulation value.
- **Humanization** (`humanizeMs`) — applied in `engine/src/engine.cpp` as a
  ms-to-PPQ timing offset against `ppqPosition`.
- **Macro resolution** (`MacroValues`) — `engine/src/macro.cpp:resolveMacros`
  maps macros to per-lane parameter deltas before the lane render loop; called
  from both the plugin processor and the WASM host.
