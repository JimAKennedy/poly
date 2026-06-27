# Architecture

Poly is split into two layers with a strict isolation boundary:

```
Host (Cubase/VST3)          poly_engine (pure C++, no VST3 deps)
┌───────────────┐          ┌──────────────────────────────────┐
│ ProcessContext ├─────────►│ Transport → Lane Generator →     │
│ Tempo / PPQ    │          │ Dynamic Shaping → Constraints →  │
│ Loop / jumps   │          │ Output Scheduler → NoteEvent[]   │
└───────────────┘          └──────────────┬───────────────────┘
┌───────────────┐                         │
│ MIDI Event Out │◄────────────────────────┘
└───────────────┘
```

- **`poly_engine`** -- pure C++ static library. Zero VST3 or audio-thread dependencies. Compiles and passes all tests without the SDK. Contains: Euclidean generator, envelopes, constraints, scenes, macros, phrase gating, mutation, drift, MIDI capture, and SMF writer.

- **`poly_plugin`** -- VST3 AudioEffect/Processor and EditController. Feeds transport and parameter state to the engine, drains `NoteEvent` output to the host's MIDI event list.

- **`tests/`** -- off-host unit tests, golden output determinism tests, UI interaction and visual regression tests.

See [IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md) for the full design, domain model, and phase plan.
