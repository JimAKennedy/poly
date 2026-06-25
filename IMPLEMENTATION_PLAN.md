# Poly — Implementation Plan

A Cubase-targeted **VST3 instrument** that generates evolving polymetric grooves from
4–8 independent rhythmic lanes, with dynamics-first velocity/emphasis modeling and
superimposed multi-timescale envelopes. This document turns the
[product roadmap](internal-docs/roadmap.md) into a concrete engineering plan.

> **Working assumptions** (call these out / change if wrong):
> - **Framework:** the **Steinberg VST3 SDK** directly (`Steinberg::Vst` component model),
>   with **VSTGUI** for the editor. No JUCE.
> - **Build:** CMake (the VST3 SDK ships a CMake project + `smtg_add_vst3plugin` helpers), C++20.
> - **Engine isolation:** the groove engine is a **host-agnostic static library** with
>   zero VST3/audio-thread dependencies, so it is fully unit-testable off-host.
> - **Platforms:** macOS (universal) + Windows first, since that is the Cubase userbase.
> - **Scope of this plan:** Phase 0 → Phase 2 from the roadmap. ML/adaptive features stay deferred.

---

## 1. Guiding architecture

The roadmap's core insight is the right one: **separate the pure musical model from the
host and the UI.** Everything below is organized around that boundary.

```
                         ┌─────────────────────────────────────────────┐
   Host (Cubase/VST3)    │                  poly_engine                 │
   ┌───────────────┐     │   (pure C++, no VST3, no audio-thread alloc) │
   │ ProcessContext│────►│  Transport/Time   →  bar/beat/sub/lane-phase │
   │ Tempo / PPQ   │     │  Lane Generator   →  hit candidates per lane │
   │ Loop / jumps  │     │  Dynamic Shaping  →  velocity/emphasis/env   │
   └───────────────┘     │  Constraint Layer →  anchors / density caps  │
   ┌───────────────┐     │  Output Scheduler →  ordered NoteEvent list  │
   │ Event MIDI out│◄────│                                              │
   └───────────────┘     └─────────────────────────────────────────────┘
            ▲                                   ▲
            │                                   │
   ┌────────┴────────────┐             ┌────────┴────────┐
   │ poly_plugin         │             │ poly_tests      │
   │ AudioEffect/Processor│            │ (off-host unit  │
   │ + EditController     │            │  + golden tests)│
   │ + VSTGUI editor      │            └─────────────────┘
   └─────────────────────┘
```

**Hard rule:** `poly_engine` must compile and pass its full test suite with **no VST3 SDK
and no DAW**. The plugin layer only (a) feeds it `ProcessContext`/parameter state and
(b) drains its `NoteEvent` output into the host's output `IEventList`.

### Why this matters for the roadmap's promises
- **Determinism** (Product Principle): the engine is a pure function of
  `(transport position, lane state, seed)`. Same inputs → same output, every pass. This is
  testable without audio.
- **Musical legibility:** each layer maps 1:1 to a roadmap concept (lanes, accents, envelopes).
- **Future ML:** an adaptive layer can *propose parameter/curve changes* without ever owning
  the note-generation path.

---

## 2. Repository layout

```
poly/
├── CMakeLists.txt              # top-level; pulls in VST3 SDK, adds subdirs
├── cmake/                      # toolchain + helper modules
├── docs/
│   ├── roadmap.md              # the source vision doc
│   ├── PRD.md                  # Phase 0 deliverable
│   ├── engine-spec.md          # timing/cycle/velocity/envelope spec
│   └── cubase-workflow.md      # routing/record/freeze guide
├── engine/                     # poly_engine — pure C++ core (NO VST3 SDK)
│   ├── include/poly/…          # public headers
│   ├── src/…
│   └── CMakeLists.txt
├── plugin/                     # poly_plugin — VST3 instrument (Steinberg SDK)
│   ├── source/
│   │   ├── factory.cpp         # GetPluginFactory / class registration
│   │   ├── processor.*         # AudioEffect: process(), ProcessContext → engine
│   │   ├── controller.*        # EditController: params, units, automation
│   │   ├── params/             # ParamID enum, normalized<->plain conversions
│   │   └── ui/                 # VSTGUI: lane / macro / envelope views (.uidesc)
│   └── CMakeLists.txt          # smtg_add_vst3plugin(...)
├── tests/                      # poly_tests — Catch2/GoogleTest
│   ├── timing_tests.cpp
│   ├── lane_tests.cpp
│   ├── envelope_tests.cpp
│   └── golden/                 # recorded reference event streams
└── tools/
    └── harness/                # headless engine runner (CLI) for Phase 0 prototype
```

