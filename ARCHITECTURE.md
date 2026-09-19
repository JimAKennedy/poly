# Architecture

Poly is split into two layers with a strict isolation boundary:

```mermaid
flowchart LR
  subgraph host["Host (Cubase/VST3)"]
    pc["ProcessContext<br/>Tempo / PPQ<br/>Loop / jumps"]
    midiout["MIDI Event Out"]
  end

  subgraph engine["poly_engine (pure C++, no VST3 deps)"]
    transport["Transport"] --> generator["Lane Generator"]
    generator --> shaping["Dynamic Shaping"]
    shaping --> constraints["Constraints"]
    constraints --> scheduler["Output Scheduler"]
    scheduler --> events["NoteEvent[]"]
  end

  pc --> transport
  events --> midiout
```

- **`poly_engine`** -- pure C++ static library. Zero VST3 or audio-thread dependencies. Compiles and passes all tests without the SDK. Contains: Euclidean generator, envelopes, constraints, scenes, macros, phrase gating, mutation, drift, MIDI capture, and SMF writer.

- **`poly_plugin`** -- VST3 AudioEffect/Processor and EditController. Feeds transport and parameter state to the engine, drains `NoteEvent` output to the host's MIDI event list.

- **`tests/`** -- off-host unit tests, golden output determinism tests, UI interaction and visual regression tests.

Active work is tracked in the public
[GitHub milestones](https://github.com/JimAKennedy/poly/milestones) and
[CHANGELOG](CHANGELOG.md). [IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md)
is the archived Phase 0 planning document — useful as a historical record of
early design intent, not as a current-state reference.
