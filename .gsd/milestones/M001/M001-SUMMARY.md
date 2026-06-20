---
id: M001
title: "Definition and Architecture De-risking"
status: complete
completed_at: 2026-06-20T20:24:22.229Z
key_decisions:
  - Engine isolation: poly_engine as pure C++ static lib with zero VST3 SDK dependency
  - Euclidean rhythm algorithm (Bjorklund) for variable-length lane cycles
  - SplitMix64 as deterministic RNG — fast, splittable, and proven in audio contexts
  - PPQ-absolute timing: derive phase from projectTimeMusic, never accumulate
  - C++20 trial: designated initializers and modern patterns, proving before portfolio adoption
  - Golden test strategy: byte-identical output verification under transport stress
  - VST3 SDK 3.7+ with VSTGUI 4 (chose over JUCE for Cubase-first targeting)
key_files:
  - engine/include/poly/types.h
  - engine/include/poly/engine.h
  - engine/include/poly/euclidean.h
  - engine/include/poly/rng.h
  - engine/src/engine.cpp
  - engine/src/euclidean.cpp
  - plugin/source/processor.cpp
  - plugin/source/controller.cpp
  - tests/euclidean_tests.cpp
  - tests/golden_tests.cpp
  - tests/golden/default_patch_4bars.txt
  - docs/PRD.md
  - docs/engine-spec.md
  - docs/review/architecture-decisions.md
lessons_learned:
  - Golden tests are the strongest determinism proof — they catch accumulator drift, float instability, and block-boundary bugs that unit tests miss
  - Engine isolation pays off immediately: the CLI harness enables rapid iteration without DAW round-trips
  - C++20 designated initializers improve test readability significantly (named fields in Patch/TransportState construction)
  - Starting with -Wall -Wextra from commit 1 prevents warning debt accumulation
---

# M001: Definition and Architecture De-risking

**Established engine isolation boundary, proved deterministic timing with golden tests, and scaffolded the VST3 plugin structure for Poly.**

## What Happened

M001 de-risked the three foundational concerns for Poly: build system architecture, engine isolation, and timing determinism.

**S01** stood up the CMake project with VST3 SDK integration, producing a loadable `.vst3` bundle with factory, processor, and controller stubs. The `jk_warnings.cmake` module enforces `-Wall -Wextra` from day one.

**S02** created the `poly_engine` static library with the core domain model: `LaneConfig` (variable-length Euclidean cycles), `NoteEvent` (MIDI-ready output), `Patch` (multi-lane configuration), and `TransportState`. A CLI harness (`tools/harness/`) exercises the engine without the VST3 SDK, confirming the isolation boundary.

**S03** — the highest-risk slice — implemented Euclidean rhythm generation and proved deterministic output with 8 golden tests covering loop restart, position jump, tempo independence, block-size independence, and polymetric phase variation. The `SplitMix64` RNG ensures reproducibility from any seed.

**S04** produced 5 specification documents (PRD, engine-spec, Cubase workflow, automation mapping, wireframes) establishing the product vision and technical contracts.

**S05** delivered a milestone review: 7 architecture decision records, test coverage analysis, engine API audit, and a retrospective capturing lessons for M002.

## Success Criteria Results

All 5 success criteria met: VST3 builds cleanly, engine compiles without VST3 SDK, 18/18 tests pass (including 8 golden determinism tests), repo layout matches plan, Phase 0 documentation complete.

## Definition of Done Results

Not provided.

## Requirement Outcomes

Not provided.

## Deviations

None.

## Follow-ups

M002 (Core Groove Engine MVP) is next: implement the full multi-lane groove engine with velocity/swing, pattern variation, and real-time parameter control. M003 covers VST3 integration and Cubase testing.