---

## 3. Engine domain model

The data model derives directly from the roadmap's lane properties and envelope targets.

```cpp
// All times are in musical units (PPQ / fractions of a bar), never samples,
// so the engine is sample-rate and buffer-size independent.

struct LaneConfig {
    int        id;
    Role       role;              // AnchorPulse, Backbeat, Shimmer, Accent, Ghost, Ornament…
    int        midiNote;          // target voice (drum map slot)
    Cycle      cycle;             // length in steps + step subdivision (e.g. 5 steps of 1/16)
    int        hitCount;          // pulses distributed across the cycle (Euclidean by default)
    int        rotation;          // offset of the pulse pattern
    float       probability;       // 0..1 per-step trigger chance
    uint8_t     baseVelocity;      // nominal intensity
    AccentMask  accents;          // stronger positions/steps
    float       emphasisProb;     // how often accents actually express
    uint8_t     ghostFloor;       // min velocity for supporting texture
    float       velocitySpread;   // controlled randomization
    float       humanizeMs;       // timing looseness (applied as PPQ jitter)
    std::vector<EnvelopeAssign> envelopes;   // multiple per lane
};

enum class EnvTarget { Velocity, Density, Probability, AccentBias,
                       NoteLength, TimingLooseness, ActivationWeight, FillLikelihood };

struct Envelope {
    EnvTarget    target;
    float        periodBars;      // 1, 4, 7, 16, or custom (need NOT divide evenly — that's the point)
    Shape        shape;           // ramp / sine / curve / step list
    float        depth;           // modulation amount
    float        phaseOffset;     // independent phase → multiphase interest
};

struct GrooveState {            // the full serializable patch
    Tempo/meter context;
    std::array<LaneConfig, 8> lanes;   // 4..8 active
    std::vector<Envelope>     globalEnvelopes;
    MacroValues               macros;  // complexity, density, syncopation, swing, tension, humanize…
    uint64_t                  seed;
};
```

### The processing contract
```cpp
// Called per process block. Pure: no allocation, no locks, deterministic.
void Engine::renderRange(const TransportContext& tc,   // ppqStart, ppqEnd, tempo, looping, jumped
                         const GrooveState& state,
                         NoteEventBuffer& out);
```
- **Envelope phase** is derived from absolute song PPQ position, *not* accumulated over blocks
  → loop restarts and song-position jumps reproduce identically (no drift).
- **Macros** are resolved into concrete per-lane parameters *coherently* (e.g. "Density"
  scales hitCount + probability + activation envelopes together), not as a single multiply.

---

## 4. Phase plan & milestones

### Phase 0 — Definition & architecture de-risking
**Goal:** lock the Cubase-first model and prove deterministic timing before feature work.

| # | Task | Output |
|---|------|--------|
| 0.1 | Stand up CMake + VST3 SDK skeleton (`smtg_add_vst3plugin`); empty instrument loads in Cubase; CI builds mac+win | green build |
| 0.2 | Define `poly_engine` public headers (model from §3) | `engine/include` |
| 0.3 | Implement Transport/Time layer + headless CLI harness | `tools/harness` |
| 0.4 | **Timing prototype:** prove cycle + envelope phase are identical across loop restarts / tempo changes | golden test |
| 0.5 | Write PRD, engine-spec, Cubase-workflow docs | `docs/*` |
| 0.6 | UI wireframes: lane overview, focused lane editor, envelope editor | `docs/` images |
| 0.7 | Decide automation contract: which params exposed to Cubase vs internal compounds | spec section |

**Exit criteria:** an empty VST3 instantiates in Cubase; the engine harness produces a
deterministic event stream that is byte-identical under loop/tempo/jump stress.

### Phase 1 — Core groove engine MVP
**Goal:** a musically useful generator, no ML.

