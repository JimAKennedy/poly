# Poly — Product Requirements Document

## Overview

Poly is a Cubase-targeted VST3 instrument that generates evolving polymetric drum grooves from 4–8 independent rhythmic lanes. It outputs MIDI note events, not audio. The core value proposition is structured rhythmic complexity that remains editable, deterministic, and musically legible.

## Problem Statement

Producers working in Cubase lack a compositional tool for polymetric percussion. Existing tools offer pattern libraries, basic Euclidean sequencing, or probability-based randomization, but none provide:

- Independent variable-length cycle lanes that create emergent composite rhythms
- Dynamics-first velocity/emphasis modeling as a structural rhythmic element
- Multi-timescale envelope superposition for phrase- and section-level motion
- Deterministic output that reproduces identically under transport stress

## Target Users

**Primary:** Cubase producers making electronic, hybrid, and groove-oriented music who want evolving drum patterns that go beyond static step-sequencer programming.

**Secondary:** Composers and sound designers driving percussion instruments or sampled ensembles from a lane-based generator.

## Product Principles

1. **Musical legibility** — every result is explainable in terms of lanes, cycles, accents, and envelopes.
2. **Editable output** — MIDI can be captured, dragged, or recorded into standard Cubase workflows.
3. **Determinism** — same `(patch, seed, transport)` inputs produce identical output every time.
4. **Dynamics-first** — velocity and emphasis are part of the composition model, not cosmetic modifiers.
5. **Long-form motion** — phrase- and section-level development through envelope superposition.
6. **Host focus** — optimize for Cubase; defer cross-host compatibility.

## Scope

### In Scope (Phase 0–2)

| Area | Description |
|------|-------------|
| Lane engine | 4–8 independent lanes with variable cycle length, subdivision, hit count, rotation |
| Euclidean rhythms | Bresenham/Bjorklund pulse distribution as default pattern generation |
| Velocity model | Base velocity, accent masks, emphasis probability, ghost floor, spread |
| Probability | Per-step trigger probability with seeded deterministic RNG |
| Envelope superposition | Multiple per-lane + global envelopes modulating velocity, density, probability, and more |
| Macro controls | Complexity, density, syncopation, swing, tension, humanize |
| Cubase integration | VST3 instrument on instrument track, MIDI output routing, record/freeze/export |
| State management | Versioned binary serialization via `getState()`/`setState()` |

### Out of Scope

- Audio synthesis — Poly is MIDI-only
- ML/adaptive features — deferred beyond Phase 2
- Cross-host compatibility — Cubase-only for initial phases
- drumcore integration — Poly's variable-length cycles don't map to drumcore's fixed bar-grid model

## Lane Architecture

Each lane is a self-contained rhythmic agent:

| Property | Type | Description |
|----------|------|-------------|
| Role | enum | AnchorPulse, Backbeat, Shimmer, Accent, Ghost, Ornament, Fill, Custom |
| MIDI Note | int16 | Target drum map slot (default 36 = kick) |
| Cycle | steps × subdivision | Variable length; e.g. 5 steps of 1/16, 7 steps of 1/8 |
| Hit Count | int | Pulses distributed across cycle via Euclidean algorithm |
| Rotation | int | Pattern offset within cycle |
| Probability | float 0–1 | Per-step trigger chance |
| Base Velocity | uint8 | Nominal intensity (0–127) |
| Accent Mask | bool[64] | Stronger positions within the cycle |
| Emphasis Probability | float 0–1 | How often accents actually express |
| Ghost Floor | uint8 | Minimum velocity for supporting texture |
| Velocity Spread | float | Controlled randomization range |
| Humanize | float ms | Timing looseness (applied as PPQ jitter) |
| Envelopes | up to 4 | Per-lane modulation assignments |

4–8 lanes active simultaneously. Upper bound of 8 balances musical richness against UI complexity.

## Velocity and Dynamics Model

Velocity is modeled at multiple levels, not as a single knob:

1. **Base velocity** — nominal lane intensity
2. **Accent mask** — structural strong-beat positions
3. **Emphasis probability** — stochastic accent expression
4. **Ghost floor** — minimum articulation for texture
5. **Velocity spread** — controlled per-step variation via deterministic RNG
6. **Envelope modulation** — phrase-scale intensity changes

## Envelope Superposition

Multiple envelopes can be superimposed per lane and globally. Each envelope has:

- **Target**: Velocity, Density, Probability, AccentBias, NoteLength, TimingLooseness, ActivationWeight, FillLikelihood
- **Period**: 1, 4, 7, 16 bars or custom (need not divide evenly)
- **Shape**: Ramp, Sine, Triangle, Curve, StepList
- **Depth**: Modulation amount
- **Phase offset**: Independent phase for multi-timescale interest

Limits: 4 envelopes per lane, 8 global envelopes.

## Macro Controls

High-level controls that coherently alter multiple underlying parameters:

| Macro | Effect |
|-------|--------|
| Complexity | Scales hit counts, rotation variety, accent density |
| Density | Scales hit counts + probability + activation envelopes |
| Syncopation | Shifts accent positions, adjusts rotation |
| Swing | Timing offset on even subdivisions |
| Tension | Increases velocity spread, accent probability, envelope depth |
| Humanize | Timing looseness across all lanes |

## Determinism Contract

Engine output is a pure function of `(TransportContext, GrooveState)`:

- Cycle phase derived from absolute PPQ position, never accumulated
- RNG is position-seeded: `deterministicRand(seed, laneId, absStep, channel)`
- Same inputs produce identical `NoteEvent` output regardless of:
  - Block size or buffer boundaries
  - Loop restart position
  - Tempo changes
  - Transport position jumps

## MVP Success Criteria

1. Empty VST3 instrument instantiates in Cubase without error
2. `poly_engine` compiles and passes all tests with no VST3 SDK dependency
3. Deterministic event stream is byte-identical under loop/tempo/jump stress
4. 4+ interlocking lanes produce audible composite grooves
5. MIDI output can be recorded into Cubase MIDI parts
6. Velocity/emphasis changes are musically audible
7. Stable under transport start/stop, loop, and tempo changes

## State Serialization

- Binary stream via VST3 `getState()`/`setState()`
- First int32 is always `kStateVersion` — mandatory for preset compatibility
- Version branching in `setState()` for forward migration
- Full `GrooveState` serialized: lanes, envelopes, macros, seed

## Key Constraints

- **Real-time safety**: No heap allocation, locks, exceptions, or I/O in `process()` or `renderRange()`
- **Pre-allocation**: All buffers allocated in `initialize()`, only cleared in `setActive()`
- **C++20**: First jk.digital project using C++20 features (std::span, designated initializers, concepts)
- **Engine isolation**: `poly_engine` has zero VST3/audio-thread dependencies