| # | Task | Notes |
|---|------|-------|
| 1.1 | Lane generator: cycle/subdivision/hitCount/rotation, Euclidean default | per-lane independence |
| 1.2 | Per-lane probability + seeded RNG (reproducible) | seed in patch |
| 1.3 | Dynamic shaping: base velocity, accent mask, emphasis prob, ghost floor, spread | dynamics-first |
| 1.4 | Superimposed envelopes — v1 targets **velocity + density** | multi-period, phase-offset |
| 1.5 | Macros: complexity, density, syncopation, swing, tension | coherent multi-param mapping |
| 1.6 | Plugin layer: EditController params + processor `process()`, emit notes to output `IEventList`, read `ProcessContext` | real-time safe |
| 1.7 | Minimal UI: 4–8 lane grid + macro row + per-lane velocity/accent view (VSTGUI `.uidesc`) | legibility |
| 1.8 | Patch save/load via `IComponent::getState`/`setState` (versioned stream) | state model |

**Exit / quality criteria (from roadmap):** stable under start/stop, loop, tempo change;
repeatable when randomness disabled; visible lane→audio correspondence; audible velocity/emphasis.

### Phase 2 — Musical refinement & Cubase polish
**Goal:** turn the generator into a composition tool.

| # | Task |
|---|------|
| 2.1 | Multiple simultaneous envelopes per lane; full target set (length, looseness, activation, fill) |
| 2.2 | Phrase/section modulation lengths: 4 / 8 / 16 / custom bars |
| 2.3 | Scene / snapshot switching with A/B and morph |
| 2.4 | Constraints: anchor kick, preserve backbeat, protect lane density ranges |
| 2.5 | Better Cubase automation mapping for high-value controls |
| 2.6 | MIDI capture / drag-to-track / freeze / pattern export workflow |
| 2.7 | Phase-relationship + envelope-interaction visualizations |

**Exit criteria:** user can write a full rhythmic section while keeping structural anchors intact.

---

## 5. Functional breakdown (tracks the roadmap table)

| Area | Phase 0 | Phase 1 | Phase 2 |
|---|---|---|---|
| Cubase workflow model | Define (`docs/`) | Implement (record/route) | Polish (freeze/drag/export) |
| VST3 packaging | Skeleton + CI | Param + MIDI out | Stabilize |
| Lane engine | Headers + timing | Implement | Optimize |
| 4–8 lane UI | Wireframe | Implement | Refine |
| Velocity/emphasis | Spec | Implement | Expand |
| Envelope superposition | Design | Velocity+density | Advanced layering |
| Macro controls | Define | 5 core macros | Tune |
| MIDI capture/export | Define | Basic | Better UX |
| Scene/snapshot | Define | If feasible | Expand |
| Adaptive/ML | Deferred | Deferred | Deferred |

---

## 6. Testing strategy

- **Unit tests** (off-host, fast): Euclidean distribution, rotation, accent masks, envelope
  phase math, macro→param mapping.
- **Golden / characterization tests:** serialize the engine's `NoteEvent` stream for a fixed
  `(patch, seed, transport script)` and diff against a checked-in reference. This is how we
  *enforce determinism* in CI.
- **Stress scripts:** loop-restart, mid-bar transport jump, tempo ramp — assert identical output.
- **Real-time safety checks:** assert no allocation/locks in `renderRange` (debug allocator hook).
- **Manual host QA:** documented Cubase pass/fail checklist (instantiate, route, record, freeze).

---

## 7. Key technical risks & mitigations

| Risk | Mitigation |
|---|---|
| VST3 note output / event routing semantics | Target Cubase only; emit `Event` (NoteOn/Off) into the output `IEventList`; validate the canonical record/route path early (Phase 0). Declare a silent audio output bus (Cubase expects instruments to have one). |
| Envelope phase drift across blocks/loops | Derive phase from absolute `ProcessContext` PPQ (`projectTimeMusic`), never accumulate; golden tests guard it. |
| UI density with 8 lanes × N envelopes | Three modes: overview / focused lane / envelope editor. Progressive disclosure. |
| Determinism vs surprise | Seed management + explicit randomness toggles; "freeze seed" by default. |
| Real-time audio-thread safety | Engine pre-allocates; consume host `IParameterChanges` queues into a lock-free snapshot; no STL alloc in `process()`. |

---

## 8. Immediate next actions

1. Scaffold CMake + Steinberg VST3 SDK (`smtg_add_vst3plugin`) so an empty instrument loads in Cubase (Phase 0.1).
2. Land `poly_engine` public headers (§3 model) + the headless harness (0.2–0.3).
3. Build the **timing prototype** and its golden determinism test (0.4) — the single
   highest-leverage de-risking step.
4. Move `roadmap.md` into `docs/` and draft the PRD + engine-spec alongside the code.

> Tell me to proceed and I'll start with steps 1–3 (scaffold + engine headers + timing
> prototype) on this branch.
